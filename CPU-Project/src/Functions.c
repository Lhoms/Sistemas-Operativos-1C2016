/*
 * Functions.c
 *
 *  Created on: 1/5/2016
 *      Author: utnso
 */

#include "CPU.h"

s_conf get_config(t_log* LOG){
	//CREACION DE VARIABLES Y ESTRUCTURAS
	s_conf config;
	t_config* config_file;
	int cantConfig = 0;
	char* logMessage = string_new();

	//LEVANTO CONFIG
	config_file = config_create("config.properties");
	cantConfig = config_keys_amount(config_file);

	//VERIFICACION DE EXISTENCIA DE CONFIG
	if (cantConfig == 0)
	{
		log_error(LOG, "Archivo de configuración vacío");
	}

	//SETEO DE PROPIEDADES
	config.ip_nucleo = string_duplicate(config_get_string_value(config_file, "IP_NUCLEO"));
	if (!string_is_empty(config.ip_nucleo))
	{
		logMessage = string_from_format("IP_NUCLEO: %s", config.ip_nucleo);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "Error al obtener IP del núcleo");
	}

	config.puerto_nucleo = string_duplicate(config_get_string_value(config_file, "PUERTO_NUCLEO"));
	if (!string_is_empty(config.puerto_nucleo))
	{
		logMessage = string_from_format("PUERTO_NUCLEO: %s", config.puerto_nucleo);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "Error al obtener puerto del núcleo");
	}

	config.ip_umc = string_duplicate(config_get_string_value(config_file, "IP_UMC"));
	if (!string_is_empty(config.ip_nucleo))
	{
		logMessage = string_from_format("IP_NUCLEO: %s", config.ip_nucleo);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "Error al obtener IP del núcleo");
	}

	config.puerto_umc = string_duplicate(config_get_string_value(config_file, "PUERTO_UMC"));
	if (!string_is_empty(config.puerto_nucleo))
	{
		logMessage = string_from_format("PUERTO_NUCLEO: %s", config.puerto_nucleo);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "Error al obtener puerto del núcleo");
	}

	config_destroy(config_file);
	return config;
}

struct s_protocol get_sender_from_file(char* path){
	FILE* file;
	struct s_protocol sender;
	long size;

	//Inicializo struct de protocolo para el sender;
	init_struct_protocol(&sender);

	//Apertura de archivo en modo lectura
	file = fopen(path, "r");

	//Leo propiedades del archivo para saber la longitud
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	if (file == NULL || size == 0){
		return sender;
	}

	//Leo archivo hasta longitud sacada.
	char* file_string = malloc(sizeof(char) * size);
	fread(file_string, sizeof(char), size, file);
	sender.payload = string_duplicate(file_string);
	free(file_string);

	//Le agrego el \0 al final(no estoy tan seguro)
	//sender.payload[size] = '\0';
	//strcat(&(sender.payload), "\0");
	sender.payload[strlen(sender.payload)] = '\0';

	//Armo header de archivo.
	sender.id = 90;						//por ahora hasta definir bien los IDs
	sender.size = size + 1;

	//Cierro archivo
	fclose(file);

	return sender;
}

void init_struct_protocol(struct s_protocol* protocol){

	protocol->id = 0;
	protocol->size = 0;
	//memset(&protocol->payload, ' ', sizeof(protocol->payload));
	protocol->payload = "";

}

void serialize_from_header(struct s_header header, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, header.id, &offset);
	serialize_from_int(buffer, header.size, &offset);
}

void serialize_from_protocol(struct s_protocol protocol, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, protocol.id, &offset);
	serialize_from_int(buffer, protocol.size, &offset);
	serialize_from_string(buffer, protocol.payload, &offset);

}

