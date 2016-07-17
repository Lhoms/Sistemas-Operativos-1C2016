/*
 * Socket.h
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#ifndef SRC_SOCKET_H_
#define SRC_SOCKET_H_
	#include <commons/config.h>
	#include <commons/log.h>
	#include <commons/string.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>

int 	create_socket_cliente(char*ip, char* puerto);
int 	create_socket_servidor(char* puerto);

t_log*  LOG;

#endif /* SRC_SOCKET_H_ */
