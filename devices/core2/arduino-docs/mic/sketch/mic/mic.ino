#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>

// Core2 SD card pins
constexpr int SD_SPI_CS_PIN   = 4;
constexpr int SD_SPI_SCK_PIN  = 18;
constexpr int SD_SPI_MISO_PIN = 38;
constexpr int SD_SPI_MOSI_PIN = 23;

// Recording parameters
constexpr size_t RECORD_LENGTH      = 512;
constexpr size_t RECORD_SAMPLE_RATE = 12000;
constexpr size_t RECORD_BUFFER_COUNT = 3;

static int16_t recordBuffer[RECORD_BUFFER_COUNT][RECORD_LENGTH];
static File wavFile;

static bool isRecording = false;
static bool recordingError = false;

static size_t submitIndex = 0;
static size_t writeIndex = 0;
static size_t submittedBlocks = 0;
static size_t writtenBlocks = 0;
static size_t totalSamples = 0;

static int32_t displayWidth = 0;

struct __attribute__((packed)) WavHeader {
  char riff[4];
  uint32_t fileSize;
  char wave[4];
  char fmt[4];
  uint32_t fmtSize;
  uint16_t format;
  uint16_t channels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  char data[4];
  uint32_t dataSize;
};

bool writeWavHeader(File& file, uint32_t pcmBytes) {
  WavHeader header = {};

  memcpy(header.riff, "RIFF", 4);
  memcpy(header.wave, "WAVE", 4);
  memcpy(header.fmt, "fmt ", 4);
  memcpy(header.data, "data", 4);

  header.fileSize = 36 + pcmBytes;
  header.fmtSize = 16;
  header.format = 1;
  header.channels = 1;
  header.sampleRate = RECORD_SAMPLE_RATE;
  header.byteRate = RECORD_SAMPLE_RATE * sizeof(int16_t);
  header.blockAlign = sizeof(int16_t);
  header.bitsPerSample = 16;
  header.dataSize = pcmBytes;

  if (!file.seek(0)) {
    return false;
  }

  return file.write(
           reinterpret_cast<const uint8_t*>(&header),
           sizeof(header)
         ) == sizeof(header);
}

void showRecordingInfo() {
  char info[64];

  float seconds =
      static_cast<float>(totalSamples) / RECORD_SAMPLE_RATE;

  uint32_t pcmBytes =
      static_cast<uint32_t>(totalSamples * sizeof(int16_t));

  snprintf(
      info,
      sizeof(info),
      "Time: %.2fs  Size: %luKB",
      seconds,
      static_cast<unsigned long>(pcmBytes / 1024)
  );

  M5.Display.fillRect(0, 105, displayWidth, 32, TFT_WHITE);
  M5.Display.drawCenterString(info, displayWidth / 2, 110);
}

// Write the oldest completed block to the SD card.
bool writeCompletedBlock() {
  if (!wavFile || recordingError) {
    return false;
  }

  const size_t bytes = RECORD_LENGTH * sizeof(int16_t);

  if (wavFile.write(
          reinterpret_cast<const uint8_t*>(recordBuffer[writeIndex]),
          bytes
      ) != bytes) {
    recordingError = true;
    return false;
  }

  totalSamples += RECORD_LENGTH;
  writtenBlocks++;
  writeIndex = (writeIndex + 1) % RECORD_BUFFER_COUNT;

  return true;
}

// M5.Mic.record() is asynchronous.
// Three buffers allow the microphone to fill one buffer while another
// buffer is being written to the SD card.
bool captureOneBlock() {
  if (!M5.Mic.record(
          recordBuffer[submitIndex],
          RECORD_LENGTH,
          RECORD_SAMPLE_RATE,
          false
      )) {
    recordingError = true;
    return false;
  }

  submitIndex = (submitIndex + 1) % RECORD_BUFFER_COUNT;
  submittedBlocks++;

  // M5Unified uses two internal recording slots.
  // After submitting the third block, the first block is complete.
  if (submittedBlocks >= RECORD_BUFFER_COUNT) {
    if (!writeCompletedBlock()) {
      return false;
    }

    showRecordingInfo();
  }

  return true;
}