int realize_handshake(int socket, t_log* LOG){
	int ret;

	struct s_header sendHandshake;
	struct s_header recvHandshake;

	sendHandshake.id = SALUDO_CPU;
	sendHandshake.size = 0;

	void* senderBufferHandshake = malloc(sizeof(struct s_header));

	serialize_from_header(sendHandshake, senderBufferHandshake);

	int status = send(socket, senderBufferHandshake, sizeof(struct s_header), 0);
	if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);free(senderBufferHandshake);return ERROR;}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(socket);free(senderBufferHandshake);return ERROR;}

	else{
		log_info(LOG, "Se recibe mensaje");
      	status = recv(socket, &recvHandshake, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);free(senderBufferHandshake);return ERROR;}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(socket);free(senderBufferHandshake);return ERROR;}

		else{
			if (recvHandshake.id == OK){
				log_info(LOG, "Handshake realizado correctamente");
				ret = OK;
			}else{
				log_error(LOG, "Handshake falló");
				ret = FALLO;
			}
		}
	}

	free(senderBufferHandshake);

	return ret;

}

int receive_tamanio_pagina(int socket, int* tamanio_pagina, t_log* LOG){
	int ret;

	struct s_header recvBuffer;

      	int status = recv(socket, &recvBuffer, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);return ERROR;}
		else if(status ==  0) {log_error(LOG, "Error: se cayo umc");	    close(socket);return ERROR;}

		else{
			if (recvBuffer.id == TAMANIO_PAGINA){
				status = recv(socket, tamanio_pagina, recvBuffer.size, 0);
				if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);return ERROR;}
				else if(status ==  0) {log_error(LOG, "Error: se cayo umc");	    close(socket);return ERROR;}

				log_info(LOG, "Se recibe tamaño de pagina correctamente");
				ret = OK;

			}else{
				log_error(LOG, "Falló recibir tamaño de pagina");
				ret = ERROR;
			}
		}

	return ret;

}

int receive_quantum (int socket, int* quantum, t_log* LOG){

	int ret;

	struct s_header recvBuffer;

      	int status = recv(socket, &recvBuffer, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);return ERROR;}
		else if(status ==  0) {log_error(LOG, "Error: se cayo nucleo");	    close(socket);return ERROR;}

		else{
			if (recvBuffer.id == TAMANIO_QUANTUM){
				status = recv(socket, quantum, recvBuffer.size, 0);
				if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);return ERROR;}
				else if(status ==  0) {log_error(LOG, "Error: se cayo nucleo");	    close(socket);return ERROR;}

				log_info(LOG, "Se recibe tamaño de quantum correctamente");
				socket_ok_response(socket);
				ret = OK;

			}else{
				log_error(LOG, "Falló recibir tamaño de quantum");
				ret = ERROR;
			}
		}

	return ret;


}

int receive_quantumSleep (int socket, int* quantumSleep, t_log* LOG){

	int ret;

	struct s_header recvBuffer;

      	int status = recv(socket, &recvBuffer, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);return ERROR;}
		else if(status ==  0) {log_error(LOG, "Error: se cayo nucleo");	    close(socket);return ERROR;}

		else{
			if (recvBuffer.id == TAMANIO_QUANTUMSLEEP){
				status = recv(socket, quantumSleep, recvBuffer.size, 0);
				if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);return ERROR;}
				else if(status ==  0) {log_error(LOG, "Error: se cayo nucleo");	    close(socket);return ERROR;}

				log_info(LOG, "Se recibe tamaño de quantum sleep correctamente");
				socket_ok_response(socket);
				ret = OK;

			}else{
				log_error(LOG, "Falló recibir tamaño de quantum sleep");
				ret = ERROR;
			}
		}

	return ret;


}

void notificar_estado_pcb(int socket, int estado, t_log* LOG){

	struct s_header receiver;
	struct s_header senderHeader = {
		  .id = estado,
		.size = 0,
	};

	void* senderBuffer = malloc(sizeof(struct s_header));

	serialize_from_header(senderHeader, senderBuffer);


	int status = send(socket, senderBuffer, sizeof(struct s_header), 0);

	if(status == -1) {log_error(LOG, "Error: no se envio estado");        close(socket);}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket"); close(socket);}
	else{
		log_info(LOG, "Estado enviado correctamente");
		log_info(LOG, "Se recibe mensaje");
	}

      	status = recv(socket, &receiver, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);free(senderBuffer);}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(socket);free(senderBuffer);}

		else{
			if (receiver.id == OK){
				log_info(LOG, string_from_format("Se envio estado pcb N: %d correctamente", estado));
				free(senderBuffer);

			}else{
				log_error(LOG, "Fallo notificando estado de pcb");
				free(senderBuffer);
			    }
			}


}

