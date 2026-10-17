#!/bin/sh
# Compila la biblioteca, la aplicación y el helloworld con el SDK de Yocto.
#
# Uso (desde la raíz de esta carpeta):
#   ./scripts/compilar.sh [ruta/al/environment-setup-...]
# Por defecto usa el SDK de 64 bits del proyecto Roomba-Disco

set -e

ENTORNO=${1:-/opt/poky/5.0.20/environment-setup-cortexa72-poky-linux}

if [ ! -f "$ENTORNO" ]; then
    echo "No existe $ENTORNO"
    echo "Hay que instalar el SDK (ver PASOS_COMPANERO.md) o pasar la ruta del environment-setup como argumento."
    exit 1
fi

# Siempre desde la raíz del taller, aunque se llame desde otra carpeta
cd "$(dirname "$0")/.."

# shellcheck disable=SC1090
. "$ENTORNO"

echo "Compilador: $CC"
case "$CC" in
    aarch64-poky-linux-gcc*) ;;
    *) echo "AVISO: el CC no es aarch64-poky-linux-gcc; ¿es el SDK de 64 bits?" ;;
esac

make clean
make all sayhello

echo
file lib/libgpio_rpi.so bin/prueba_gpio bin/sayhello
