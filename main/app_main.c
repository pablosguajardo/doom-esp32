// Copyright 2016-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp_attr.h"

#include "rom/cache.h"
#include "rom/ets_sys.h"
#include "rom/spi_flash.h"
#include "rom/crc.h"

#include "soc/soc.h"
#include "soc/dport_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/efuse_reg.h"
#include "soc/rtc_cntl_reg.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "spi_flash_mmap.h"

#undef false
#undef true
#include "i_system.h"

#include "spi_lcd.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_heap_caps.h"

// FOR BT:
#include "sdkconfig.h"



#include <stdint.h>
#include <string.h>
#include <stdio.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
// validar esto:
// #include "prboom-esp32-compat/include/gamepad.h"

extern void jsInit();

#define JOYSTICK_OPTION CONFIG_HW_JOYSTICK_OPTION



//Joystick::::::::::::::::::::::::::::::::
#define DEVICE_NAME "ESP32-DOOM-Joystick"
#define RX_QUEUE_LEN 64

static const char *TAG = "BLE_JOYSTICK";
static QueueHandle_t rx_queue;

/* UUIDs propios */
static const ble_uuid128_t service_uuid =
    BLE_UUID128_INIT(0x78, 0x56, 0x34, 0x12,
                     0x78, 0x56,
                     0x34, 0x12,
                     0x78, 0x56,
                     0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static const ble_uuid128_t rx_char_uuid =
    BLE_UUID128_INIT(0x21, 0x43, 0x65, 0x87,
                     0x21, 0x43,
                     0x65, 0x87,
                     0x21, 0x43,
                     0x65, 0x87, 0x21, 0x43, 0x65, 0x87);

static int rx_chr_access_cb(uint16_t conn_handle,
                            uint16_t attr_handle,
                            struct ble_gatt_access_ctxt *ctxt,
                            void *arg)
{
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
        return BLE_ATT_ERR_UNLIKELY;
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
        xQueueSend(rx_queue, &c, 0);
        ESP_LOGI(TAG, "RX: '%c'", c);
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
                ESP_LOGI(TAG, "Fallo conexion, reintentando advertising");
                ble_app_advertise();
            }
            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Cliente desconectado");
            ble_app_advertise();
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "Advertising completo, reiniciando");
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
    uint8_t addr_type;
    int rc = ble_hs_id_infer_auto(0, &addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_id_infer_auto failed: %d", rc);
        return;
    }

    ble_app_advertise();
}

static void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/* Forward declarations */
static int bt_available(void);
static char bt_read(void);

static void bt_input_task(void *param)
{
    while (1)
    {
        if (bt_available())
        {
            char command = bt_read();
            readCommands(command);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* API simple estilo available/read */
static int bt_available(void)
{
    return uxQueueMessagesWaiting(rx_queue);
}

static char bt_read(void)
{
    char c = 0;
    if (xQueueReceive(rx_queue, &c, 0) == pdTRUE) {
        return c;
    }
    return 0;
}

void SetupBt()
{
	if (JOYSTICK_OPTION == 2)
	{
		printf("Bt is enabled. Init\n");

		rx_queue = xQueueCreate(RX_QUEUE_LEN, sizeof(char));
		if (!rx_queue)
		{
			ESP_LOGE(TAG, "No se pudo crear rx_queue");
			return;
		}

		ESP_ERROR_CHECK(nimble_port_init());

		ble_svc_gap_init();
		ble_svc_gatt_init();

		int rc = ble_gatts_count_cfg(gatt_svcs);
		if (rc != 0)
		{
			ESP_LOGE(TAG, "ble_gatts_count_cfg failed: %d", rc);
			return;
		}

		rc = ble_gatts_add_svcs(gatt_svcs);
		if (rc != 0)
		{
			ESP_LOGE(TAG, "ble_gatts_add_svcs failed: %d", rc);
			return;
		}

		rc = ble_svc_gap_device_name_set(DEVICE_NAME);
		if (rc != 0)
		{
			ESP_LOGE(TAG, "ble_svc_gap_device_name_set failed: %d", rc);
			return;
		}

		ble_hs_cfg.sync_cb = ble_on_sync;

        nimble_port_freertos_init(host_task);

        xTaskCreatePinnedToCore(bt_input_task,
                                "bt_input",
                                4096,
                                NULL,
                                5,
                                NULL,
                                0);

        ESP_LOGI(TAG, "Bluetooth listo. Esperando conexión...");
	}
	else
	{
		printf("Bt is disabled.\n");
	}
}

void doomEngineTask(void *pvParameters)
{
	printf("doomEngineTask started\n");
	printf("Free heap: %u bytes\n", esp_get_free_heap_size());
	printf("Free internal heap: %u bytes\n",
	       heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

	char const *argv[] = {"doom", "-cout", "ICWEFDA", "nosound", "nomusic", "nosfx", NULL};
	doom_main(6, argv);
}

/* ---- Static stack in PSRAM to avoid internal RAM fragmentation ---- */
EXT_RAM_ATTR static StackType_t doomStack[32768];
static StaticTask_t doomTaskBuffer;

void app_main()
{
	printf("Reset reason: %d\n", esp_reset_reason());

	/* NVS must be initialized BEFORE starting BT controller */
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ESP_ERROR_CHECK(nvs_flash_init());
	}

	SetupBt();

	// Keep system watchdog active (required for BLE stability)

	int i;
	const esp_partition_t *part;
	spi_flash_mmap_handle_t hdoomwad;
	esp_err_t err;

	part = esp_partition_find_first(66, 6, NULL);
	if (part == 0)
		printf("Couldn't find wad part!\n");
	part = esp_partition_find_first(66, 7, NULL);
	if (part == 0)
		printf("Couldn't find prboom wad part!\n");

	printf("app_main: before spi_lcd_init()\n");
	spi_lcd_init();
	printf("app_main: after spi_lcd_init()\n");

	printf("app_main: before jsInit()\n");
	jsInit();
	printf("app_main: after jsInit()\n");

	printf("app_main: before xTaskCreate()\n");

	printf("Heap before Doom task: %u bytes\n", esp_get_free_heap_size());
	printf("Internal heap before Doom task: %u bytes\n",
	       heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

	TaskHandle_t doomHandle = xTaskCreateStaticPinnedToCore(
	    doomEngineTask,
	    "doomEngine",
	    32768,
	    NULL,
	    5,
	    doomStack,
	    &doomTaskBuffer,
	    1);

	if (doomHandle == NULL) {
	    printf("ERROR: Doom static task creation failed!\n");
	} else {
	    printf("Doom static task created successfully\n");
	}

	printf("app_main: after xTaskCreate()\n");

	// Delete main task instead of returning (prevents scheduler side effects)
	vTaskDelete(NULL);
}
