#include "bt_manager.h"

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "gamepad.h"

#define DEVICE_NAME "ESP32-DOOM-Joystick"
#define RX_QUEUE_LEN 64

static const char *TAG = "BLE_JOYSTICK";
static QueueHandle_t rx_queue;

/* Nordic UART Service UUIDs */
static const ble_uuid128_t service_uuid =
    BLE_UUID128_INIT(0x9E,0xCA,0xDC,0x24,0x0E,0xA9,0xE0,0x93,
                     0xF3,0xA3,0xB5,0x01,0x00,0x40,0x6E,0x00);

static const ble_uuid128_t rx_char_uuid =
    BLE_UUID128_INIT(0x9E,0xCA,0xDC,0x24,0x0E,0xA9,0xE0,0x93,
                     0xF3,0xA3,0xB5,0x02,0x00,0x40,0x6E,0x00);

static const ble_uuid128_t tx_char_uuid =
    BLE_UUID128_INIT(0x9E,0xCA,0xDC,0x24,0x0E,0xA9,0xE0,0x93,
                     0xF3,0xA3,0xB5,0x03,0x00,0x40,0x6E,0x00);

static int rx_chr_access_cb(uint16_t conn_handle,
                            uint16_t attr_handle,
                            struct ble_gatt_access_ctxt *ctxt,
                            void *arg)
{
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
        return 0;
    }

    uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
    if (len == 0) {
        return 0;
    }

    char buf[64];
    uint16_t copy_len = len < sizeof(buf) ? len : sizeof(buf);

    int rc = ble_hs_mbuf_to_flat(ctxt->om, buf, copy_len, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_mbuf_to_flat failed: %d", rc);
        return BLE_ATT_ERR_UNLIKELY;
    }

    for (uint16_t i = 0; i < copy_len; i++) {
        char c = buf[i];
        ESP_LOGI(TAG, "RX: '%c'", c);

        /* Directly forward command to Doom gamepad logic */
        readCommands(c);
    }

    return 0;
}

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &rx_char_uuid.u,
                .access_cb = rx_chr_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {
                .uuid = &tx_char_uuid.u,
                .access_cb = rx_chr_access_cb, /* Dummy handler required by ESP-IDF NimBLE */
                .flags = BLE_GATT_CHR_F_NOTIFY,
            },
            {0}
        }
    },
    {0}
};

static void ble_app_advertise(void);

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                ESP_LOGI(TAG, "Cliente conectado");
            } else {
                ble_app_advertise();
            }
            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Cliente desconectado");
            ble_app_advertise();
            return 0;

        default:
            return 0;
    }
}

static void ble_app_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    uint8_t addr_type;

    int rc = ble_hs_id_infer_auto(0, &addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto failed: %d", rc);
        return;
    }

    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (const uint8_t *)DEVICE_NAME;
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_set_fields failed: %d", rc);
        return;
    }

    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(addr_type,
                           NULL,
                           BLE_HS_FOREVER,
                           &adv_params,
                           ble_gap_event_cb,
                           NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_start failed: %d", rc);
        return;
    }

    ESP_LOGI(TAG, "Advertising como '%s'", DEVICE_NAME);
}

static void ble_on_sync(void)
{
    ESP_LOGI(TAG, "ble_on_sync called");
    ble_app_advertise();
}

static void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void bt_init(void)
{
    rx_queue = xQueueCreate(RX_QUEUE_LEN, sizeof(char));
    if (!rx_queue) {
        ESP_LOGE(TAG, "No se pudo crear rx_queue");
        return;
    }

    ESP_ERROR_CHECK(nimble_port_init());

    ble_svc_gap_init();
    ble_svc_gatt_init();

    int rc = ble_gatts_count_cfg(gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg failed: %d", rc);
        return;
    }

    rc = ble_gatts_add_svcs(gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_add_svcs failed: %d", rc);
        return;
    }

    rc = ble_gatts_start();
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_start failed: %d", rc);
        return;
    }

    ble_hs_cfg.sync_cb = ble_on_sync;

    nimble_port_freertos_init(host_task);

    ESP_LOGI(TAG, "Bluetooth listo. Esperando conexión...");
}

int bt_available(void)
{
    return uxQueueMessagesWaiting(rx_queue);
}

char bt_read(void)
{
    char c = 0;
    if (xQueueReceive(rx_queue, &c, 0) == pdTRUE) {
        return c;
    }
    return 0;
}
