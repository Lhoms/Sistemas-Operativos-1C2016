/*
 * Socket.h
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#ifndef SRC_SOCKET_H_
#define SRC_SOCKET_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>


int 	 create_socket_cliente(char*ip, char* puerto, t_log* LOG);
int 	create_socket_servidor(char* puerto, t_log* LOG);
#endif /* SRC_SOCKET_H_ */
