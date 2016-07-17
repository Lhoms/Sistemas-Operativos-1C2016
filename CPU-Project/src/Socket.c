/*
 * Socket.c
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#include "Socket.h"
#include <string.h>

#define EXIT_FAILURE -1

int create_socket_cliente(char*ip, char* puerto, t_log* LOG){
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int callSock;

	log_info(LOG,"Creando socket");

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	callSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	if (callSock <= 0)
	{	log_error(LOG, "Error al conectarse con UMC");
		return EXIT_FAILURE;}

	log_info(LOG, "Socket creado");

	log_info(LOG, "Conectando a server");
	connect(callSock, serverInfo->ai_addr, serverInfo->ai_addrlen);

	return callSock;
}
int create_socket_servidor(char* puerto, t_log* LOG){
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	int listenSock;

	log_info(LOG,"Creando socket");

	memset(&hints, 0, sizeof(hints));
	hints.ai_family =PF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &serverInfo);

	listenSock = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	if (listenSock <= 0)
	{	log_error(LOG, "Error al conectarse con UMC");
		return EXIT_FAILURE;}

	bind(listenSock, serverInfo->ai_addr, serverInfo->ai_addrlen);
	log_info(LOG, "Socket creado");

	return listenSock;
}

