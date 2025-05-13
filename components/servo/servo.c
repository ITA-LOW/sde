// servo.c
#include "servo.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "driver/gpio.h"

// --- CONFIGURAÇÃO DO PWM ---
#define SERVO_LEDC_TIMER       LEDC_TIMER_0
#define SERVO_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_CHANNEL     LEDC_CHANNEL_0
// Defina aqui a GPIO de saída PWM para o SG90
#define SERVO_PWM_GPIO         GPIO_NUM_25
#define SERVO_LEDC_RESOLUTION  LEDC_TIMER_16_BIT
#define SERVO_LEDC_FREQ_HZ     50               // 50 Hz => 20 ms de período

// Pulsos em microssegundos típicos para SG90
#define SERVO_PULSE_MIN_US     500   // ~0°
#define SERVO_PULSE_MAX_US     2500  // ~180°
#define SERVO_ANGLE_MIN        0     // batente mínimo em graus
#define SERVO_ANGLE_MAX        180   // batente máximo em graus

void servo_init(void) {
    // Configura timer PWM
    ledc_timer_config_t timer_conf = {
        .speed_mode       = SERVO_LEDC_MODE,
        .timer_num        = SERVO_LEDC_TIMER,
        .duty_resolution  = SERVO_LEDC_RESOLUTION,
        .freq_hz          = SERVO_LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    esp_err_t err = ledc_timer_config(&timer_conf);
    assert(err == ESP_OK);

    // Configura canal PWM
    ledc_channel_config_t ch_conf = {
        .gpio_num       = SERVO_PWM_GPIO,
        .speed_mode     = SERVO_LEDC_MODE,
        .channel        = SERVO_LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = SERVO_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };
    err = ledc_channel_config(&ch_conf);
    assert(err == ESP_OK);
}

void servo_set_angle(int angle) {
    // Limita ao intervalo suportado
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    // Converte grau em pulso em microssegundos
    uint32_t pulse_us = SERVO_PULSE_MIN_US +
        ((uint32_t)(SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US) *
         (angle - SERVO_ANGLE_MIN)) / (SERVO_ANGLE_MAX - SERVO_ANGLE_MIN);

    // Converte microssegundos em ticks (duty)
    uint32_t max_tick = (1 << SERVO_LEDC_RESOLUTION) - 1;
    uint32_t period_us = 1000000 / SERVO_LEDC_FREQ_HZ;
    uint32_t duty = (pulse_us * max_tick) / period_us;

    ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, duty);
    ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);
}