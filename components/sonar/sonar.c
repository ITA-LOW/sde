#include "sonar.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "SONAR";

#define SONAR_TRIG_GPIO  GPIO_NUM_27
#define SONAR_ECHO_GPIO  GPIO_NUM_34

// Tempo máximo de espera por pulso de eco (em us)
#define SONAR_MAX_US     30000  

void sonar_init(void) {
    // TRIG como saída
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << SONAR_TRIG_GPIO,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // ECHO como entrada
    io_conf.pin_bit_mask = 1ULL << SONAR_ECHO_GPIO;
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    // Garante TRIG em nível baixo
    gpio_set_level(SONAR_TRIG_GPIO, 0);
}

float sonar_get_distance_cm(void) {
    // 1) Envia pulso TRIG de 10 us
    gpio_set_level(SONAR_TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(SONAR_TRIG_GPIO, 0);

    // 2) Aguarda começo do pulso ECHO
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(SONAR_ECHO_GPIO) == 0) {
        if (esp_timer_get_time() - start > SONAR_MAX_US) {
            ESP_LOGW(TAG, "Echo start timeout");
            return -1.0f;
        }
    }

    // 3) Mede duração do pulso ECHO
    int64_t echo_start = esp_timer_get_time();
    while (gpio_get_level(SONAR_ECHO_GPIO) == 1) {
        if (esp_timer_get_time() - echo_start > SONAR_MAX_US) {
            ESP_LOGW(TAG, "Echo end timeout");
            return -1.0f;
        }
    }
    int64_t echo_end = esp_timer_get_time();

    // 4) Calcula distância
    float duration_us = (float)(echo_end - echo_start);
    // velocidade do som: ~343 m/s => 0.0343 cm/us; ida+volta dividem por 2
    float distance_cm = (duration_us * 0.0343f) / 2.0f;

    return distance_cm;
}
