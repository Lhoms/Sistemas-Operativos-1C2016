#makefile para UMC
#Este archivo debe estar en la carpeta UMC-Project

#Instrucciones:
#Hacer cd a la carpeta UMC-Project
#Para compilar escribir make
#Despues de compilar para ejecutar escribir make run
#Despues de ejecutar eliminar los archivos generados con make clean

default: UMC

UMC: UMC.o Functions.o serialize.o Socket.o Memory.o TimeFunctions.o UMCServer.o
	gcc -o UMC UMC.o Functions.o serialize.o Socket.o Memory.o TimeFunctions.o UMCServer.o -lcommons -pthread

UMC.o: ./src/UMC.c ./src/UMC.h
	gcc -c ./src/UMC.c -lcommons

Functions.o: ./src/Functions.c ./src/UMC.h
	gcc -c ./src/Functions.c -lcommons

serialize.o: ./src/serialize.c ./src/serialize.h
	gcc -c ./src/serialize.c -lcommons

Socket.o: ./src/Socket.c ./src/Socket.h
	gcc -c ./src/Socket.c -lcommons

Memory.o: ./src/Memory.c ./src/Memory.h
	gcc -c ./src/Memory.c -lcommons

TimeFunctions.o: ./src/TimeFunctions.c ./src/TimeFunctions.h
	gcc -c ./src/TimeFunctions.c -lcommons

UMCServer.o: ./src/UMCServer.c ./src/UMC.h
	gcc -c ./src/UMCServer.c -lcommons


clean:
	-rm -f UMC.o
	-rm -f Functions.o
	-rm -f serialize.o
	-rm -f Socket.o
	-rm -f Memory.o
	-rm -f TimeFunctions.o
	-rm -f UMCServer.o
	-rm -f UMC
	-rm -f UMC.log

run:
	./UMC "config.properties"
