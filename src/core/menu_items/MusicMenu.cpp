#include "MusicMenu.h"
#include "core/display.h"
#include "core/mykeyboard.h" // for check(), NextPress/PrevPress/EscPress
#include "core/sd_functions.h"
#include "core/utils.h"
#include <interface.h>

#include <AudioFileSourceSD.h>
#include <AudioFileSourceBuffer.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2SNoDAC.h>

static AudioFileSourceSD *audioFile = nullptr;
// Reads ahead into RAM so a slow/jittery SD read doesn't starve playback
// mid-stream -- this is what actually fixes stuttering, not a GPIO change.
static AudioFileSourceBuffer *audioBuffer = nullptr;
static AudioGeneratorMP3 *mp3 = nullptr;
static AudioOutputI2SNoDAC *audioOut = nullptr;
static float volume = 0.6f; // 0.0 - 1.0
static constexpr size_t AUDIO_BUFFER_BYTES = 4096; // try 8192 if still stuttering

void MusicMenu::optionsMenu() {
    // loopSD runs its own full navigation loop (same one FileMenu uses) and
    // returns the chosen file's path, or "" if the user backed all the way out.
    String picked = loopSD(SD, true, "mp3", "/MUSIC");
    if (picked != "") playScreen(picked);
    addOptionToMainMenu();
}

void MusicMenu::playScreen(String filepath) {
    audioFile = new AudioFileSourceSD(filepath.c_str());
    audioBuffer = new AudioFileSourceBuffer(audioFile, AUDIO_BUFFER_BYTES);
    audioOut = new AudioOutputI2SNoDAC();

    // AudioOutputI2SNoDAC still needs all 3 I2S pins via SetPinout(bclk,
    // wclk, dout), even though it bit-bangs a simulated bitstream instead
    // of using a real I2S DAC chip. BCLK (0) and WCLK (15) are unused dummy
    // pins -- nothing is wired to them, they just have to be valid GPIO
    // numbers for the call to succeed. DOUT (22) is the real, confirmed
    // audio signal pin going to the PAM8403.
    audioOut->SetPinout(0, 15, 22);
    audioOut->SetGain(volume);

    mp3 = new AudioGeneratorMP3();
    mp3->begin(audioBuffer, audioOut);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    String name = filepath.substring(filepath.lastIndexOf('/') + 1);
    tft.drawString(name, tftWidth / 2, tftHeight / 2 - 20, 2);
    tft.drawString("Vol: " + String((int)(volume * 100)) + "%", tftWidth / 2, tftHeight / 2 + 10, 2);
    tft.drawString("UP/DOWN=Vol  BACK=Stop", tftWidth / 2, tftHeight / 2 + 30, 2);

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

        // Same input pattern as every other continuous-loop screen in
        // Bruce -- no raw GPIO reads needed, it's all native check() calls
        // now that the board has 6 real physical buttons.
        if (check(EscPress)) {
            mp3->stop();
            playing = false;
        }
        if (check(NextPress)) { // physical UP button
            volume = min(1.0f, volume + 0.05f);
            audioOut->SetGain(volume);
            tft.fillRect(0, tftHeight / 2, tftWidth, 20, TFT_BLACK);
            tft.drawString("Vol: " + String((int)(volume * 100)) + "%", tftWidth / 2, tftHeight / 2 + 10, 2);
            delay(120);
        }
        if (check(PrevPress)) { // physical DOWN button
            volume = max(0.0f, volume - 0.05f);
            audioOut->SetGain(volume);
            tft.fillRect(0, tftHeight / 2, tftWidth, 20, TFT_BLACK);
            tft.drawString("Vol: " + String((int)(volume * 100)) + "%", tftWidth / 2, tftHeight / 2 + 10, 2);
            delay(120);
        }
    }

    delete mp3;
    mp3 = nullptr;
    delete audioOut;
    audioOut = nullptr;
    delete audioBuffer;
    audioBuffer = nullptr;
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
