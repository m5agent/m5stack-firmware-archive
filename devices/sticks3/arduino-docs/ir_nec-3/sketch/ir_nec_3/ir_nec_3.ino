#include "M5Unified.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"

#define IR_RECEIVE_PIN 42
#define IR_SEND_PIN    46

rmt_channel_handle_t rx_chan = NULL;
rmt_channel_handle_t tx_chan = NULL;
rmt_encoder_handle_t copy_encoder = NULL;

static rmt_symbol_word_t rx_raw_symbols[256];
static volatile bool rx_done = false;
static size_t rx_symbol_num = 0;

#define IR_CARRIER_FREQ_HZ  38000
#define IR_DUTY_CYCLE       0.33

/* ================= Dump level data================= */
void dump_pulse_array(rmt_symbol_word_t *symbols, size_t count) {
    Serial.println("IR PULSE (us):(L0 | L1)");

    for (size_t i = 0; i < count; i++) {
        int32_t p0 = symbols[i].level0 ? symbols[i].duration0 : -symbols[i].duration0;
        int32_t p1 = symbols[i].level1 ? symbols[i].duration1 : -symbols[i].duration1;
        Serial.printf(" %d, %d\n", p0, p1);
    }
    Serial.println();
}

void dump_duration_array(rmt_symbol_word_t *symbols, size_t count) {
    Serial.println("IR DURATIONS (us):(L0 | L1)");

    for (size_t i = 0; i < count; i++) {
        Serial.printf(" %d, %d\n", symbols[i].duration0, symbols[i].duration1);
    }
    Serial.println();
}

void dump_raw_hex(rmt_symbol_word_t *symbols, size_t count) {
    Serial.println("IR RAW HEX:(L0 | L1)");

    for (size_t i = 0; i < count; i++) {
        Serial.printf(" %04X %04X\n", symbols[i].duration0, symbols[i].duration1);
    }
    Serial.println();
}

bool rmt_rx_done_callback(rmt_channel_handle_t, const rmt_rx_done_event_data_t *edata, void *) {
    rx_symbol_num = edata->num_symbols;
    rx_done = true;
    return true;
}

void setup_rmt_rx() {
    rmt_rx_channel_config_t rx_cfg = {
        .gpio_num = (gpio_num_t)IR_RECEIVE_PIN,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .mem_block_symbols = 128,
    };

    if (rmt_new_rx_channel(&rx_cfg, &rx_chan) != ESP_OK) return;

    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = rmt_rx_done_callback,
    };

    rmt_rx_register_event_callbacks(rx_chan, &cbs, NULL);
    rmt_enable(rx_chan);
}

void start_rmt_receive() {
    rmt_receive_config_t cfg = {
        .signal_range_min_ns = 1000,
        .signal_range_max_ns = 20000000,
    };

    rmt_receive(rx_chan, rx_raw_symbols, sizeof(rx_raw_symbols), &cfg);
}

void setup_rmt_tx() {
    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = (gpio_num_t)IR_SEND_PIN,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .mem_block_symbols = 128,
        .trans_queue_depth = 4,
    };

    rmt_new_tx_channel(&tx_cfg, &tx_chan);

    rmt_carrier_config_t carrier_cfg = {
        .frequency_hz = IR_CARRIER_FREQ_HZ,
        .duty_cycle = IR_DUTY_CYCLE,
    };

    rmt_apply_carrier(tx_chan, &carrier_cfg);

    rmt_copy_encoder_config_t enc_cfg = {};
    rmt_new_copy_encoder(&enc_cfg, &copy_encoder);

    rmt_enable(tx_chan);
}

void send_raw_symbols(rmt_symbol_word_t *symbols, size_t count) {
    rmt_transmit_config_t cfg = { .loop_count = 0 };
    rmt_transmit( tx_chan, copy_encoder, symbols, count * sizeof(rmt_symbol_word_t), &cfg);
    rmt_tx_wait_all_done(tx_chan, 1000);
    Serial.printf("Transmit back OK!\n\n");
}

void setup() {
    M5.begin();
    M5.Speaker.end();// Disable speaker amp to avoid IR RX interference
    Serial.begin(115200);

    // Display initialization
    M5.Display.setRotation(3);
    M5.Display.setFont(&fonts::FreeMonoBold9pt7b);
    M5.Display.clear();
    M5.Display.println("StickS3 IR example");
    M5.Display.setCursor(0, 30);
    M5.Display.println("Waiting for NEC...");

    setup_rmt_rx();
    setup_rmt_tx();
    start_rmt_receive();

    // Enable external power output for IR receiver module
    M5.Power.setExtOutput(true, m5::ext_none);
}

void loop() {
    if (rx_done) {
        rx_done = false;

        Serial.printf("RX symbols num: %d\n", rx_symbol_num);
        dump_pulse_array(rx_raw_symbols, rx_symbol_num);
        dump_duration_array(rx_raw_symbols, rx_symbol_num);
        dump_raw_hex(rx_raw_symbols, rx_symbol_num);

        M5.Display.fillRect(0, 30, 240, 105, TFT_BLACK);
        M5.Display.setCursor(0, 30);
        M5.Display.setTextColor(GREEN);
        M5.Display.printf("Received!\nsymbols num: %d\nDetails see Serial", rx_symbol_num);

        send_raw_symbols(rx_raw_symbols, rx_symbol_num);
        start_rmt_receive();
    }
    delay(10);
}
