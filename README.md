# Taller_Yocto
This repository includes the fifth workshop of Embedded Systems course. 

## Research
1. ¿Qué pasos debe seguir antes de escribir o leer de un puerto de entrada/salida general (GPIO)?
2. ¿Qué comando podría utilizar, bajo Linux, para escribir a un GPIO específico?

## Answers
1. Se debe de importar lal librería GPIO y time. Posterioremente se deberá inicializar el pin que se quiera utilizar(definición de uso i/o).
2. Primero se debe de pasar  amodo superusuario:
```bash
sudo su
```
Luego se debe exportar al espacio de usuario el nro. del GPIO que se quiera utilizar, por ejemplo:
```bash
echo 25 > /sys/class/gpio/export
```
Segguidamente se cambia de directorio al GPIO que se acaba de exportar:
```bash
cd /sys/class/gpio/gpio25
```
Se puede observar que dentro de este directorio hay un archivo llamado "direction" que permite definir si el GPIO será de entrada o salida. Para definirlo como salida se puede utilizar el siguiente comando:
```bash
echo out > direction
```
Finalmente, para escribir un valor al GPIO se puede utilizar el siguiente comando:
```bash
echo 1 > value
```
Para liberar el pin se puede utilizar:
```bash
echo 25 > /sys/class/gpio/unexport
```
