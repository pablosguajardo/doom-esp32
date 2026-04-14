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

// FOR BT:
#include "sdkconfig.h"

#define JOYSTICK_OPTION CONFIG_HW_JOYSTICK_OPTION

int usejoystick = 0;
int joyleft = 0;
int joyright = 0;
int joyup = 0;
int joydown = 0;

bool moveForward = false;
bool moveBackward = false;
bool moveLeft = false;
bool moveRight = false;

/* ================= BT INPUT MODE ================= */

#define BT_INPUT_MODE_HOLD 1
#define BT_INPUT_MODE_TAP 2

#ifndef BT_INPUT_MODE
#define BT_INPUT_MODE BT_INPUT_MODE_HOLD /* Default = TAP mode */
#endif

#define BT_TAP_FORWARD_TICS 2
#define BT_TAP_SIDE_TICS 1
static int tapButtonTicks = 1;

static int tapForwardTicks = 0;
static int tapBackwardTicks = 0;
static int tapLeftTicks = 0;
static int tapRightTicks = 0;

#define TAP_INTERVAL_MS 250

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
#define PIN_UP 8
#define PIN_DOWN 9
#define PIN_LEFT 10
#define PIN_RIGHT 11

// Main actions
#define PIN_FIRE 12
#define PIN_USE 2 // moved from 17 (audio conflict)
#define PIN_RUN 16
#define PIN_STRAFE 18

// System
#define PIN_ESCAPE 13
#define PIN_MAP 14
#define PIN_MENU_ENTER 19
#define PIN_PAUSE 20

// Remaining buttons (using truly free pins)
#define PIN_WEAPONTOGGLE 1
#define PIN_STRAFELEFT 3
#define PIN_STRAFERIGHT 46

volatile int joyVal = 0;

typedef struct
{
    int ps2mask;
    int *key;
} JsKeyMap;

static const JsKeyMap keymap[] = {
    {0x10, &key_up},
    {0x40, &key_down},
    {0x80, &key_left},
    {0x20, &key_right},

    {0x4000, &key_use},
    {0x2000, &key_fire},
    {0x2000, &key_menu_enter},
    {0x8000, &key_pause},
    {0x1000, &key_weapontoggle},

    {0x8, &key_escape},
    {0x1, &key_map},

    {0x400, &key_strafeleft},
    {0x100, &key_speed},
    {0x800, &key_straferight},
    {0x200, &key_strafe},

    {0, NULL},
};

