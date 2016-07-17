#!/bin/sh

# Tomo archivo con variables de configuración.
. ./Vars.sh

# Me paro sobre Nucleo, borro archivo de configuración y lo armo de nuevo.
cd tp-2016-1c-Lajew
cd Nucleo-Project
rm config.properties

echo "PUERTO_PROG=5000" >> config.properties
echo "PUERTO_CPU=5001" >> config.properties
echo "QUANTUM=3" >> config.properties
echo "QUANTUM_SLEEP=1000" >> config.properties
echo "IO_IDS=[HDD1, LPT1]" >> config.properties
echo "IO_SLEEP=[500, 200]" >> config.properties
echo "SEM_ID=[b, c]" >> config.properties
echo "SEM_INIT=[1, 0]" >> config.properties
echo "SHARED_VARS=[!colas, !compartida]" >> config.properties
echo IP_UMC=$IP_UMC >> config.properties
echo "PUERTO_UMC=5002" >> config.properties
echo "STACK_SIZE=8" >> config.properties

# Ejecuto (Limpio y compilo)
#make clean
#make

