
// gpio_rpi.c - biblioteca de GPIO adaptada de la del proyecto para taller 5
 
/* cada pin configurado guarda su propia "petición de línea" de libgpiod:
 * mientras el proceso la tenga, ningún otro proceso puede usar ese pin.
 * Un mutex protege la tabla de pines para que blink() pueda correr en un hilo mientras el hilo principal R/W otros pines.*/
 
#define _POSIX_C_SOURCE 200809L

#include "gpio_rpi.h"

#include <errno.h>
#include <gpiod.h>
#include <pthread.h>
#include <time.h>

#define GPIO_CHIP_PATH "/dev/gpiochip0"
#define GPIO_MAX_PIN   64     // el gpiochip0 de la Pi 4 tiene 58 líneas, aunque solo vamos a dejar que el usuario use las primeras 27

typedef struct {
    struct gpiod_line_request *request;   // NULL = pin sin configurar
} pin_estado_t;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static struct gpiod_chip *chip = NULL;
static pin_estado_t pines[GPIO_MAX_PIN];

static int pin_valido(int pin) {
    return pin >= 0 && pin < GPIO_MAX_PIN;
}

// Abre el /dev/gpiochip0 la primera vez que se necesita. Con el mutex tomado
static int abrir_chip(void) {
    if (chip == NULL) {
        chip = gpiod_chip_open(GPIO_CHIP_PATH);
    }
    return chip ? 0 : -1;
}

// Suelta la línea de un pin. Igual con el mutex tomado
static void liberar_pin(int pin) {
    if (pines[pin].request) {
        gpiod_line_request_release(pines[pin].request);
        pines[pin].request = NULL;
    }
}

int pinMode(int pin, gpio_mode_t mode) {
    if (!pin_valido(pin) || (mode != GPIO_INPUT && mode != GPIO_OUTPUT)) {
        return -1;
    }

    pthread_mutex_lock(&mutex);
    if (abrir_chip() != 0) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    liberar_pin(pin);   // reconfigurar = soltar y volver a pedir la línea

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    struct gpiod_line_config *config = gpiod_line_config_new();
    struct gpiod_line_request *request = NULL;

    if (settings && config) {
        if (mode == GPIO_OUTPUT) {
            gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
            gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
        } else {
            gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
            // Pull-down: hace que una entrada al aire se lee 0 en vez de un valor al azar 
            gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_DOWN);
        }
        unsigned int offset = (unsigned int)pin;
        if (gpiod_line_config_add_line_settings(config, &offset, 1, settings) == 0) {
            request = gpiod_chip_request_lines(chip, NULL, config);
        }
    }
    gpiod_line_config_free(config);
    gpiod_line_settings_free(settings);

    pines[pin].request = request;
    pthread_mutex_unlock(&mutex);
    return request ? 0 : -1;
}

int digitalWrite(int pin, int value) {
    if (!pin_valido(pin)) {
        return -1;
    }
    pthread_mutex_lock(&mutex);
    int rv = -1;
    if (pines[pin].request) {
        rv = gpiod_line_request_set_value(pines[pin].request, (unsigned int)pin,
                 value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
    }
    pthread_mutex_unlock(&mutex);
    return rv == 0 ? 0 : -1;
}

int digitalRead(int pin) {
    if (!pin_valido(pin)) {
        return -1;
    }
    pthread_mutex_lock(&mutex);
    int rv = -1;
    if (pines[pin].request) {
        enum gpiod_line_value v =
            gpiod_line_request_get_value(pines[pin].request, (unsigned int)pin);
        if (v != GPIOD_LINE_VALUE_ERROR) {
            rv = (v == GPIOD_LINE_VALUE_ACTIVE) ? 1 : 0;
        }
    }
    pthread_mutex_unlock(&mutex);
    return rv;
}

// Suma ns a un instante, normalizando tv_nsec
static void sumar_ns(struct timespec *t, long long ns) {
    t->tv_sec += (time_t)(ns / 1000000000LL);
    t->tv_nsec += (long)(ns % 1000000000LL);
    if (t->tv_nsec >= 1000000000L) {
        t->tv_sec += 1;
        t->tv_nsec -= 1000000000L;
    }
}

int blink(int pin, double freq, double duration) {
    if (!pin_valido(pin) || freq <= 0.0 || duration <= 0.0) {
        return -1;
    }

    pthread_mutex_lock(&mutex);
    int configurado = pines[pin].request != NULL;
    pthread_mutex_unlock(&mutex);
    if (!configurado && pinMode(pin, GPIO_OUTPUT) != 0) {
        return -1;
    }

    //un ciclo = medio periodo en 1 + medio periodo en 0.  
	//Se cuentan los cambios y se espera hasta TIMER_ABSTIME, enonces el tiempo que tarda c/W no se va acumulando como error
    long long medio_periodo_ns = (long long)(1e9 / (2.0 * freq));
    long cambios = (long)(2.0 * freq * duration + 0.5);
    if (medio_periodo_ns <= 0 || cambios <= 0) {
        return -1;
    }

    struct timespec siguiente;
    clock_gettime(CLOCK_MONOTONIC, &siguiente);
    int valor = 0;
    for (long i = 0; i < cambios; ++i) {
        valor = !valor;
        if (digitalWrite(pin, valor) != 0) {
            return -1;
        }
        sumar_ns(&siguiente, medio_periodo_ns);
        while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &siguiente, NULL) == EINTR) {
            // una señal interrumpió la espera, seguir esperando
        }
    }
    return digitalWrite(pin, 0);
}

void gpio_rpi_cleanup(void) {
    pthread_mutex_lock(&mutex);
    for (int pin = 0; pin < GPIO_MAX_PIN; ++pin) {
        liberar_pin(pin);
    }
    if (chip) {
        gpiod_chip_close(chip);
        chip = NULL;
    }
    pthread_mutex_unlock(&mutex);
}