void notificar_estado_error_pcb(int socket, int estado, char* error, t_log* LOG){

	struct s_header receiver;
	struct s_header senderHeader = {
		  .id = estado,
		.size = strlen(error) + 1,
	};

	void* senderBuffer = malloc(sizeof(struct s_header) + senderHeader.size);

	serialize_from_header(senderHeader, senderBuffer);
	memcpy(senderBuffer + sizeof(struct s_header), error, senderHeader.size );


	int status = send(socket, senderBuffer, sizeof(struct s_header) + senderHeader.size, 0);

	if(status == -1) {log_error(LOG, "Error: no se envio estado error");     }
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket"); }
	else{
		log_info(LOG, "Estado error enviado correctamente");
		log_info(LOG, "Se recibe mensaje");
	}

      	status = recv(socket, &receiver, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");free(senderBuffer);}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	free(senderBuffer);}

		else{
			if (receiver.id == OK){
				log_info(LOG, string_from_format("Se envio error : %s correctamente", error));
				free(senderBuffer);

			}else{
				log_error(LOG, "Fallo notificando estado error de pcb");
				free(senderBuffer);
			    }
			}


}

int notificar_semaforo_pcb(int NucleoSock,int estadoPcb, char* id,t_log* LOG){

	int valor = 0;
	struct s_header receiver;
	struct s_header senderHeader = {
	     	  .id = estadoPcb,
			.size = strlen(id) + 1,
			};

	char* senderBuffer = malloc (sizeof(struct s_header) + senderHeader.size);

	serialize_from_header(senderHeader, senderBuffer);

	memcpy(senderBuffer+sizeof(struct s_header), id, senderHeader.size);

	int status = send(NucleoSock, senderBuffer, sizeof(struct s_header) + senderHeader.size, 0);

	if(status == -1) {log_error(LOG, "Error: no se el semaforo");      }
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket"); }
	else{
		log_info(LOG, "semaforo enviado correctamente");
		log_info(LOG, "Se recibe mensaje");
	}

      	status = recv(NucleoSock, &receiver, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");free(senderBuffer);}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	free(senderBuffer);}

		else{
			if (receiver.id == OK){
				log_info(LOG, "Se recibio Ok");

				if(estadoPcb == SEM_GET)
				recv(NucleoSock, &valor, receiver.size, 0);

				free(senderBuffer);

			}else{
				log_error(LOG, "Fallo notificando semaforo");
				free(senderBuffer);
			    }
			}

	return valor;
}

void notificar_io_pcb(int NucleoSock,int estadoPcb, char* id, int tiempo, t_log* LOG){

	struct s_header receiver;
	struct s_header senderHeader = {
	     	  .id = estadoPcb,
			.size = strlen(id) + 1 + sizeof(int),
			};

	char* senderBuffer = malloc (sizeof(struct s_header) + senderHeader.size);

	serialize_from_header(senderHeader, senderBuffer);

	int offset = sizeof(struct s_header);
	memcpy(senderBuffer + offset, id, strlen(id) + 1); offset += strlen(id) + 1;
	memcpy(senderBuffer + offset, &tiempo, sizeof(int));

	int status = send(NucleoSock, senderBuffer, sizeof(struct s_header) + senderHeader.size, 0);

	if(status == -1) {log_error(LOG, "Error: no se el semaforo");}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");}
	else{
		log_info(LOG, "io enviado correctamente");
		log_info(LOG, "Se recibe respuesta");
	}

      	status = recv(NucleoSock, &receiver, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");free(senderBuffer);}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	free(senderBuffer);}

		else{
			if (receiver.id == OK){
				log_info(LOG, "Se recibio Ok");
				free(senderBuffer);

			}else{
				log_error(LOG, "Fallo recibiendo OK");
				free(senderBuffer);
			    }
			}


}

void imprimir_en_consola(int NucleoSock, char* string,t_log* LOG){

	struct s_header receiver;
	struct s_header senderHeader = {
	     	  .id = IMPRIMIR,
			.size = strlen(string) + 1,
			};

	char* senderBuffer = malloc (sizeof(struct s_header) + senderHeader.size);

	serialize_from_header(senderHeader, senderBuffer);

	memcpy(senderBuffer+sizeof(struct s_header), string, senderHeader.size);

	int status = send(NucleoSock, senderBuffer, sizeof(struct s_header) + senderHeader.size, 0);

	if(status == -1) {log_error(LOG, "Error: no se envio imprimir");   close(NucleoSock);}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket"); close(NucleoSock);}
	else{
		log_info(LOG, string_from_format("se imprime %s en consola", string));
		log_info(LOG, "Se recibe mensaje");
	}

      	status = recv(NucleoSock, &receiver, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(NucleoSock);free(senderBuffer);}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(NucleoSock);free(senderBuffer);}

		else{
			if (receiver.id == OK){
				log_info(LOG, "Se recibio Ok");
				free(senderBuffer);

			}else{
				log_error(LOG, "Fallo imprimir");
				free(senderBuffer);
			    }
			}


}

void notificar_cambio_proceso_activo(int socket,int pid, t_log* LOG){

	struct s_header receiver;
	struct s_header senderHeader = {
		.id = CAMBIO_PROCESO_ACTIVO,
		.size = sizeof(int),
	};

	void* senderBuffer = malloc(sizeof(struct s_header) + sizeof(int));

	serialize_from_header(senderHeader, senderBuffer);

	int offset = sizeof(struct s_header);
	serialize_from_int(senderBuffer, pid, &offset);

	log_info(LOG, "Se envia pid");
	int status = send(socket, senderBuffer, sizeof(struct s_header) + sizeof(int), 0);

	if(status == -1) {log_error(LOG, "Error: no se envio pcb");        close(socket);}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket"); close(socket);}
	else{
		log_info(LOG, "Pid enviado correctamente");
		log_info(LOG, "Se recibe mensaje");
	}

      	status = recv(socket, &receiver, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);free(senderBuffer);}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(socket);free(senderBuffer);}

		else{
			if (receiver.id == OK){
				log_info(LOG, "Se recibio Ok");
				free(senderBuffer);

			}else{
				log_error(LOG, "Fallo notificando cambio de proceso activo");
				free(senderBuffer);
			    }
			}

}

