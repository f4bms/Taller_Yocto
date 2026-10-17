// Para probar la biblioteca gpio_rpi para el Taller 5

/* Configura 2 GPIO de salida y 1 de entrada:
* - salida 1: escribe 1, 0, 1, 0... cada 0.25 s
* - salida 2: blink a la frec indicada durante 5s 
* - entrada: lee cada 0.25s y muestra en pantalla
*
* Uso: prueba_gpio [salida1 salida2 entrada frecuencia_hz]
* Default: GPIO20 (pin 38), GPIO19 (pin 35), GPIO4 (pin 7), 1 Hz.
* Como lo estamos terminando despuecito del proyecto, usamos pines que no choquen con la lógica del robot */

#define _POSIX_C_SOURCE 200809L

#include "gpio_rpi.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DURACION_S     5.0
#define PASO_MS        250

typedef struct {
    int pin;
    double freq;
    int resultado;
} blink_args_t;

//creamos el blink en un hilo para aislarlo y poder escribir y leer en los otros pines
static void *hilo_blink(void *arg) {
    blink_args_t *b = arg;
    b->resultado = blink(b->pin, b->freq, DURACION_S);
    return NULL;
}
//entre 0 y 27 porque esos son los GPIO de la rasp que salen al conector de 40 pines
//Del 28 al 57 son internos de la placa
static int leer_entero(const char *texto, int *valor) {
    char *fin;
    long v = strtol(texto, &fin, 10);
    if (*texto == '\0' || *fin != '\0' || v < 0 || v > 27) return -1;
    *valor = (int)v;
    return 0;
}

int main(int argc, char **argv) {
    int salida1 = 20, salida2 = 19, entrada = 4;
    double freq = 1.0;

    if (argc != 1 && argc != 5) {
        fprintf(stderr, "Uso: %s [salida1 salida2 entrada frecuencia_hz]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 5) {
        if (leer_entero(argv[1], &salida1) || leer_entero(argv[2], &salida2)
                || leer_entero(argv[3], &entrada)) {
            fprintf(stderr, "Los pines deben ser números BCM entre 0 y 27\n");
            return EXIT_FAILURE;
        }
        freq = strtod(argv[4], NULL);
        if (freq <= 0.0) {
            fprintf(stderr, "La frecuencia debe ser mayor que 0\n");
            return EXIT_FAILURE;
        }
        if (salida1 == salida2 || salida1 == entrada || salida2 == entrada) {
            fprintf(stderr, "Los tres pines deben ser distintos\n");
            return EXIT_FAILURE;
        }
    }

    printf("Salida fija: GPIO%d | Blink: GPIO%d a %.2f Hz por %.0f s | Entrada: GPIO%d\n",
           salida1, salida2, freq, DURACION_S, entrada);
    fflush(stdout);

    if (pinMode(salida1, GPIO_OUTPUT) != 0 || pinMode(salida2, GPIO_OUTPUT) != 0
            || pinMode(entrada, GPIO_INPUT) != 0) {
        perror("pinMode (¿pin ocupado por otro proceso o sin permiso en /dev/gpiochip0?)");
        gpio_rpi_cleanup();
        return EXIT_FAILURE;
    }

    blink_args_t args = {salida2, freq, 0};
    pthread_t hilo;
    if (pthread_create(&hilo, NULL, hilo_blink, &args) != 0) {
        fprintf(stderr, "No se pudo crear el hilo del blink\n");
        gpio_rpi_cleanup();
        return EXIT_FAILURE;
    }

    const struct timespec paso = {0, PASO_MS * 1000000L};
    int pasos = (int)(DURACION_S * 1000 / PASO_MS);
    for (int i = 1; i <= pasos; ++i) {
        int valor = i % 2;                       /* 1, 0, 1, 0, ... */
        digitalWrite(salida1, valor);
        int leido = digitalRead(entrada);
        printf("t=%5.2f s | GPIO%d (salida) = %d | GPIO%d (blink) = %d | GPIO%d (entrada) = %d\n",
               i * PASO_MS / 1000.0, salida1, digitalRead(salida1),
               salida2, digitalRead(salida2), entrada, leido);
        fflush(stdout);
        nanosleep(&paso, NULL);
    }

    pthread_join(hilo, NULL);
    digitalWrite(salida1, 0);
    printf("Blink terminado (%s). Salidas en 0.\n", args.resultado == 0 ? "OK" : "con error");
    gpio_rpi_cleanup();
    return args.resultado == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
