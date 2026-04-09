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


void readCommands(char command) {

    printf("command: '%c'\n", command);

    switch (command) {
      // Movimiento: press
      case 'F':
        moveForward = true;
        lprintf(LO_INFO,"empiezo a moverme hacia adelante");
        break;

      case 'B':
        moveBackward = true;
        lprintf(LO_INFO,"empiezo a moverme hacia atras");
        break;

      case 'L':
        moveLeft = true;
        lprintf(LO_INFO,"empiezo a moverme hacia la izquierda");
        break;

      case 'R':
        moveRight = true;
        lprintf(LO_INFO,"empiezo a moverme hacia la derecha");
        break;

      // Movimiento: release
      case 'f':
        moveForward = false;
        lprintf(LO_INFO,"dejo de moverme hacia adelante");
        break;

      case 'b':
        moveBackward = false;
        lprintf(LO_INFO,"dejo de moverme hacia atras");
        break;

      case 'l':
        moveLeft = false;
        lprintf(LO_INFO,"dejo de moverme hacia la izquierda");
        break;

      case 'r':
        moveRight = false;
        lprintf(LO_INFO,"dejo de moverme hacia la derecha");
        break;

      // Acciones: press
      case 'S':
        lprintf(LO_INFO,"cuadrado");
        break;

      case 'T':
        lprintf(LO_INFO,"triangulo");
        break;

      case 'X':
        lprintf(LO_INFO,"cross");
        break;

      case 'C':
        lprintf(LO_INFO,"circulo");
        break;

      case 'A':
        lprintf(LO_INFO,"start");
        break;

      case 'P':
        lprintf(LO_INFO,"pause");
        break;

      // Acciones: release
      case 's':
        lprintf(LO_INFO,"suelto cuadrado");
        break;

      case 't':
        lprintf(LO_INFO,"suelto triangulo");
        break;

      case 'x':
        lprintf(LO_INFO,"suelto cross");
        break;

      case 'c':
        lprintf(LO_INFO,"suelto circulo");
        break;

      case 'a':
        lprintf(LO_INFO,"suelto start");
        break;

      case 'p':
        lprintf(LO_INFO,"suelto pause");
        break;

      default:
       printf("comando no reconocido: '%c'\n", command);
        break;
    }
 updateMovement();
}
void updateMovement() {
   /*static unsigned long lastMoveMs = 0;
    const unsigned long moveInterval = 200;

    if (millis() - lastMoveMs < moveInterval) {
        return;
    }
    lastMoveMs = millis();*/

    if (moveForward && moveLeft && !moveBackward && !moveRight) {
        lprintf(LO_INFO,"me muevo diagonal adelante-izquierda");
    } else if (moveForward && moveRight && !moveBackward && !moveLeft) {
        lprintf(LO_INFO,"me muevo diagonal adelante-derecha");
    } else if (moveBackward && moveLeft && !moveForward && !moveRight) {
        lprintf(LO_INFO,"me muevo diagonal atras-izquierda");
    } else if (moveBackward && moveRight && !moveForward && !moveLeft) {
        lprintf(LO_INFO,"me muevo diagonal atras-derecha");
    } else if (moveForward && !moveBackward) {
        lprintf(LO_INFO,"me muevo hacia adelante");
    } else if (moveBackward && !moveForward) {
        lprintf(LO_INFO,"me muevo hacia atras");
    } else if (moveLeft && !moveRight) {
        lprintf(LO_INFO,"me muevo hacia la izquierda");
    } else if (moveRight && !moveLeft) {
        lprintf(LO_INFO,"me muevo hacia la derecha");
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
