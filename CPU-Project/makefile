#makefile para CPU
#Este archivo debe estar en la carpeta CPU-Project

#Instrucciones:
#Hacer cd a la carpeta CPU-Project
#Para compilar escribir make
#Despues de compilar para ejecutar escribir make run
#Despues de ejecutar eliminar los archivos generados con make clean

default: CPU

CPU: CPU.o Functions.o serialize.o Socket.o PCB.o
	gcc -o CPU CPU.o Functions.o serialize.o Socket.o PCB.o -lcommons -lparser-ansisop

CPU.o: ./src/CPU.c ./src/Protocolo.h ./src/CPU.h ./src/PCB.h
	gcc -c ./src/CPU.c -lcommons

Functions.o: ./src/Functions.c ./src/CPU.h
	gcc -c ./src/Functions.c -lcommons

serialize.o: ./src/serialize.c ./src/serialize.h
	gcc -c ./src/serialize.c -lcommons

Socket.o: ./src/Socket.c ./src/Socket.h
	gcc -c ./src/Socket.c -lcommons

PCB.o: ./src/PCB.c ./src/PCB.h ./src/Protocolo.h ./src/serialize.h
	gcc -c ./src/PCB.c -lcommons


clean:
	-rm -f CPU.o
	-rm -f Functions.o
	-rm -f serialize.o
	-rm -f Socket.o
	-rm -f PCB.o
	-rm -f CPU
	-rm -f Cpu.log

run:
	./CPU "config.properties"
