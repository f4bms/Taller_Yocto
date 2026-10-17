#!/bin/sh
# Copia bin/ y lib/ a la Raspberry Pi por SSH, en /home/root/taller5/
#
# Uso (desde la raíz de esta carpeta, después de compilar):
#   ./scripts/desplegar.sh <ip-de-la-pi>
#
# Se usa tar por SSH en vez de scp: la imagen del proyecto tiene dropbear

set -e

if [ -z "$1" ]; then
    echo "Uso: $0 <ip-de-la-pi>"
    exit 1
fi
IP=$1
DESTINO=/home/root/taller5

cd "$(dirname "$0")/.."

for f in lib/libgpio_rpi.so bin/prueba_gpio bin/sayhello; do
    [ -f "$f" ] || { echo "Falta $f: correr primero ./scripts/compilar.sh"; exit 1; }
done

tar cf - bin lib | ssh "root@$IP" "mkdir -p $DESTINO && tar xf - -C $DESTINO"

echo "Copiado a root@$IP:$DESTINO"
echo "En la Pi:  cd $DESTINO && ./bin/sayhello && ./bin/prueba_gpio"
