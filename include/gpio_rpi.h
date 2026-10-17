// gpio_rpi.h - Biblioteca dinámica para manejo de GPIO digitales en la Rasp para taller 5
 
/* Usa el dispositivo de caracteres /dev/gpiochip0 por medio de libgpiod v2
* Los números de pin son la numeración BCM (GPIO17 = 17), que en el
* gpiochip0 de la Pi 4 coinciden con el offset de la línea*/

#ifndef GPIO_RPI_H
#define GPIO_RPI_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GPIO_INPUT = 0,
    GPIO_OUTPUT = 1
} gpio_mode_t;

/* Configura un pin como entrada o salida. 
 *Debe llamarse antes de digitalWrite/digitalRead sobre ese pin. 
 *Las entradas quedan con resistencia pull-down interna (sin conectar se leen en 0) y las salidas arrancan en 0
 *Si el pin ya estaba configurado, se reconfigura.
 * Devuelve 0 en éxito, -1 en error. */
int pinMode(int pin, gpio_mode_t mode);

//Escribe 0/1 (todo valor dif de 0 es 1) en un pin conf como salida. Devuelve 0 en éxito, -1 en error
int digitalWrite(int pin, int value);

// Lee el estado de un pin configurado (I/O). Devuelve 0 o 1, o -1 en error
int digitalRead(int pin);

/* Alterna el pin entre 1 y 0 a la frecuencia freq (Hz) durante duration (seg) y lo deja en 0 al terminar
 * Si el pin no estaba configurado, lo configura como salida.
 *Bloquea. Devuelve 0 en éxito, -1 en error. */
int blink(int pin, double freq, double duration);

// Libera todos los pines y cierra el chip GPIO. Llamar al terminar
void gpio_rpi_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif 
