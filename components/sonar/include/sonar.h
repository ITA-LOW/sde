#ifndef SONAR_H
#define SONAR_H

#include <stdint.h>

/**
 * Inicializa os pinos TRIG e ECHO.
 * TRIG: GPIO de saída (pulso de trigger)
 * ECHO: GPIO de entrada (pulso de retorno)
 */
void sonar_init(void);

/**
 * Dispara o pulso de trigger e mede o tempo de retorno.
 * @return distância em cm (float)
 */
float sonar_get_distance_cm(void);

#endif // SONAR_H