int enviar_bytes_por_socket(int socket, int nroPag, int offset, int tamanioData, void* data, char* string_error, t_log* LOG){

	struct s_header senderHeader;
	struct s_header receiverHeader;

	senderHeader.id = ALMACENAR_BYTES;
	senderHeader.size = (sizeof(int) + sizeof(int) + sizeof(int) + tamanioData);

	void* senderBuffer = malloc(sizeof(struct s_header) + sizeof(int) + sizeof(int) + sizeof(int) + tamanioData);

	serialize_from_header(senderHeader, senderBuffer);

	int off = sizeof(struct s_header);
	serialize_from_int(senderBuffer, nroPag, &off);
	serialize_from_int(senderBuffer, offset, &off);
	serialize_from_int(senderBuffer, tamanioData, &off);
	memcpy(senderBuffer+off, data, tamanioData );

	int status = send(socket, senderBuffer, sizeof(struct s_header) + senderHeader.size, 0);
	if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	}

	else{
		log_info(LOG, "Se procede a recibir mensaje");
      	status = recv(socket, &receiverHeader, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	}

		else{
			switch(receiverHeader.id){

				case OK:
					log_info(LOG, "enviado correctamente");
					free(senderBuffer);
					return OK;

				case FALLO :
					log_error(LOG, "Fallo almacenar bytes");
					//string_error =  string_duplicate("Fallo almacenar bytes");
					free(senderBuffer);
					return FALLO;

				case PEDIDO_INVALIDO:
					log_error(LOG, "Stack Overflow");
					//string_error =  string_duplicate("Stack Overflow");
					free(senderBuffer);
					return PEDIDO_INVALIDO;

				case NO_HAY_MEMORIA:
					log_error(LOG, "No hay memoria");
					//string_error =  string_duplicate("No hay memoria");
					free(senderBuffer);
					return NO_HAY_MEMORIA;
				default:
					return FALLO;
			}
		}
	}


	free(senderBuffer);
	return FALLO;
}

