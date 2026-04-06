// GPIO-based Gamepad replacement (Corrected ESP32-S3 pin usage)

#include <stdlib.h>

#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"
#include "d_event.h"
#include "g_game.h"
#include "d_main.h"
#include "gamepad.h"
#include "lprintf.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

int usejoystick = 0;
int joyleft = 0;
int joyright = 0;
int joyup = 0;
int joydown = 0;

/*
ESP32-S3 SAFE GPIO MAP (based on your hardware)

AUDIO:
BCLK  = 17  (DO NOT USE)
WS    = 47  (DO NOT USE)
DOUT  = 15  (DO NOT USE)

LCD:
6,7,5,4,48,45 (DO NOT USE)

FREE USABLE:
1 y 3
*/

// Directions
#define PIN_UP              8
#define PIN_DOWN            9
#define PIN_LEFT            10
#define PIN_RIGHT           11

// Main actions
#define PIN_FIRE            12
#define PIN_USE             2      // moved from 17 (audio conflict)
#define PIN_RUN             16
#define PIN_STRAFE          18

// System
#define PIN_ESCAPE          13
#define PIN_MAP             14
#define PIN_MENU_ENTER      19
#define PIN_PAUSE           20

// Remaining buttons (using truly free pins)
#define PIN_WEAPONTOGGLE    1
#define PIN_STRAFELEFT      3
#define PIN_STRAFERIGHT     46

volatile int joyVal = 0;

typedef struct {
    int ps2mask;
    int *key;
} JsKeyMap;

static const JsKeyMap keymap[] = {
    {0x10,   &key_up},
    {0x40,   &key_down},
    {0x80,   &key_left},
    {0x20,   &key_right},

    {0x4000, &key_use},
    {0x2000, &key_fire},
    {0x2000, &key_menu_enter},
    {0x8000, &key_pause},
    {0x1000, &key_weapontoggle},

    {0x8,    &key_escape},
    {0x1,    &key_map},         

    {0x400,  &key_strafeleft},
    {0x100,  &key_speed},
    {0x800,  &key_straferight},
    {0x200,  &key_strafe},

    {0, NULL},
};

void gamepadPoll(void)
{
    static int oldPollJsVal = 0;
    int newJoyVal = joyVal;
    event_t ev;

    for (int i = 0; keymap[i].key != NULL; i++) {
        if ((oldPollJsVal ^ newJoyVal) & keymap[i].ps2mask) {
            ev.type = (newJoyVal & keymap[i].ps2mask) ? ev_keydown : ev_keyup;
            ev.data1 = *keymap[i].key;
            D_PostEvent(&ev);
        }
    }

    oldPollJsVal = newJoyVal;
}

void gamepadInit(void)
{
    lprintf(LO_INFO, "GPIO gamepad init\n");

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask =
            (1ULL<<PIN_UP) |
            (1ULL<<PIN_DOWN) |
            (1ULL<<PIN_LEFT) |
            (1ULL<<PIN_RIGHT) |
            (1ULL<<PIN_FIRE) |
            (1ULL<<PIN_USE) |
            (1ULL<<PIN_RUN) |
            (1ULL<<PIN_STRAFE) |
            (1ULL<<PIN_ESCAPE) |
            (1ULL<<PIN_MAP) |
            (1ULL<<PIN_MENU_ENTER) |
            (1ULL<<PIN_PAUSE) |
            (1ULL<<PIN_WEAPONTOGGLE) |
            (1ULL<<PIN_STRAFELEFT) |
            (1ULL<<PIN_STRAFERIGHT),
        .pull_down_en = 0,
        .pull_up_en = 1
    };

    gpio_config(&io_conf);
    lprintf(LO_INFO, "GPIO gamepad initialized.\n");
}

static void jsTask(void *arg)
{
    while (1) {
        int newJoyVal = 0;

        if (!gpio_get_level(PIN_UP))              newJoyVal |= 0x10;
        if (!gpio_get_level(PIN_DOWN))            newJoyVal |= 0x40;
        if (!gpio_get_level(PIN_LEFT))            newJoyVal |= 0x80;
        if (!gpio_get_level(PIN_RIGHT))           newJoyVal |= 0x20;

        if (!gpio_get_level(PIN_USE))             newJoyVal |= 0x4000;
        if (!gpio_get_level(PIN_FIRE))            newJoyVal |= 0x2000;
        if (!gpio_get_level(PIN_MENU_ENTER))      newJoyVal |= 0x2000;

        if (!gpio_get_level(PIN_PAUSE))           newJoyVal |= 0x8000;
        if (!gpio_get_level(PIN_WEAPONTOGGLE))    newJoyVal |= 0x1000;

        if (!gpio_get_level(PIN_ESCAPE))          newJoyVal |= 0x8;
        if (!gpio_get_level(PIN_MAP))             newJoyVal |= 0x1;

        if (!gpio_get_level(PIN_STRAFELEFT))      newJoyVal |= 0x400;
        if (!gpio_get_level(PIN_STRAFERIGHT))     newJoyVal |= 0x800;

        if (!gpio_get_level(PIN_STRAFE))          newJoyVal |= 0x200;
        if (!gpio_get_level(PIN_RUN))             newJoyVal |= 0x100;

        joyVal = newJoyVal;

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void jsInit()
{
    gamepadInit();
    xTaskCreatePinnedToCore(jsTask, "js", 4096, NULL, 6, NULL, 0);
}
