#include "i2s.hpp"
#include <iostream>

void setup_i2s(i2s_port_t i2s_num) {
    i2s_config_t cfg = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 128
    };

    i2s_pin_config_t pcfg = {
        .bck_io_num = 27, // J8
        .ws_io_num = 25, // J10
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 19, // J12
    };

    esp_err_t res;

    res = i2s_driver_install(i2s_num, &cfg, 0, NULL);
    while (res != ESP_OK) {
        std::cout << "failed install of i2s driver" << std::endl;
    }
    res = i2s_set_pin(i2s_num, &pcfg);
    while (res != ESP_OK) {
        std::cout << "failed set pin of i2s driver" << std::endl;
    }
    i2s_set_sample_rates(i2s_num, 44100);
    res = i2s_start(i2s_num);
    while (res != ESP_OK) {
        std::cout << "failed start of i2s driver" << std::endl;
    }
}
