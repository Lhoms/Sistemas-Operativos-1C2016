# Tomo archivo con variables de configuración.
include Vars.sh

default: Distrib

consola:
	scp Consola.sh CPU.sh Vars.sh utnso@${IP_CONSOLA}:${REMOTE_PATH}
	ssh utnso@${IP_CONSOLA} "chmod 777 ${REMOTE_PATH}/*.sh && ${REMOTE_PATH}/Consola.sh && ${REMOTE_PATH}/CPU.sh"

nucleo:
	scp Consola.sh CPU.sh Nucleo.sh Vars.sh utnso@${IP_NUCLEO}:${REMOTE_PATH}
	ssh utnso@${IP_NUCLEO} "chmod 777 ${REMOTE_PATH}/*.sh && ${REMOTE_PATH}/Nucleo.sh && ${REMOTE_PATH}/Consola.sh && ${REMOTE_PATH}/CPU.sh"

umc:
	scp Consola.sh CPU.sh UMC.sh Vars.sh utnso@${IP_UMC}:${REMOTE_PATH}
	ssh utnso@${IP_UMC} "chmod 777 ${REMOTE_PATH}/*.sh && ${REMOTE_PATH}/UMC.sh && ${REMOTE_PATH}/Consola.sh && ${REMOTE_PATH}/CPU.sh"

cpu:
	scp Consola.sh CPU.sh Vars.sh utnso@${IP_CPU}:${REMOTE_PATH}
	ssh utnso@${IP_CPU} "chmod 777 ${REMOTE_PATH}/*.sh && ${REMOTE_PATH}/Consola.sh && ${REMOTE_PATH}/CPU.sh"

swap:
	scp Consola.sh CPU.sh SWAP.sh Vars.sh utnso@${IP_SWAP}:${REMOTE_PATH}
	ssh utnso@${IP_SWAP} "chmod 777 ${REMOTE_PATH}/*.sh && ${REMOTE_PATH}/SWAP.sh && ${REMOTE_PATH}/Consola.sh && ${REMOTE_PATH}/CPU.sh"

Distrib:
	./distrib.sh
