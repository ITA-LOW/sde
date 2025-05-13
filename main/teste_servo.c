#include "servo.h"
#include "sonar.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "Inicializando...");
    servo_init();
    sonar_init();

    int angle = 0, delta = 1;
    const TickType_t delay = pdMS_TO_TICKS(20);

    while (1) {
        servo_set_angle(angle);
        vTaskDelay(delay);
        float d = sonar_get_distance_cm();
        ESP_LOGI(TAG, "Angle: %d°, Dist: %.2f cm", angle, d);
        angle += delta;
        if (angle <= 0 || angle >= 180) delta = -delta;
    }
}