void processTap(int ticks, int key)
{
    printf("(%d) down\n", key);
    event_t ev;
    ev.type = ev_keydown;
    ev.data1 = key;
    D_PostEvent(&ev);

    // TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(ticks * TAP_INTERVAL_MS);
    TickType_t start = xTaskGetTickCount();
    TickType_t waitTicks = pdMS_TO_TICKS(ticks * TAP_INTERVAL_MS);

    printf("start=%lu waitTicks=%lu\n", (unsigned long)start, (unsigned long)waitTicks);

    TickType_t now;
    bool cont = true;
    while (cont)
    {
        // if ((xTaskGetTickCount() - waitTicks) >= 0)
        now = xTaskGetTickCount();

        if ((now - start) >= waitTicks)
        {

            cont = false;
            ev.type = ev_keyup;
            ev.data1 = key;
            D_PostEvent(&ev);

            printf("up: (%d) ", key);
            printf("start=%lu waitTicks=%lunow=%lu\n", (unsigned long)start, (unsigned long)waitTicks, (unsigned long)now);
        }
        vTaskDelay(1);
    }
}
void readCommands(char command)
{
    event_t ev;
    printf("command: '%c' ", command);

    switch (command)
    {
#if BT_INPUT_MODE == BT_INPUT_MODE_TAP

        /* TAP MODE: impulse movement */

    case 'F':
        tapForwardTicks = BT_TAP_FORWARD_TICS;
        processTap(tapForwardTicks, key_up);
        break;

    case 'T':
        tapBackwardTicks = BT_TAP_FORWARD_TICS;
        processTap(tapBackwardTicks, key_down);
        break;

    case 'L':
        tapLeftTicks = BT_TAP_SIDE_TICS;
        processTap(tapLeftTicks, key_left);
        break;

    case 'R':
        tapRightTicks = BT_TAP_SIDE_TICS;
        processTap(tapRightTicks, key_right);
        break;
    // Acciones: press
    /* Fire */
    case 'D':
        lprintf(LO_INFO, "Fire");
        // ev.type = ev_keydown;
        // ev.data1 = key_fire;
        // D_PostEvent(&ev);
        // ev.type = ev_keyup;
        // D_PostEvent(&ev);
        processTap(tapButtonTicks, key_fire);
        break;

    case 'A':
        lprintf(LO_INFO, "triangulo");
        // key_weapontoggle
        // ev.type = ev_keydown;
        // ev.data1 = key_weapontoggle;
        // D_PostEvent(&ev);
        // ev.type = ev_keyup;
        // D_PostEvent(&ev);
        processTap(tapButtonTicks, key_weapontoggle);
        break;

    case 'C':
        lprintf(LO_INFO, "cross");
        // key_use
        // ev.type = ev_keydown;
        // ev.data1 = key_use;
        // D_PostEvent(&ev);
        // ev.type = ev_keyup;
        // D_PostEvent(&ev);
        processTap(tapButtonTicks, key_use);
        break;

    case 'B':
        lprintf(LO_INFO, "circulo");
        // key_escape
        // ev.type = ev_keydown;
        // ev.data1 = key_escape;
        // D_PostEvent(&ev);
        // ev.type = ev_keyup;
        // D_PostEvent(&ev);
        processTap(tapButtonTicks, key_escape);
        break;

    case 'S':
        lprintf(LO_INFO, "start");
        // key_menu_enter
        // ev.type = ev_keydown;
        // ev.data1 = key_menu_enter;
        // D_PostEvent(&ev);
        // ev.type = ev_keyup;
        // D_PostEvent(&ev);
        processTap(tapButtonTicks, key_menu_enter);
        break;

    case 'P':
        lprintf(LO_INFO, "Pause");
        // ev.type = ev_keydown;
        // ev.data1 = key_pause;
        // D_PostEvent(&ev);
        // ev.type = ev_keyup;
        // D_PostEvent(&ev);

        processTap(tapButtonTicks, key_pause);
        break;

#else

        // HOLD MODE (original behavior)
        /* Forward */
    case 'F':
        moveForward = true;
        lprintf(LO_INFO, "empiezo a moverme hacia adelante");
        break;
        /* towards the rear */
    case 'T':
        lprintf(LO_INFO, "empiezo a moverme hacia atras");
        moveBackward = true;
        break;
        /* Left */
    case 'L':
        moveLeft = true;
        lprintf(LO_INFO, "empiezo a moverme hacia la izquierda");
        break;
        /* Right */
    case 'R':
        moveRight = true;
        lprintf(LO_INFO, "empiezo a moverme hacia la derecha");
        break;

    // MOV: release
    case 'f':
        moveForward = false;
        lprintf(LO_INFO, "dejo de moverme hacia adelante");
        ev.type = ev_keyup;
        ev.data1 = key_up;
        D_PostEvent(&ev);
        break;

    case 't':
        moveBackward = false;
        lprintf(LO_INFO, "dejo de moverme hacia atras");
        ev.type = ev_keyup;
        ev.data1 = key_down;
        D_PostEvent(&ev);
        break;

    case 'l':
        moveLeft = false;
        lprintf(LO_INFO, "dejo de moverme hacia la izquierda");
        ev.type = ev_keyup;
        ev.data1 = key_left;
        D_PostEvent(&ev);
        break;

    case 'r':
        moveRight = false;
        lprintf(LO_INFO, "dejo de moverme hacia la derecha");
        ev.type = ev_keyup;
        ev.data1 = key_right;
        D_PostEvent(&ev);
        break;

    // Acciones: press
    /* Fire */
    case 'D':
        lprintf(LO_INFO, "Fire");
        ev.type = ev_keydown;
        ev.data1 = key_fire;
        D_PostEvent(&ev);
        break;

    case 'A':
        lprintf(LO_INFO, "triangulo");
        // key_weapontoggle
        ev.type = ev_keydown;
        ev.data1 = key_weapontoggle;
        D_PostEvent(&ev);
        break;

    case 'C':
        lprintf(LO_INFO, "cross");
        // key_use
        ev.type = ev_keydown;
        ev.data1 = key_use;
        D_PostEvent(&ev);
        break;

    case 'B':
        lprintf(LO_INFO, "circulo");
        // key_escape
        ev.type = ev_keydown;
        ev.data1 = key_escape;
        D_PostEvent(&ev);
        break;

    case 'S':
        lprintf(LO_INFO, "start");
        // key_menu_enter
        ev.type = ev_keydown;
        ev.data1 = key_menu_enter;
        D_PostEvent(&ev);
        break;

    case 'P':
        lprintf(LO_INFO, "Pause");
        ev.type = ev_keydown;
        ev.data1 = key_pause;
        D_PostEvent(&ev);
        break;

    case 'X':
        lprintf(LO_INFO, "strafeleft");
        // key_menu_enter
        ev.type = ev_keydown;
        ev.data1 = key_strafeleft;
        D_PostEvent(&ev);
        break;

    case 'Y':
        lprintf(LO_INFO, "straferight");
        ev.type = ev_keydown;
        ev.data1 = key_straferight;
        D_PostEvent(&ev);
        break;

    case 'Z':
        lprintf(LO_INFO, "straferight");
        ev.type = ev_keydown;
        ev.data1 = key_speed;
        D_PostEvent(&ev);
        break;

    // Acciones: release
    case 'd':
        lprintf(LO_INFO, "suelto cuadrado");
        ev.type = ev_keyup;
        ev.data1 = key_fire;
        D_PostEvent(&ev);
        break;

    case 'a':
        lprintf(LO_INFO, "suelto triangulo");
        ev.type = ev_keyup;
        ev.data1 = key_weapontoggle;
        D_PostEvent(&ev);
        break;

    case 'c':
        lprintf(LO_INFO, "suelto cross");
        ev.type = ev_keyup;
        ev.data1 = key_use;
        D_PostEvent(&ev);
        break;

    case 'b':
        lprintf(LO_INFO, "suelto circulo");
        ev.type = ev_keyup;
        ev.data1 = key_escape;
        D_PostEvent(&ev);
        break;

    case 's':
        lprintf(LO_INFO, "suelto start");
        ev.type = ev_keyup;
        ev.data1 = key_menu_enter;
        D_PostEvent(&ev);
        break;

    case 'p':
        lprintf(LO_INFO, "suelto pause");
        ev.type = ev_keyup;
        ev.data1 = key_pause;
        D_PostEvent(&ev);
        break;

     case 'x':
        lprintf(LO_INFO, "strafeleft end");
        // key_menu_enter
        ev.type = ev_keyup;
        ev.data1 = key_strafeleft;
        D_PostEvent(&ev);
        break;

    case 'y':
        lprintf(LO_INFO, "straferight  end");
        ev.type = ev_keyup;
        ev.data1 = key_straferight;
        D_PostEvent(&ev);
        break;

    case 'z':
        lprintf(LO_INFO, "straferight  end");
        ev.type = ev_keyup;
        ev.data1 = key_speed;
        D_PostEvent(&ev);
        break;

#endif

    default:
        printf("comando no reconocido: '%c'\n", command);
        break;
    }

#if BT_INPUT_MODE == BT_INPUT_MODE_HOLD
    updateMovement();
#endif
}
void updateMovement()
{
    event_t ev;

    // #if BT_INPUT_MODE == BT_INPUT_MODE_TAP

    /*static unsigned long lastMoveMs = 0;
     const unsigned long moveInterval = 200;

     if (millis() - lastMoveMs < moveInterval) {
         return;
     }
     lastMoveMs = millis();*/

    if (moveForward && moveLeft && !moveBackward && !moveRight)
    {
        lprintf(LO_INFO, "me muevo diagonal adelante-izquierda");
        ev.type = ev_keydown;
        ev.data1 = key_up;
        D_PostEvent(&ev);
        ev.type = ev_keydown;
        ev.data1 = key_left;
        D_PostEvent(&ev);
    }
    else if (moveForward && moveRight && !moveBackward && !moveLeft)
    {
        lprintf(LO_INFO, "me muevo diagonal adelante-derecha");
        ev.type = ev_keydown;
        ev.data1 = key_up;
        D_PostEvent(&ev);
        ev.type = ev_keydown;
        ev.data1 = key_right;
        D_PostEvent(&ev);
    }
    else if (moveBackward && moveLeft && !moveForward && !moveRight)
    {
        lprintf(LO_INFO, "me muevo diagonal atras-izquierda");
        ev.type = ev_keydown;
        ev.data1 = key_down;
        D_PostEvent(&ev);
        ev.type = ev_keydown;
        ev.data1 = key_left;
        D_PostEvent(&ev);
    }
    else if (moveBackward && moveRight && !moveForward && !moveLeft)
    {
        lprintf(LO_INFO, "me muevo diagonal atras-derecha");
        ev.type = ev_keydown;
        ev.data1 = key_down;
        D_PostEvent(&ev);
        ev.type = ev_keydown;
        ev.data1 = key_right;
        D_PostEvent(&ev);
    }
    else if (moveForward && !moveBackward)
    {
        lprintf(LO_INFO, "me muevo hacia adelante");
        ev.type = ev_keydown;
        ev.data1 = key_up;
        D_PostEvent(&ev);
    }
    else if (moveBackward && !moveForward)
    {
        lprintf(LO_INFO, "me muevo hacia atras");
        ev.type = ev_keydown;
        ev.data1 = key_down;
        D_PostEvent(&ev);
    }
    else if (moveLeft && !moveRight)
    {
        lprintf(LO_INFO, "me muevo hacia la izquierda");
        ev.type = ev_keydown;
        ev.data1 = key_left;
        D_PostEvent(&ev);
    }
    else if (moveRight && !moveLeft)
    {
        lprintf(LO_INFO, "me muevo hacia la derecha");
        ev.type = ev_keydown;
        ev.data1 = key_right;
        D_PostEvent(&ev);
    }
}

