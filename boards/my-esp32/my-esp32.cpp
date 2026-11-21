#include "core/powerSave.h"
#include "hal/device.h"
#include "hal/inputs/buttons.h"
#include <interface.h>

// Confirmed wiring (your final pin table):
// UP=32, DOWN=33, OK=12, BACK=21, RIGHT=17, LEFT=16, Audio signal=22
#define L_BTN   16
#define R_BTN   17
#define UP_BTN  32
#define DW_BTN  33
#define SEL_BTN 12
#define ESC_BTN 21

static DeviceButtons buttonsCfg() {
    return DeviceButtons{L_BTN, R_BTN, UP_BTN, DW_BTN, SEL_BTN, ESC_BTN};
}

/***************************************************************************************
** Function name: _setup_gpio()
** Location: main.cpp
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio() {
    bruceConfigPins.rotation = 0; // flip to 2 later from the on-device Config menu if upside down

    // SD card bus (confirmed): sck=18, miso=19, mosi=23, cs=5
    bruceConfigPins.SDCARD_bus = {
        (gpio_num_t)18, (gpio_num_t)19, (gpio_num_t)23, (gpio_num_t)5
    };

    // No RF/NRF24/IR hardware wired yet -- left unset on purpose.
    // Add a line like bruceConfigPins.irTx = <pin>; here later if an IR LED is added.

    hal_buttons_init(buttonsCfg(), 6);

    bruceConfig.colorInverted = 0;
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Location: main.cpp
** Description:   second stage gpio setup to make a few functions work
***************************************************************************************/
void _post_setup_gpio() {
    // Backlight is wired straight to 3.3V (no GPIO control), so TFT_BL is -1
    // and there is no pin to set up here.
#if defined(TFT_BL) && (TFT_BL >= 0)
    pinMode(TFT_BL, OUTPUT);
#endif
}

/*********************************************************************
** Function: setBrightness
** location: settings.cpp
** set brightness value
** No-op here: backlight has no GPIO to control (wired straight to 3.3V),
** so brightness is always fixed at full.
**********************************************************************/
void _setBrightness(uint8_t brightval) {
#if defined(TFT_BL) && (TFT_BL >= 0)
    pinMode(TFT_BL, OUTPUT);
    if (brightval > 5) {
        digitalWrite(TFT_BL, LOW);
        digitalWrite(TFT_BL, HIGH);
    } else {
        digitalWrite(TFT_BL, HIGH);
        digitalWrite(TFT_BL, LOW);
    }
#endif
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/
void InputHandler(void) { hal_buttons_poll_6(buttonsCfg()); }

/*********************************************************************
** Function: powerOff
** location: mykeyboard.cpp
** Turns off the device (or try to)
**********************************************************************/
void powerOff() {}

/*********************************************************************
** Function: checkReboot
** location: mykeyboard.cpp
** Btn logic to turn off the device (name is odd btw)
**********************************************************************/
void checkReboot() {}