void startRecording() {
  if (SD.exists("/recording.wav")) {
    if (!SD.remove("/recording.wav")) {
      M5.Display.drawCenterString(
          "Delete Old File Error!",
          displayWidth / 2,
          100
      );
      return;
    }
  }

  wavFile = SD.open("/recording.wav", FILE_WRITE);

  if (!wavFile) {
    M5.Display.drawCenterString(
        "Create WAV File Error!",
        displayWidth / 2,
        100
    );
    return;
  }

  submitIndex = 0;
  writeIndex = 0;
  submittedBlocks = 0;
  writtenBlocks = 0;
  totalSamples = 0;
  recordingError = false;

  if (!writeWavHeader(wavFile, 0)) {
    wavFile.close();
    wavFile = File();

    M5.Display.drawCenterString(
        "Write WAV Header Error!",
        displayWidth / 2,
        100
    );
    return;
  }

  isRecording = true;

  M5.Display.fillRect(0, 100, displayWidth, 100, TFT_WHITE);
  M5.Display.drawCenterString(
      "Recording...",
      displayWidth / 2,
      155
  );
}

void stopRecording() {
  isRecording = false;

  // Wait until all queued microphone buffers are complete.
  while (M5.Mic.isRecording() != 0) {
    M5.delay(1);
  }

  // Write the remaining buffers in order.
  while (!recordingError && writtenBlocks < submittedBlocks) {
    if (!writeCompletedBlock()) {
      break;
    }
  }

  if (wavFile) {
    wavFile.flush();

    uint32_t pcmBytes =
        static_cast<uint32_t>(totalSamples * sizeof(int16_t));

    bool headerOK = writeWavHeader(wavFile, pcmBytes);

    wavFile.flush();
    wavFile.close();
    wavFile = File();

    float seconds =
        static_cast<float>(totalSamples) / RECORD_SAMPLE_RATE;

    char info[64];

    if (!headerOK) {
      snprintf(info, sizeof(info), "WAV Header Error!");
    } else if (recordingError) {
      snprintf(
          info,
          sizeof(info),
          "Partial WAV: %.2fs",
          seconds
      );
    } else {
      snprintf(
          info,
          sizeof(info),
          "WAV Saved: %.2fs %luKB",
          seconds,
          static_cast<unsigned long>(pcmBytes / 1024)
      );
    }

    M5.Display.fillRect(0, 100, displayWidth, 100, TFT_WHITE);
    M5.Display.drawCenterString(
        info,
        displayWidth / 2,
        155
    );
  }
}

void setup() {
  auto cfg = M5.config();

  // The speaker and microphone share the I2S peripheral.
  cfg.internal_spk = false;
  cfg.internal_mic = true;

  M5.begin(cfg);

  M5.Display.setRotation(1);
  displayWidth = M5.Display.width();

  M5.Display.setTextDatum(top_center);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Display.setFont(&fonts::FreeSansBoldOblique9pt7b);
  M5.Display.clear(TFT_WHITE);

  SPI.begin(
      SD_SPI_SCK_PIN,
      SD_SPI_MISO_PIN,
      SD_SPI_MOSI_PIN,
      SD_SPI_CS_PIN
  );

  M5.Display.drawCenterString(
      "SD Initializing...",
      displayWidth / 2,
      10
  );

  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    M5.Display.drawCenterString(
        "SD Init Error!",
        displayWidth / 2,
        55
    );

    while (true) {
      M5.delay(1000);
    }
  }

  // Keep the Core2 auto-detected microphone pins.
  auto micConfig = M5.Mic.config();
  micConfig.sample_rate = RECORD_SAMPLE_RATE;
  micConfig.over_sampling = 1;
  M5.Mic.config(micConfig);

  if (!M5.Mic.isEnabled() || !M5.Mic.begin()) {
    M5.Display.drawCenterString(
        "Microphone Error!",
        displayWidth / 2,
        55
    );

    while (true) {
      M5.delay(1000);
    }
  }

  M5.Display.clear(TFT_WHITE);
  M5.Display.drawCenterString(
      "Core2 MIC Recorder",
      displayWidth / 2,
      10
  );
  M5.Display.drawCenterString(
      "A: Start    C: Stop",
      displayWidth / 2,
      45
  );
}

void loop() {
  M5.update();

  if (M5.BtnA.wasClicked() && !isRecording) {
    startRecording();
  }

  if (M5.BtnC.wasClicked() && isRecording) {
    stopRecording();
    return;
  }

  if (isRecording && wavFile) {
    if (!captureOneBlock()) {
      stopRecording();
    }
  }
}
