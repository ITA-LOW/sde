#include "servo.h"                 // Prototipos e inicialização do servo SG90
#include "sonar.h"                 // Prototipos e inicialização do sensor HC-SR04
#include "esp_log.h"               // API de logging (ESP_LOGx)
#include "freertos/FreeRTOS.h"     // Definições básicas do FreeRTOS
#include "freertos/task.h"         // API de tasks e delays do FreeRTOS

static const char *TAG = "MAIN";   // Tag usada nas mensagens de log

void app_main(void) {
    ESP_LOGI(TAG, "Inicializando...");      // Log de informação: início da aplicação
    servo_init();                            // Configura PWM e pino para controle de servo
    sonar_init();                            // Configura GPIOs para sensor ultrassônico

    int angle = 0, delta = 1;                // 'angle': posição atual do servo; 'delta': passo de variação
    const TickType_t delay = pdMS_TO_TICKS(20); // Converte 20 ms em ticks do RTOS para vTaskDelay

    while (1) {                              // Loop infinito: varre o servo e lê distância continuamente
        servo_set_angle(angle);              // Move servo para o ângulo atual
        vTaskDelay(delay);                   // Aguarda para servo se posicionar (20 ms)

        float d = sonar_get_distance_cm();   // Lê distância em cm do sensor ultrassônico
        ESP_LOGI(TAG, "Angle: %d°, Dist: %.2f cm", angle, d); // Exibe ângulo e distância no log

        angle += delta;                      // Atualiza o ângulo: soma o passo (delta)
        if (angle <= 0 || angle >= 180)      // Se atingir ou ultrapassar limites (0° ou 180°)
            delta = -delta;                 // Inverte direção do passo para efeito de vai-e-vem
    }
}
