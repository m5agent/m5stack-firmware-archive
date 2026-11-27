#include <M5Unified.h>

static constexpr const size_t record_number     = 256;
static constexpr const size_t record_length     = 200;
static constexpr const size_t record_size       = record_number * record_length;
static constexpr const size_t record_samplerate = 16000;

static int16_t *rec_data;
static size_t rec_record_idx  = 0;
static size_t draw_record_idx = 0;
static bool is_recording      = false;
static bool auto_playback     = false;

M5Canvas canvas(&M5.Display);

void drawUI()
{
    const int w = M5.Display.width();
    const int h = M5.Display.height();

    canvas.fillSprite(TFT_WHITE);
    canvas.setTextDatum(top_center);

    // Title
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextColor(TFT_BLACK);
    canvas.drawString("AUDIO RECORDER", w / 2, 14);

    // Instructions
    canvas.setFont(&fonts::FreeSansBold18pt7b);
    canvas.setTextSize(1);
  
    canvas.setTextColor(TFT_RED);
    canvas.drawString("Press Button A", w / 2, h / 2 - 50);

    canvas.setTextColor(TFT_BLUE);
    canvas.drawString("to Record", w / 2, h / 2);

    canvas.setTextColor(TFT_GREEN);
    canvas.drawString("Release to Playback", w / 2, h / 2 + 50);

    canvas.pushSprite(0, 0);
}

void setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setEpdMode(epd_mode_t::epd_quality);

    if (M5.Display.width() > M5.Display.height()) {
        M5.Display.setRotation(M5.Display.getRotation() ^ 1);
    }

    canvas.createSprite(M5.Display.width(), M5.Display.height());
    drawUI();

    // Allocate recording buffer
    rec_data = (int16_t *)heap_caps_malloc(record_size * sizeof(int16_t), MALLOC_CAP_8BIT);
    memset(rec_data, 0, record_size * sizeof(int16_t));

    M5.Speaker.setVolume(255);
    M5.Speaker.end();
    M5.Mic.begin();
}

void loop()
{
    M5.update();

    if (M5.BtnA.wasPressed() && M5.Mic.isEnabled()) {
        is_recording   = true;
        rec_record_idx = 0;
        memset(rec_data, 0, record_size * sizeof(int16_t));
    }

    // If recording
    if (is_recording && M5.Mic.isEnabled()) {
        auto data = &rec_data[rec_record_idx * record_length];
        if (M5.Mic.record(data, record_length, record_samplerate)) {
            if (++rec_record_idx >= record_number) {
                rec_record_idx = 0;
            }
        }

        if (M5.BtnA.wasReleased()) {
            is_recording  = false;
            auto_playback = true;

            while (M5.Mic.isRecording()) {
                M5.delay(1);
            }
            M5.Mic.end();
            M5.Speaker.begin();
        }
    }

    if (auto_playback && M5.Speaker.isEnabled()) {
        M5.Speaker.playRaw(rec_data, record_size, record_samplerate, false, 1, 0);
        do {
            M5.delay(1);
            M5.update();
        } while (M5.Speaker.isPlaying());

        M5.Speaker.end();
        M5.Mic.begin();
        auto_playback = false;
    }

    M5.delay(1);
}