int pedir_bytes_por_socket(int socket, int nroPag, int offset, int tamanio, void* receiverBuffer, char* string_error, t_log* LOG){

	struct s_header receiver;
	struct s_header sender = {
			.id = SOLICITAR_BYTES,
		  .size = 3 * sizeof(int),
	};

	void* senderBuffer = malloc(sizeof(struct s_header) + sender.size);

	serialize_from_header(sender, senderBuffer);

	int offsetLoc = sizeof(struct s_header);
	serialize_from_int (senderBuffer, nroPag,  &offsetLoc);
	serialize_from_int (senderBuffer, offset,  &offsetLoc);
	serialize_from_int (senderBuffer, tamanio, &offsetLoc);

	int status = send(socket, senderBuffer, sizeof(struct s_header) + sender.size, 0);
	if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");free(senderBuffer);}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");  free(senderBuffer);}

	else{log_info(LOG, "Se recibe mensaje");

      	status = recv(socket, &receiver, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje"); free(senderBuffer);}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	 free(senderBuffer);}

		else{
			switch(receiver.id){
			case OK :
				log_info(LOG, "Se recibio header esperado");

				status = recv(socket, receiverBuffer, receiver.size, 0);
				log_info(LOG, "Se recibe payload correctamente");
				free(senderBuffer);
				return OK;

			case FALLO :
				log_error(LOG, "Fallo pedido");
				//string_error =  string_duplicate("Fallo pedido");
				free(senderBuffer);
				return FALLO;

			case PEDIDO_INVALIDO:
				log_error(LOG, "Pedido invalido");
				//string_error =  string_duplicate("Pedido invalido");
				free(senderBuffer);
				return PEDIDO_INVALIDO;

			case NO_HAY_MEMORIA:
				log_error(LOG, "No hay memoria");
				//string_error =  string_duplicate("No hay memoria");
				free(senderBuffer);
				return NO_HAY_MEMORIA;

			default:
				return FALLO;

			}

		}
	}

	free(senderBuffer);
	return FALLO;

}

void send_wait(int socket,  t_log* LOG){

	struct s_header senderHeader;
	struct s_header recvHeader;

	senderHeader.id = SEM_WAIT;
	senderHeader.size = 0;

	void* sendBuffer = malloc(sizeof(struct s_header));
	serialize_from_header(senderHeader, sendBuffer);

	int status = send(socket, sendBuffer, sizeof(struct s_header), 0);
	if (status != 0 && status != -1){

	recv(socket, &recvHeader, sizeof(struct s_header), 0);

	if (recvHeader.id == OK)
		log_info(LOG,"Se envio wait a Nucleo");

	}

	else log_error(LOG, "error enviando wait");

	free(sendBuffer);

}

void socket_ok_response(int socket){

	struct s_header senderHeader;

	senderHeader.id = OK;
	senderHeader.size = 0;

	void* sendBuffer = malloc(sizeof(struct s_header));
	serialize_from_header(senderHeader, sendBuffer);

	send(socket, sendBuffer, sizeof(struct s_header), 0);

	free(sendBuffer);
}

void socket_fallo_response(int socket){

	struct s_header senderHeader;

	senderHeader.id = FALLO;
	senderHeader.size = 0;

	void* sendBuffer = malloc(sizeof(struct s_header));
	serialize_from_header(senderHeader, sendBuffer);

	send(socket, sendBuffer, sizeof(struct s_header), 0);

	free(sendBuffer);
}

