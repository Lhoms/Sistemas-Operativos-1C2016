/*
 * CPU.h
 *
 *  Created on: 1/5/2016
 *      Author: utnso
 */

#ifndef SRC_CPU_H_
#define SRC_CPU_H_

	#include <commons/txt.h>
	#include <commons/log.h>
	#include <commons/config.h>
	#include <sys/stat.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include "serialize.h"
	#include "Primitivas.h"
	#include "Socket.h"
	#include "Protocolo.h"
	#include <parser/metadata_program.h>
	#include <commons/collections/list.h>

	#define BACKLOG  4
	#define BUFFSIZE 1024

	typedef struct{
		char* ip_nucleo;
		char* puerto_nucleo;
		char* ip_umc;
		char* puerto_umc;
	}s_conf;


	s_conf  	get_config(t_log* LOG);
	struct s_protocol  get_sender_from_file(char* path);

	void		init_struct_protocol(struct s_protocol* sender);
	void 		serialize_from_header(struct s_header header, void* buffer);
	void		serialize_from_protocol(struct s_protocol protocol, void* buffer);
	void		deserialize_to_protocol(struct s_protocol* protocol, void* buffer);

	void 		hacerPedidoA_UMC(int pedido_id, char* buffer);

	int 		realize_handshake(int socket, t_log* LOG);
	int 		receive_tamanio_pagina(int socket, int* tamanio_pagina, t_log* LOG);
	int			receive_quantum (int socket, int* quantum, t_log* LOG);
	int 		receive_quantumSleep (int socket, int* quantumSleep, t_log* LOG);

	void 		notificar_cambio_proceso_activo(int socket,int pid, t_log* LOG);
	void 	    notificar_estado_pcb(int socket, int estado, t_log* LOG);
	void 		notificar_estado_error_pcb(int socket, int estado, char* error, t_log* LOG);

	int 		enviar_bytes_por_socket(int socket, int nroPag, int offset, int tamanioData, void* data, char* string_error, t_log* LOG);
	int 		pedir_bytes_por_socket(int socket, int nroPag, int offset, int tamanio, void* data, char* string_error, t_log* LOG);

	char*		pedir_bytes_a_umc(int pagina,int offset,int tamanio);
	void 		enviar_bytes_a_umc(int pagina, int offset, int tamanio, void* data);

	void 		socket_ok_response(int socket);
	void 		socket_fallo_response(int socket);
	char* 		receive_string_de_ids(int socket, int tipo_id, t_log* LOG);
	void        socket_send_end(int socket);
	void        send_wait(int socket, t_log* LOG);

	void		notificar_io_pcb(int NucleoSock,int estadoPcb, char* id, int tiempo, t_log* LOG);
	int         notificar_semaforo_pcb(int NucleoSock,int estadoPcb,char* id,t_log* LOG);
	void        imprimir_en_consola(int NucleoSock, char* string,t_log* LOG);

	int			get_value_sharvar(int socket, char* id, t_log* LOG);
	void 		set_value_sharvar(int socket, char* id, int valor, t_log* LOG);

	char* 		separarID(char* instruccion);




#endif /* SRC_CPU_H_ */