void gamepadPoll(void)
{
    static int oldPollJsVal = 0;
    int newJoyVal = joyVal;
    event_t ev;

    for (int i = 0; keymap[i].key != NULL; i++)
    {
        if ((oldPollJsVal ^ newJoyVal) & keymap[i].ps2mask)
        {
            ev.type = (newJoyVal & keymap[i].ps2mask) ? ev_keydown : ev_keyup;
            ev.data1 = *keymap[i].key;
            D_PostEvent(&ev);
        }
    }

    oldPollJsVal = newJoyVal;
}

void gamepadInit(void)
{
    if (JOYSTICK_OPTION == 2)
    {

        lprintf(LO_INFO, "Bt joystick init\n");

        lprintf(LO_INFO, "Bt joystick initialized.\n");
    }
    else
    {
        lprintf(LO_INFO, "GPIO gamepad init\n");

        gpio_config_t io_conf = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask =
                (1ULL << PIN_UP) |
                (1ULL << PIN_DOWN) |
                (1ULL << PIN_LEFT) |
                (1ULL << PIN_RIGHT) |
                (1ULL << PIN_FIRE) |
                (1ULL << PIN_USE) |
                (1ULL << PIN_RUN) |
                (1ULL << PIN_STRAFE) |
                (1ULL << PIN_ESCAPE) |
                (1ULL << PIN_MAP) |
                (1ULL << PIN_MENU_ENTER) |
                (1ULL << PIN_PAUSE) |
                (1ULL << PIN_WEAPONTOGGLE) |
                (1ULL << PIN_STRAFELEFT) |
                (1ULL << PIN_STRAFERIGHT),
            .pull_down_en = 0,
            .pull_up_en = 1};

        gpio_config(&io_conf);
        lprintf(LO_INFO, "GPIO gamepad initialized.\n");
    }
}

