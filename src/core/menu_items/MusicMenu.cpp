#include "MusicMenu.h"
#include "core/display.h"
#include "core/mykeyboard.h" // for check(), EscPress
#include "core/sd_functions.h"
#include "core/utils.h"
#include <interface.h>

#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2SNoDAC.h>

// Confirmed wiring: PAM8403 signal pin
#define MUSIC_AUDIO_PIN 22
// Extra buttons (not part of Bruce's standard Prev/Next/Sel/Esc scheme)
#define BTN_VOL_UP   17
#define BTN_VOL_DOWN 16

static AudioFileSourceSD *audioFile = nullptr;
static AudioGeneratorMP3 *mp3 = nullptr;
static AudioOutputI2SNoDAC *audioOut = nullptr;
static float volume = 0.6f; // 0.0 - 1.0

void MusicMenu::optionsMenu() {
    // loopSD runs its own full navigation loop (same one FileMenu uses) and
    // returns the chosen file's path, or "" if the user backed all the way out.
    String picked = loopSD(SD, true, "mp3", "/MUSIC");
    if (picked != "") playScreen(picked);
    addOptionToMainMenu();
}

void MusicMenu::playScreen(String filepath) {
    pinMode(BTN_VOL_UP, INPUT_PULLUP);
    pinMode(BTN_VOL_DOWN, INPUT_PULLUP);

    audioFile = new AudioFileSourceSD(filepath.c_str());
    audioOut = new AudioOutputI2SNoDAC();

    // *** UNVERIFIED: how to bind AudioOutputI2SNoDAC to MUSIC_AUDIO_PIN (22) ***
    // This has changed across ESP8266Audio library versions. After adding the
    // library in platformio.ini, open its examples folder
    // (.pio/libdeps/<env>/ESP8266Audio/examples/) and look for a NoDAC example
    // to confirm the exact call (SetPinout(), a constructor argument, or a
    // direct i2s_set_pin() override). Do not flash until this line is confirmed.
    audioOut->SetGain(volume);

    mp3 = new AudioGeneratorMP3();
    mp3->begin(audioFile, audioOut);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    String name = filepath.substring(filepath.lastIndexOf('/') + 1);
    tft.drawString(name, tftWidth / 2, tftHeight / 2 - 20, 2);
    tft.drawString("Vol: " + String((int)(volume * 100)) + "%", tftWidth / 2, tftHeight / 2 + 10, 2);
    tft.drawString("BACK = stop", tftWidth / 2, tftHeight / 2 + 30, 2);

    bool playing = true;
    while (playing) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) {
                mp3->stop();
                playing = false;
            }
        } else {
            playing = false;
        }

        // Same input pattern used across Bruce's other continuous-loop
        // screens (e.g. IR Jammer's main loop): check(EscPress) reads the
        // BACK button through Bruce's own debounced input system.
        if (check(EscPress)) {
            mp3->stop();
            playing = false;
        }

        // Vol Up/Down are extra buttons outside Bruce's standard 4, so they
        // are read directly rather than through check().
        if (digitalRead(BTN_VOL_UP) == LOW) {
            volume = min(1.0f, volume + 0.02f);
            audioOut->SetGain(volume);
            delay(80);
        }
        if (digitalRead(BTN_VOL_DOWN) == LOW) {
            volume = max(0.0f, volume - 0.02f);
            audioOut->SetGain(volume);
            delay(80);
        }
    }

    delete mp3;
    mp3 = nullptr;
    delete audioOut;
    audioOut = nullptr;
    delete audioFile;
    audioFile = nullptr;
}

void MusicMenu::drawIcon(float scale) {
    clearIconArea();
    int noteH = scale * 50;
    int noteW = max(2, (int)(scale * 6));
    int headR = max(4, (int)(scale * 10));

    int x = iconCenterX - noteW / 2;
    int yTop = iconCenterY - noteH / 2;

    tft.fillRect(x, yTop, noteW, noteH, bruceConfig.priColor);
    tft.fillCircle(x - headR + noteW, yTop + noteH - headR / 2, headR, bruceConfig.priColor);
    tft.fillRect(x, yTop, noteW * 4, noteW, bruceConfig.priColor);
}
