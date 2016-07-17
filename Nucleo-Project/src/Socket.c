/*
 * Socket.c
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "Socket.h"
#include <errno.h>
#include <string.h>

int create_socket_cliente(char*ip, char* puerto){
	//CREACION DE VARIABLES Y ESTRUCTURAS
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int callSock;

	log_info(LOG,"Creando socket");
	
	//SETEO PROPERTIES DEL SOCKET CLIENTE
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, puerto, &hints, &serverInfo);
	
	//CREACION SOCKET
	callSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	log_info(LOG, "Socket creado (y)");
	//CONEXION SERVIDOR
	log_info(LOG, "Conectando a server");
	connect(callSock, serverInfo->ai_addr, serverInfo->ai_addrlen);

	return callSock;
}

int create_socket_servidor(char* puerto){
	//CREACION DE VARIABLES Y ESTRUCTURAS
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int listenSock;

	log_info(LOG,"Creando socket");
	
	//SETEO PROPERTIES DEL SOCKET SERVIDOR
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(NULL, puerto, &hints, &serverInfo);

	//CREACION SOCKET
	listenSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	
	//ASOCIA LA DIRECCION LOCAL (IP+nroPUERTO) AL SOCKET
	if(listenSock != -1)
	{
		log_info(LOG, "Socket creado");
		if(bind(listenSock, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
			log_info(LOG, "Error en el bind");
			printf("BIND ERROR%s \n",strerror(errno));
		}else
			log_info(LOG, "Socket conectado");
	}else
		log_info(LOG, "Error al crear el socket");

	return listenSock;
}
