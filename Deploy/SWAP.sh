#!/bin/sh

# Tomo archivo con variables de configuración.
. ./Vars.sh

# Me paro sobre SWAP, borro archivo de configuración y lo armo de nuevo.
cd tp-2016-1c-Lajew
cd Swap-Project
rm config.properties

echo PUERTO_ESCUCHA=6000 >> config.properties
echo NOMBRE_SWAP=swap.data >> config.properties
echo CANTIDAD_PAGINAS=256 >> config.properties
echo TAMANIO_PAGINA=$TAM_PAG >> config.properties
echo RETARDO_COMPACTACION=10000 >> config.properties
echo RETARDO_ACCESO=3000 >> config.properties

# Ejecuto (Limpio y compilo)
#make clean
#make

