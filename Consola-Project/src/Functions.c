/*
 * Functions.c
 *
 *  Created on: 1/5/2016
 *      Author: utnso
 */

#include "Consola.h"

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

	config_destroy(config_file);
	return config;
}

struct s_protocol get_sender_from_file(char* path){
	FILE* file;
	struct s_protocol sender;
	long size;

	//Inicializo struct de protocolo para el sender;
	init_struct_protocol(&sender);

	if(path[0] == '*'){
		char* dir = string_duplicate("/home/utnso/scripts-ansisop/scripts");
		string_append(&dir, path+sizeof(char));
		path = dir;

		printf("\n%s \n", dir);
	}

	if (string_ends_with(path, "/")){
		printf("Path invalido, intente nuevamente\n");
		return sender;
	}


	//Apertura de archivo en modo lectura
	file = fopen(path, "r");

	if (file == NULL){
		printf("Path invalido, intente nuevamente\n");
		return sender;
	}

	//Leo propiedades del archivo para saber la longitud
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	if (size == 0){
		return sender;
	}


	//Leo archivo hasta longitud sacada.
	char* file_string = malloc( (sizeof(char) * size) +1);
	fread(file_string, sizeof(char), size, file);
	file_string[size] = '\0';

	sender.payload = malloc((sizeof(char) * size) +1);
	memcpy( sender.payload, file_string, (sizeof(char) * size) +1);

	free(file_string);


	//Armo header de archivo.
	sender.id = ARCHIVO_ANSISOP;
	sender.size = size + 1;

	//Cierro archivo
	fclose(file);

	return sender;
}

int realize_handshake(int socket, t_log* LOG){
	int ret;

	struct s_header sendHandshake;
	struct s_header recvHandshake;

	sendHandshake.id = SALUDO_CONSOLA;
	sendHandshake.size = 0;

	void* senderBufferHandshake = malloc(sizeof(struct s_header));

	serialize_from_header(sendHandshake, senderBufferHandshake);

	int result1 = send(socket, senderBufferHandshake, sizeof(struct s_header), 0);
	if(result1 == -1){
		log_error(LOG, "SWAP no está conectado");
	}else{
		log_info(LOG, "Se recibe mensaje de Nucleo");
      	result1 = recv(socket, &recvHandshake, sizeof(struct s_header), MSG_WAITALL); //recv bloqueante
		if(result1 == -1)
		{
			log_error(LOG, "Error: no se recibio el mensaje de Nucleo");
		}else{
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
void init_struct_protocol(struct s_protocol* protocol){

	protocol->id = 0;
	protocol->size = 0;
	//memset(&protocol->payload, ' ', sizeof(protocol->payload));
	protocol->payload = "";

}

void serialize_from_protocol(struct s_protocol protocol, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, protocol.id, &offset);
	serialize_from_int(buffer, protocol.size, &offset);
	memcpy(buffer+offset, protocol.payload, protocol.size);

}

void serialize_from_header(struct s_header header, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, header.id, &offset);
	serialize_from_int(buffer, header.size, &offset);

}

void deserialize_to_protocol(struct s_protocol* protocol, void* buffer){
	int offset = 0;

	deserialize_to_int(buffer, &(protocol->id), &offset);
	deserialize_to_int(buffer, &(protocol->size), &offset);
	//deserialize_to_string(buffer, protocol->payload, protocol->size, &offset);
}
