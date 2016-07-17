/*
 * Socket.c
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#include "Socket.h"

int create_socket_cliente(char*ip, char* puerto, t_log* LOG)
{
	//CREACION DE ESTRUCTURAS Y VARIABLES
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int callSock;

	log_info(LOG,"Creando socket");
	
	//CONFIGURANDO TIPO DE SOCKET CLIENTE
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, puerto, &hints, &serverInfo);
	
	//CREACION SOCKET CLIENTE
	callSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	log_info(LOG, "Socket creado");

	//CONEXION A SERVIDOR 
	log_info(LOG, "Conectando a server");
	connect(callSock, serverInfo->ai_addr, serverInfo->ai_addrlen);

	return callSock;
}
int create_socket_servidor(char* puerto, t_log* LOG)
{
	//CREACION DE ESTRUCTURAS Y VARIABLES
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int listenSock;

	log_info(LOG,"Creando socket");

	//CONFIGURANDO TIPO DE SOCKET SERVIDOR
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(NULL, puerto, &hints, &serverInfo);

	//CREACION SOCKET SERVIDOR
	listenSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	//ASOCIA LA DIRECCION LOCAL (IP+nroPUERTO) AL SOCKET
	bind(listenSock, serverInfo->ai_addr, serverInfo->ai_addrlen);
	log_info(LOG, "Socket creado");

	return listenSock;
}