void socket_send_end(int socket){

	struct s_header sender = {
			.id = CPU_FIN,
		  .size = 0,
	};

	void* sendBuffer = malloc(sizeof(struct s_header));
	serialize_from_header(sender, sendBuffer);

	send(socket, sendBuffer, sizeof(struct s_header), 0);

	free(sendBuffer);

}

char* receive_string_de_ids(int socket, int tipo_id, t_log* LOG){

	struct s_header receive_header;

	log_info(LOG, string_from_format("Se prepara para recibir id: %d",tipo_id));

	recv(socket, &receive_header, sizeof(struct s_header), 0);

	if (receive_header.id != tipo_id){
		log_error(LOG, string_from_format("Se falla al recibir id: %d",tipo_id));
		char* vacio = "";
		return vacio;
	}

	else{

		log_info(LOG, string_from_format("Se recibe header de id: %d",tipo_id));

		char* listaIDS = malloc(receive_header.size);

		recv(socket,listaIDS,receive_header.size,0);

		log_info(LOG,string_from_format("Se cargaron los ids : %s", listaIDS));

		socket_ok_response(socket);

		return listaIDS;

	}

}

int	get_value_sharvar(int NucleoSock, char* id, t_log* LOG){

	int valor = 0;

	struct s_header senderHeader;
	struct s_header receiver;

	senderHeader.id   = SHARVAR_GET;
	senderHeader.size = strlen(id)+1;

	void* sendBuffer = malloc(sizeof(struct s_header) + senderHeader.size);

	serialize_from_header(senderHeader, sendBuffer);
	memcpy(sendBuffer+sizeof(struct s_header), id, senderHeader.size);

	int status = send(NucleoSock, sendBuffer, sizeof(struct s_header) + senderHeader.size, 0);

	if(status == -1) {log_error(LOG, "Error: no se envio get");}
		else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");}
		else{
			log_info(LOG, string_from_format("se pide valor de: %s", id));
			log_info(LOG, "Se recibe mensaje");
		}

	      	status = recv(NucleoSock, &receiver, sizeof(struct s_header), 0);
			if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");free(sendBuffer);}
			else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");  free(sendBuffer);}

			else{
				if (receiver.id == OK){
					log_info(LOG, "Se recibio Ok, se recibe valor");
					recv(NucleoSock, &valor, sizeof(int), 0);


				}else{
					log_error(LOG, "Fallo get");
				    }
				}

    free(sendBuffer);
	return valor;


}

void set_value_sharvar(int NucleoSock, char* id, int valor, t_log* LOG){

	struct s_header senderHeader;
	struct s_header receiver;

	senderHeader.id   = SHARVAR_SET;
	senderHeader.size = strlen(id) + 1 + sizeof(int);

	void* sendBuffer = malloc(sizeof(struct s_header) + senderHeader.size);

	serialize_from_header(senderHeader, sendBuffer); int offset = sizeof(struct s_header);
	memcpy(sendBuffer+offset, id, strlen(id) + 1);		 offset = offset + strlen(id) + 1;
	memcpy(sendBuffer+offset, &valor, sizeof(int));

	int status = send(NucleoSock, sendBuffer, sizeof(struct s_header) + senderHeader.size, 0);

	if(status == -1) {log_error(LOG, "Error: no se envio set");}
			else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");}
			else{
				log_info(LOG, string_from_format("se envia set de valor: %d a variable: %s", valor, id));
				log_info(LOG, "Se recibe mensaje");
			}

		      	status = recv(NucleoSock, &receiver, sizeof(struct s_header), 0);
				if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");free(sendBuffer);}
				else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");  free(sendBuffer);}

				else{
					if (receiver.id == OK){
						log_info(LOG, "Se recibio Ok");


					}else{
						log_error(LOG, "Fallo set");
					    }
					}

	    free(sendBuffer);

}

char* separarID(char* instruccion){
	int i;
	for (i = 0; instruccion[i] != '\0'; i++){


		if (instruccion[i] == '\n'){

			instruccion[i] = '\0';
		}
	}

 return instruccion;
}


