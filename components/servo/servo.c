// servo.c  — Driver de controle de servo SG90 usando PWM no ESP32

#include "servo.h"                 // Protótipo do servo
#include "driver/ledc.h"          // API de PWM do ESP-IDF
#include "esp_err.h"              // Definições de códigos de erro ESP_OK, etc.
#include "driver/gpio.h"          // Definições de GPIO do ESP32

// --- CONFIGURAÇÃO DO PWM ---
#define SERVO_LEDC_TIMER       LEDC_TIMER_0        // Seleciona o timer 0 para o PWM
#define SERVO_LEDC_MODE        LEDC_LOW_SPEED_MODE // Modo de baixa velocidade para o PWM
#define SERVO_LEDC_CHANNEL     LEDC_CHANNEL_0      // Canal 0 para saída PWM

// --- PINO E FREQUÊNCIA DO SERVO SG90 ---
#define SERVO_PWM_GPIO         GPIO_NUM_25         // Pino GPIO 25 como saída de PWM
#define SERVO_LEDC_RESOLUTION  LEDC_TIMER_16_BIT   // Resolução de 16 bits para duty cycle
#define SERVO_LEDC_FREQ_HZ     50                  // Frequência de 50 Hz (período de 20 ms)

// Pulsos em microssegundos para SG90
#define SERVO_PULSE_MIN_US     500   // Pulso mínimo (~0° de rotação)
#define SERVO_PULSE_MAX_US     2500  // Pulso máximo (~180° de rotação)
#define SERVO_ANGLE_MIN        0     // Ângulo mínimo permitido em graus
#define SERVO_ANGLE_MAX        180   // Ângulo máximo permitido em graus

// Inicializa o PWM para controle do servo
void servo_init(void) {
    // Configura parâmetros do timer LEDC
    ledc_timer_config_t timer_conf = {
        .speed_mode       = SERVO_LEDC_MODE,       // Define modo de velocidade
        .timer_num        = SERVO_LEDC_TIMER,      // Define número do timer
        .duty_resolution  = SERVO_LEDC_RESOLUTION, // Define resolução de duty
        .freq_hz          = SERVO_LEDC_FREQ_HZ,    // Define frequência do PWM
        .clk_cfg          = LEDC_AUTO_CLK          // Seleciona relógio automático interno
    };
    esp_err_t err = ledc_timer_config(&timer_conf);  // Aplica configuração do timer
    assert(err == ESP_OK);                           // Verifica sucesso da configuração

    // Configura parâmetros do canal LEDC para saída PWM
    ledc_channel_config_t ch_conf = {
        .gpio_num       = SERVO_PWM_GPIO,       // Pino GPIO para saída PWM
        .speed_mode     = SERVO_LEDC_MODE,      // Mesmo modo de velocidade do timer
        .channel        = SERVO_LEDC_CHANNEL,   // Canal de PWM selecionado
        .intr_type      = LEDC_INTR_DISABLE,    // Desabilita interrupções do LEDC
        .timer_sel      = SERVO_LEDC_TIMER,     // Timer vinculado a esse canal
        .duty           = 0,                    // Duty inicial (0 = pulso desligado)
        .hpoint         = 0                     // Ponto de início do pulso no ciclo
    };
    err = ledc_channel_config(&ch_conf);        // Aplica configuração do canal
    assert(err == ESP_OK);                      // Verifica sucesso da configuração
}

// Define o ângulo do servo em graus (0 a 180)
void servo_set_angle(int angle) {
    // Limita o valor do ângulo ao intervalo suportado
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    // Converte o ângulo em tempo de pulso (microssegundos)
    uint32_t pulse_us = SERVO_PULSE_MIN_US +
        ((uint32_t)(SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US) *
         (angle - SERVO_ANGLE_MIN)) / (SERVO_ANGLE_MAX - SERVO_ANGLE_MIN);

    // Converte o tempo de pulso em ticks de duty cycle
    uint32_t max_tick = (1 << SERVO_LEDC_RESOLUTION) - 1;    // Ticks máximos em 16 bits
    uint32_t period_us = 1000000 / SERVO_LEDC_FREQ_HZ;      // Período total em microssegundos
    uint32_t duty = (pulse_us * max_tick) / period_us;      // Cálculo do duty correspondente

    // Ajusta o duty do canal e atualiza para aplicar o novo pulso
    ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, duty);
    ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);
}
