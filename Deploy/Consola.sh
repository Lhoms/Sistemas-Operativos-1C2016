#!/bin/sh

# Tomo archivo con variables de configuración.
. ./Vars.sh

cd tp-2016-1c-Lajew
cd Consola-Project
rm config.properties

echo IP_NUCLEO=$IP_NUCLEO >> config.properties
echo PUERTO_NUCLEO=5000 >> config.properties

# Ejecuto (Limpio y compilo)
#make clean
#make

