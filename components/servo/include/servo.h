// servo.h
#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

/**
 * Inicializa o PWM para controle de servo.
 * Configura o timer e o canal na GPIO definida.
 */
void servo_init(void);

/**
 * Envia o ângulo (0–180°) para o servo na GPIO definida.
 */
void servo_set_angle(int angle);

#endif // SERVO_H