#!/bin/sh

# Tomo archivo con variables de configuración.
. ./Vars.sh

# Me paro sobre UMC, borro archivo de configuración y lo armo de nuevo.
cd tp-2016-1c-Lajew
cd UMC-Project
rm config.properties

echo PUERTO=5002 >> config.properties
echo IP_SWAP=$IP_SWAP >> config.properties
echo PUERTO_SWAP=6000 >> config.properties
echo MARCOS=5 >> config.properties
echo MARCO_SIZE=$TAM_PAG >> config.properties
echo MARCO_X_PROC=2 >> config.properties
echo ALGORITMO=CLOCK >> config.properties
echo ENTRADAS_TLB=3 >> config.properties
echo RETARDO=2500 >> config.properties
echo "RUTA_DUMP=/home/utnso/tp-2016-1c-Lajew/UMC-Project" >> config.properties

# Ejecuto (Limpio y compilo)
#make clean
#make


