#include "sonar.h"                // Prototipos do sonar
#include "driver/gpio.h"         // API de GPIO do ESP-IDF
#include "esp_timer.h"           // Funções de temporização de alta resolução
#include "esp_err.h"             // Códigos de erro ESP_OK, etc.
#include "esp_log.h"             // Logging (ESP_LOGx)

static const char *TAG = "SONAR";  // Tag usada nas mensagens de log

// Definições dos pinos conectados ao sensor HC-SR04
#define SONAR_TRIG_GPIO  GPIO_NUM_27  // Pino TRIG envia pulso para iniciar medição
#define SONAR_ECHO_GPIO  GPIO_NUM_34  // Pino ECHO recebe pulso de retorno

// Tempo máximo de espera pelo pulso de eco (em microssegundos)
#define SONAR_MAX_US     30000       // 30 ms (aprox. detecção de até ~5 m)

// Inicializa os pinos do sensor ultrassônico
void sonar_init(void) {
    // Configura TRIG como saída
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << SONAR_TRIG_GPIO,  // Máscara para o pino TRIG
        .mode = GPIO_MODE_OUTPUT,                  // Modo saída digital
    };
    gpio_config(&io_conf);                        // Aplica configuração TRIG

    // Configura ECHO como entrada
    io_conf.pin_bit_mask = 1ULL << SONAR_ECHO_GPIO; // Máscara para o pino ECHO
    io_conf.mode = GPIO_MODE_INPUT;                // Modo entrada digital
    gpio_config(&io_conf);                         // Aplica configuração ECHO

    // Garante que TRIG comece em nível baixo
    gpio_set_level(SONAR_TRIG_GPIO, 0);            // Sinal TRIG = 0 (sem pulso)
}

// Realiza medição de distância em centímetros
float sonar_get_distance_cm(void) {
    // 1) Gera pulso de trigger de 10 microssegundos
    gpio_set_level(SONAR_TRIG_GPIO, 1);            // Seta TRIG = 1 (pulso)
    esp_rom_delay_us(10);                          // Aguarda 10 µs
    gpio_set_level(SONAR_TRIG_GPIO, 0);            // Volta TRIG a 0

    // 2) Aguarda inicio do pulso ECHO (sinal alto)
    int64_t start = esp_timer_get_time();         // Marca o tempo atual (µs)
    while (gpio_get_level(SONAR_ECHO_GPIO) == 0) {  // Enquanto ECHO == 0
        // Se demora muito, aborta para evitar bloqueio
        if (esp_timer_get_time() - start > SONAR_MAX_US) {
            ESP_LOGW(TAG, "Echo start timeout");   // Aviso de timeout
            return -1.0f;                          // Código de erro
        }
    }

    // 3) Mede duração do pulso ECHO
    int64_t echo_start = esp_timer_get_time();    // Marca início do pulso ECHO
    while (gpio_get_level(SONAR_ECHO_GPIO) == 1) { // Enquanto ECHO == 1
        // Timeout se o pulso durar tempo demais
        if (esp_timer_get_time() - echo_start > SONAR_MAX_US) {
            ESP_LOGW(TAG, "Echo end timeout");     // Aviso de timeout
            return -1.0f;                          // Código de erro
        }
    }
    int64_t echo_end = esp_timer_get_time();      // Marca fim do pulso ECHO

    // 4) Calcula distancia usando tempo de ida e volta do som
    float duration_us = (float)(echo_end - echo_start); // Duração em µs
    // Velocidade do som ≈ 343 m/s => 0.0343 cm/µs; divide por 2 (ida+volta)
    float distance_cm = (duration_us * 0.0343f) / 2.0f;

    return distance_cm;                           // Retorna distância em cm
}
