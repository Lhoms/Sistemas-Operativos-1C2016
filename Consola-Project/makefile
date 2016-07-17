#makefile para Consola
#Este archivo debe estar en la carpeta Consola-Project

#Instrucciones:
#Hacer cd a la carpeta Consola-Project
#Para compilar escribir make
#Despues de compilar para ejecutar escribir make run
#Despues de ejecutar eliminar los archivos generados con make clean

default: Consola

Consola: Consola.o Functions.o serialize.o Socket.o
	gcc -o Consola Consola.o Functions.o serialize.o Socket.o -lcommons

Consola.o: ./src/Consola.c ./src/Consola.h
	gcc -c ./src/Consola.c -lcommons

Functions.o: ./src/Functions.c ./src/Consola.h
	gcc -c ./src/Functions.c -lcommons

serialize.o: ./src/serialize.c ./src/serialize.h
	gcc -c ./src/serialize.c -lcommons

Socket.o: ./src/Socket.c ./src/Socket.h
	gcc -c ./src/Socket.c -lcommons


clean:
	-rm -f Consola.o
	-rm -f Functions.o
	-rm -f serialize.o
	-rm -f Socket.o
	-rm -f Consola
	-rm -f Consola.log

run:
	./Consola
