#makefile para Swap
#Este archivo debe estar en la carpeta Swap-Project

#Instrucciones:
#Hacer cd a la carpeta Swap-Project
#Para compilar escribir make
#Despues de compilar para ejecutar escribir make run
#Despues de ejecutar eliminar los archivos generados con make clean

default: swap

swap: Swap.o Functions.o serialize.o Socket.o
	gcc -o swap Swap.o Functions.o serialize.o Socket.o -lcommons

Swap.o: ./src/Swap.c ./src/Swap.h
	gcc -c ./src/Swap.c -lcommons

Functions.o: ./src/Functions.c ./src/Swap.h
	gcc -c ./src/Functions.c -lcommons

serialize.o: ./src/serialize.c ./src/serialize.h
	gcc -c ./src/serialize.c -lcommons

Socket.o: ./src/Socket.c ./src/Socket.h
	gcc -c ./src/Socket.c -lcommons


clean:
	-rm -f Swap.o
	-rm -f Functions.o
	-rm -f serialize.o
	-rm -f Socket.o
	-rm -f swap
	-rm -f Swap.log
	-rm -f swap.data

run:
	./swap "config.properties"
