#makefile para Nucleo
#Este archivo debe estar en la carpeta Nucleo-Project

#Instrucciones:
#Hacer cd a la carpeta Nucleo-Project
#Para compilar escribir make
#Despues de compilar para ejecutar escribir make run
#Despues de ejecutar eliminar los archivos generados con make clean

default: Nucleo

Nucleo: Nucleo.o Functions.o serialize.o Socket.o PCB.o
	gcc -o Nucleo Nucleo.o Functions.o serialize.o Socket.o PCB.o -lcommons -pthread -lparser-ansisop

Nucleo.o: ./src/Nucleo.c ./src/Socket.h ./src/serialize.h ./src/PCB.h
	gcc -c ./src/Nucleo.c -lcommons

Functions.o: ./src/Functions.c ./src/Nucleo.h ./src/serialize.h ./src/PCB.h
	gcc -c ./src/Functions.c -lcommons

serialize.o: ./src/serialize.c ./src/serialize.h
	gcc -c ./src/serialize.c -lcommons

Socket.o: ./src/Socket.c ./src/Socket.h
	gcc -c ./src/Socket.c -lcommons

PCB.o: ./src/PCB.c ./src/PCB.h ./src/Protocolo.h ./src/serialize.h ./src/Socket.h ./src/Nucleo.h
	gcc -c ./src/PCB.c -lcommons


clean:
	-rm -f Nucleo.o
	-rm -f Functions.o
	-rm -f serialize.o
	-rm -f Socket.o
	-rm -f PCB.o
	-rm -f Nucleo
	-rm -f Nucleo.log

run:
	./Nucleo "config.properties"
