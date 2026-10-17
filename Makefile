# Taller 5 - Biblioteca dinámica GPIO para la Raspberry Pi 4, montado sobre la imagen creada para el proyecto

# Se compila con el SDK de Yocto (compilación cruzada). Antes de "make":
#   source /opt/poky/5.0.20/environment-setup-cortexa72-poky-linux
# Ese script define CC (aarch64-poky-linux-gcc --sysroot=...), CFLAGS y LDFLAGS
# este Makefile los usa y solo les agrega lo propio o usar directamente: ./scripts/compilar.sh

LIB_NOMBRE := gpio_rpi
LIB        := lib/lib$(LIB_NOMBRE).so
APP        := bin/prueba_gpio
HELLO      := bin/sayhello

# Opciones propias en otra variable: así no se pierden si alguien pasa
# CFLAGS en la línea de comandos (eso reemplaza cualquier CFLAGS +=).
TALLER_CFLAGS := -Wall -Wextra -O2 -Iinclude
# La app busca la biblioteca en ../lib relativo a donde esté el ejecutable,
# así se puede copiar la carpeta a la Pi sin instalar nada en /usr/lib.
APP_RPATH := -Wl,-rpath,'$$ORIGIN/../lib'

# Solo para probar en la PC con una libgpiod propia (ver README): make GPIOD_PREFIX=/ruta
ifdef GPIOD_PREFIX
TALLER_CFLAGS += -I$(GPIOD_PREFIX)/include
LDFLAGS += -L$(GPIOD_PREFIX)/lib -Wl,-rpath,$(GPIOD_PREFIX)/lib
endif

# Nombre del archivo de entrega (lo que dice el enunciado)
NOMBRE ?= Fabiola_Irene_Jeremy_Marco_Eval_Tut_7

.PHONY: all sayhello info clean entrega

all: $(LIB) $(APP)

$(LIB): src/gpio_rpi.c include/gpio_rpi.h | lib
	$(CC) $(CFLAGS) $(TALLER_CFLAGS) -fPIC -shared -Wl,-soname,lib$(LIB_NOMBRE).so -o $@ $< $(LDFLAGS) -lgpiod -lpthread

$(APP): src/main.c include/gpio_rpi.h $(LIB) | bin
	$(CC) $(CFLAGS) $(TALLER_CFLAGS) -o $@ $< $(LDFLAGS) -Llib -l$(LIB_NOMBRE) -lpthread $(APP_RPATH)

# Prueba del toolchain de la sección 3 del enunciado
sayhello: $(HELLO)
$(HELLO): test_toolchain/helloworld.c | bin
	$(CC) $(CFLAGS) $(TALLER_CFLAGS) -o $@ $< $(LDFLAGS)

lib bin:
	mkdir -p $@

info:
	@echo "CC     = $(CC)"
	@echo "Destino: $$($(CC) -dumpmachine)"

clean:
	rm -f lib/*.so bin/prueba_gpio bin/sayhello

# Empaqueta la entrega con la estructura del enunciado (src, include, lib, bin...).
# No compila: exige que los binarios ya existan y sean ARM (aarch64), para no entregar por error binarios de la PC si no se cargó el SDK.
entrega:
	@for f in $(LIB) $(APP) $(HELLO); do \
	    [ -f $$f ] || { echo "Falta $$f: correr primero ./scripts/compilar.sh"; exit 1; }; \
	    file $$f | grep -q aarch64 || { echo "$$f no es aarch64: recompilar con el SDK (./scripts/compilar.sh)"; exit 1; }; \
	done
	tar czf ../$(NOMBRE).tar.gz --transform 's,^\.,$(NOMBRE),' \
	    --exclude=.git \
	    ./README.md ./Makefile ./src ./include ./lib ./bin ./scripts ./test_toolchain \
	    $(if $(wildcard capturas),./capturas)
	@echo "Creado ../$(NOMBRE).tar.gz"