static void jsTask(void *arg)
{
    while (1)
    {
        int newJoyVal = 0;

        if (!gpio_get_level(PIN_UP))
            newJoyVal |= 0x10;
        if (!gpio_get_level(PIN_DOWN))
            newJoyVal |= 0x40;
        if (!gpio_get_level(PIN_LEFT))
            newJoyVal |= 0x80;
        if (!gpio_get_level(PIN_RIGHT))
            newJoyVal |= 0x20;

        if (!gpio_get_level(PIN_USE))
            newJoyVal |= 0x4000;
        if (!gpio_get_level(PIN_FIRE))
            newJoyVal |= 0x2000;
        if (!gpio_get_level(PIN_MENU_ENTER))
            newJoyVal |= 0x2000;

        if (!gpio_get_level(PIN_PAUSE))
            newJoyVal |= 0x8000;
        if (!gpio_get_level(PIN_WEAPONTOGGLE))
            newJoyVal |= 0x1000;

        if (!gpio_get_level(PIN_ESCAPE))
            newJoyVal |= 0x8;
        if (!gpio_get_level(PIN_MAP))
            newJoyVal |= 0x1;

        if (!gpio_get_level(PIN_STRAFELEFT))
            newJoyVal |= 0x400;
        if (!gpio_get_level(PIN_STRAFERIGHT))
            newJoyVal |= 0x800;

        if (!gpio_get_level(PIN_STRAFE))
            newJoyVal |= 0x200;
        if (!gpio_get_level(PIN_RUN))
            newJoyVal |= 0x100;

        joyVal = newJoyVal;

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void jsInit()
{
    gamepadInit();
    if (JOYSTICK_OPTION == 2)
    {

        lprintf(LO_INFO, "Bt jsInit init\n");

        lprintf(LO_INFO, "Bt jsInit end.\n");
    }
    else
    {
        lprintf(LO_INFO, "GPIO gamepad jsInit init\n");
        xTaskCreatePinnedToCore(jsTask, "js", 4096, NULL, 6, NULL, 0);
        lprintf(LO_INFO, "GPIO gamepad jsInit end\n");
    }
}
