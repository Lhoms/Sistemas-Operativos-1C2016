#!/bin/sh

# Tomo archivo con variables de configuración.
. ./Vars.sh

# Me paro sobre CPU, borro archivo de configuración y lo armo de nuevo.
cd tp-2016-1c-Lajew
cd CPU-Project
rm config.properties

echo IP_UMC=$IP_UMC >> config.properties
echo PUERTO_UMC=5002 >> config.properties
echo IP_NUCLEO=$IP_NUCLEO >> config.properties
echo PUERTO_NUCLEO=5000 >> config.properties

# Ejecuto (Limpio y compilo)
#make clean
#make

