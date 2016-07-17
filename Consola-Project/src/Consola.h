/*
 * Consola.h
 *
 *  Created on: 1/5/2016
 *      Author: utnso
 */

#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

	#include <commons/txt.h>
	#include <commons/log.h>
	#include <commons/config.h>
	#include <sys/stat.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include "Socket.h"
	#include "serialize.h"
	#include "Protocolo.h"

	#define BACKLOG  4
	#define BUFFSIZE 1024
	#define	ERROR	 -1

	typedef struct{
		char* ip_nucleo;
		char* puerto_nucleo;}s_conf;

	s_conf  	get_config(t_log* LOG);
	struct s_protocol  get_sender_from_file(char* path);
	int 		realize_handshake(int socket, t_log* LOG);
	void		init_struct_protocol(struct s_protocol* sender);
	void		serialize_from_protocol(struct s_protocol protocol, void* buffer);
	void 		serialize_from_header(struct s_header header, void* buffer);
	void		deserialize_to_protocol(struct s_protocol* protocol, void* buffer);

#endif /* SRC_CONSOLA_H_ */
