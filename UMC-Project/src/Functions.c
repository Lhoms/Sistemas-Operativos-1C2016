/*
 * Functions.c
 *
 *  Created on: 25/4/2016
 *      Author: Lajew
 *      Descrp: Funciones de UMC
 */

#include "UMC.h"

s_conf get_config(char* path, t_log* LOG){

	s_conf config;

	t_config* config_file;

	int cantConfig = 0;
	char* logMessage = string_new();

	log_info(LOG, "Leyendo archivo de configuración");
	log_info(LOG, path);
	log_info(LOG,"\n////////////////////////////////////////////////////////////////////////comienza módulo.");

	config_file = config_create(path);
	cantConfig = config_keys_amount(config_file);

	if (cantConfig == 0){
		log_error(LOG, "Archivo de configuración vacío");
	}


	config.PUERTO = string_duplicate(config_get_string_value(config_file, "PUERTO"));
	if (!string_is_empty(config.PUERTO)){
		logMessage = string_from_format("PUERTO: %s", config.PUERTO);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró puerto del nucleo y cpus");
	}


	config.IP_SWAP = string_duplicate(config_get_string_value(config_file, "IP_SWAP"));
	if (!string_is_empty(config.IP_SWAP)){
		logMessage = string_from_format("IP SWAP: %s", config.IP_SWAP);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró direccion ip del proceso Swap");
	}


	config.PUERTO_SWAP = string_duplicate(config_get_string_value(config_file, "PUERTO_SWAP"));
	if (!string_is_empty(config.PUERTO_SWAP)){
		logMessage = string_from_format("PUERTO SWAP: %s", config.PUERTO_SWAP);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró puerto del proceso Swap");
	}


	config.MARCOS = config_get_int_value(config_file, "MARCOS");
	if (config.MARCOS != 0){
		logMessage = string_from_format("MARCOS: %d", config.MARCOS);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró numero de marcos disponibles en el sistema");
	}


	config.MARCO_SIZE = config_get_int_value(config_file, "MARCO_SIZE");
	if (config.MARCO_SIZE != 0){
		logMessage = string_from_format("MARCOS SIZE: %d", config.MARCO_SIZE);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró tamaño de marco");
	}


	config.MARCO_X_PROC = config_get_int_value(config_file, "MARCO_X_PROC");
	if (config.MARCO_X_PROC != 0){
		logMessage = string_from_format("MARCOS POR PROCESO: %d", config.MARCO_X_PROC);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró cantidad de marcos asignadas a programas ansisop");
	}


	config.ALGORITMO = string_duplicate(config_get_string_value(config_file, "ALGORITMO"));
	if (!string_is_empty(config.ALGORITMO)){
		logMessage = string_from_format("ALGORITMO: %s", config.ALGORITMO);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró algoritmo de reemplazo en tabla de páginas");
	}

	config.ENTRADAS_TLB = config_get_int_value(config_file, "ENTRADAS_TLB");
	if (config.ENTRADAS_TLB != 0){
		logMessage = string_from_format("ENTRADAS_TLB: %d", config.ENTRADAS_TLB);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró cantidad de entradas TLB disponibles");
	}


	config.RETARDO = config_get_int_value(config_file, "RETARDO");
	if (config.RETARDO != 0){
		logMessage = string_from_format("RETARDO: %d", config.RETARDO);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró tiempo de retardo");
	}

	config.RUTA_DUMP = string_duplicate(config_get_string_value(config_file, "RUTA_DUMP"));
	if (!string_is_empty(config.RUTA_DUMP)){
		logMessage = string_from_format("RUTA_DUMP: %s", config.RUTA_DUMP);
		log_info(LOG, logMessage);
		if(!string_ends_with(config.RUTA_DUMP, "\\")){
			string_append(&config.RUTA_DUMP, "\\");
		}
	}else{
		log_info(LOG, "No se encontró ruta para dump. Se escribirá en carpeta de proyecto");
	}

	config_destroy(config_file);
	return config;
}

void init_struct_protocol(struct s_protocol* protocol){

	protocol->id = 0;
	protocol->size = 0;
	protocol->payload = "";

}

void init_struct_header(struct s_header* header){

	header->id = 0;
	header->size = 0;

}

void serialize_from_protocol(struct s_protocol protocol, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, protocol.id, &offset);
	serialize_from_int(buffer, protocol.size, &offset);
	serialize_from_string(buffer, protocol.payload, &offset);

}

void serialize_from_header(struct s_header header, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, header.id, &offset);
	serialize_from_int(buffer, header.size, &offset);

}

int realize_handshake(int socket, t_log* LOG){
	int ret;

	struct s_header sendHandshake;
	struct s_header recvHandshake;

	sendHandshake.id = SALUDO_UMC;
	sendHandshake.size = 0;

	void* senderBufferHandshake = malloc(sizeof(struct s_header));

	serialize_from_header(sendHandshake, senderBufferHandshake);

	int result1 = send(socket, senderBufferHandshake, sizeof(struct s_header), 0);
	if(result1 == -1){
		log_error(LOG, "Núcleo no está conectado");
	}else{
		log_info(LOG, "Se recibe mensaje del Swap");
      	result1 = recv(socket, &recvHandshake, sizeof(struct s_header), MSG_WAITALL); //recv bloqueante
		if(result1 == -1)
		{
			log_error(LOG, "Error: no se recibio el mensaje de Núcleo");
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

void socket_bytes_response(int socket, int tamanio, char* bytes){

	struct s_header senderHeader;
	int offset = 0;

	senderHeader.id = OK;
	senderHeader.size = tamanio;

	void* sendBuffer = malloc(sizeof(struct s_header) + senderHeader.size);
	serialize_from_int(sendBuffer, senderHeader.id, &offset);
	serialize_from_int(sendBuffer, senderHeader.size, &offset);
	memcpy(sendBuffer+offset, bytes, tamanio);

	send(socket, sendBuffer, sizeof(struct s_header)+senderHeader.size, 0);

	log_info(LOG, "Se responde al pedido: %s", string_substring(bytes, 0, tamanio));

	free(sendBuffer);
}

void deserialize_init_program(struct s_init_program* data, int size_content, void* buffer){
	int offset = 0;

	data->codigo = malloc(size_content);

	deserialize_to_int(buffer, &(data->pid), &offset);
	deserialize_to_int(buffer, &(data->cant_pag), &offset);
	deserialize_to_string(buffer, data->codigo, size_content, &offset);
}

void deserialize_req_bytes(struct s_req_bytes* data, void* buffer){
	int offset = 0;

	deserialize_to_int(buffer, &(data->pagina), &offset);
	deserialize_to_int(buffer, &(data->offset), &offset);
	deserialize_to_int(buffer, &(data->tamanio), &offset);

}

void deserialize_save_bytes(struct s_req_bytes* data, char*bytes, int size, void* buffer){
	int offset = 0;

	deserialize_to_int(buffer, &(data->pagina), &offset);
	deserialize_to_int(buffer, &(data->offset), &offset);
	deserialize_to_int(buffer, &(data->tamanio), &offset);
	deserialize_to_string(buffer, bytes, size, &offset);
}

int request_bytes(struct s_req_bytes* data, int pid, int socketSwap, char* resp){
	int ret;
	int offset = 0;
	char* bytes = malloc(config.MARCO_SIZE);//string_new();
	int frame; //Frame en el que se actualiza

	log_info(LOG, "Se solicitaron bytes: se bloquea memoria para la búsqueda");
	pthread_mutex_lock( &mutex_memory );

	//Busco página en TLB(si está activa).
	if(config.ENTRADAS_TLB > 0){
		log_info(LOG, "TLB habilitada. Se busca proceso");
		s_tlb* tlb_entry = find_process_page_in_tlb(pid, data->pagina);

		if (tlb_entry == NULL){
			log_info(LOG, "No se encontró entrada en la TLB");
			usleep(config.RETARDO * 1000);
			//Si no está en TLB, busco en la tabla de páginas
			log_info(LOG, "Se busca en la tabla de páginas");
			s_process* process = find_page_table_from_pid(pid);
			if(process != NULL){
				s_page_table* page = get_page_from_process(process, data->pagina);

				if(page->memory_flag == 1){
					log_info(LOG, "La página %d se encuentra en memoria", data->pagina);
					//Está en memoria, obtengo el marco, busco el dato y actualizo TLB.
					offset = ( config.MARCO_SIZE * page->marco ) + data->offset;
					memcpy(resp, memory+offset, data->tamanio);
					//Actualizo TLB
					add_page_to_tlb(socketSwap, page, pid, LECTURA);
					ret = OK;
					//Actualizo tiempo de referencia y bitUso
					page->bituso   = 1;
					page->ref_time = get_actual_time_integer();
				}else{
					//Page fault
					log_info(LOG, "Ocurrió un page fault. Se pide información a SWAP");
					//1) Pido información a SWAP
					int status = request_bytes_to_SWAP(data, pid, socketSwap, bytes);
					if (status == OK){
						log_info(LOG, "Se recibió el pedido de SWAP. Se graba información");
						//2) Actualizo información
						ret = save_request_bytes(data, pid, bytes, socketSwap, &frame, LECTURA);
						//3) Si actualicé la información, devuelvo los bytes que me pidieron(pero ya desde memoria)
						if (ret == OK){
							offset = ( config.MARCO_SIZE * frame ) + data->offset;
							memcpy(resp, memory+offset, data->tamanio);
						}else{
							ret = NO_HAY_MEMORIA;
							log_error(LOG, "No se pudo cargar la información en memoria");
						}
					}else{
						ret = status;
					}
				}
			}else{
				log_error(LOG, "Error. Proceso %d no inicializado", pid);
				ret = ERROR;
			}
		}else{
			//Se encontró en la TLB. Se actualiza tiempo de referencia
			log_info(LOG, "Se encontró en la TLB. Se bloquea para actualización de TIMEREF");
			pthread_mutex_lock( &mutex_tlb );
				tlb_entry->ref_time = get_actual_time_integer();
			pthread_mutex_unlock( &mutex_tlb );
			log_info(LOG, "Se encontró en la TLB. Se desbloquea para actualización de TIMEREF");
			log_info(LOG, "Se encontró entrada en la TLB. Marco: %d", tlb_entry->marco);

			offset = ( tlb_entry->marco * config.MARCO_SIZE ) + data->offset;
			memcpy(resp, memory+offset, data->tamanio);
			ret = OK;
		}
	}else{
		usleep(config.RETARDO * 1000);
		log_info(LOG, "Se busca en la tabla de páginas");
		s_process* process = find_page_table_from_pid(pid);
		if(process != NULL){
			s_page_table* page = get_page_from_process(process, data->pagina);

			if(page->memory_flag == 1){
				log_info(LOG, "La página %d se encuentra en memoria", data->pagina);
				//Está en memoria, obtengo el marco, busco el dato y actualizo TLB.
				offset = ( config.MARCO_SIZE * page->marco ) + data->offset;
				memcpy(resp, memory+offset, data->tamanio);
				ret = OK;

				//Actualizo tiempo de referencia y bitUso
				page->bituso   = 1;
				page->ref_time = get_actual_time_integer();
			}else{
				//Page fault
				log_info(LOG, "Ocurrió un page fault. Se pide información a SWAP");
				//1) Pido información a SWAP
				int status = request_bytes_to_SWAP(data, pid, socketSwap, bytes);
				if (status == OK){
					log_info(LOG, "Se recibió el pedido de SWAP. Se graba información");
					//2) Actualizo información
					ret = save_request_bytes(data, pid, bytes, socketSwap, &frame, LECTURA);
					//3) Si actualicé la información, devuelvo los bytes que me pidieron(pero ya desde memoria)
					if (ret == OK){
						log_info(LOG, "Se escribe en memoria la información recibida");
						offset = ( config.MARCO_SIZE * frame ) + data->offset;
						memcpy(resp, memory+offset, data->tamanio);
					}else{
						ret = NO_HAY_MEMORIA;
						log_error(LOG, "No se pudo cargar la información en memoria: no hay memoria");
					}
				}else{
					ret = status;
				}
			}
		}else{
			log_error(LOG, "Error. Proceso %d no inicializado", pid);
			ret = ERROR;
		}

	}

	pthread_mutex_unlock( &mutex_memory );
	log_info(LOG, "Se solicitaron bytes: se desbloquea memoria para la búsqueda");

	//Sleep que simula el retraso de ir a buscar en memoria
	usleep(config.RETARDO * 1000);

	free(bytes);
	return ret;
}

int save_bytes(struct s_req_bytes* data, int pid, int socketSwap, char* buffer){
	int ret;
	int offset = 0;
	char* bytes = malloc(config.MARCO_SIZE);//string_new();
	int frame; //Frame en el que se actualiza

	log_info(LOG, "Se solicitaron almacenar bytes: se bloquea memoria para la búsqueda");
	pthread_mutex_lock( &mutex_memory );

	//Busco página en TLB(si está activa).
	if(config.ENTRADAS_TLB > 0){
		log_info(LOG, "TLB habilitada. Se busca proceso");
		s_tlb* tlb_entry = find_process_page_in_tlb(pid, data->pagina);

		if (tlb_entry == NULL){
			log_info(LOG, "No se encontró entrada en la TLB");
			//Si no está en TLB, busco en la tabla de páginas
			usleep(config.RETARDO * 1000);
			log_info(LOG, "Se busca en la tabla de páginas");
			s_process* process = find_page_table_from_pid(pid);
			if(process != NULL){
				s_page_table* page = get_page_from_process(process, data->pagina);

				if(page->memory_flag == 1){
					log_info(LOG, "La página %d se encuentra en memoria", data->pagina);
					//Está en memoria, obtengo el marco, actualizo memoria
					offset = ( config.MARCO_SIZE * page->marco ) + data->offset;
					memcpy(memory+offset, buffer, data->tamanio);
					//Actualizo TLB
					add_page_to_tlb(socketSwap, page, pid, ESCRITURA);
					ret = OK;
					//Actualizo tiempo de referencia
					page->ref_time = get_actual_time_integer();
					page->modif_flag = 1;
				}else{
					//Page fault
					log_info(LOG, "Ocurrió un page fault. Se pide información a SWAP");
					//1) Pido información a SWAP
					int status = request_bytes_to_SWAP(data, pid, socketSwap, bytes);
					if (status == OK){
						log_info(LOG, "Se recibió el pedido de SWAP. Se graba información");
						//2) Actualizo información (PARA QUE SEPA QUÉ LE TIENE QUE PONER AL FLAG DE MODIFICACIÓN)
						ret = save_request_bytes(data, pid, bytes, socketSwap, &frame, ESCRITURA);
						//3) Si actualicé la información que estaba en SWAP, escribo en memoria
						if (ret == OK){
							log_info(LOG, "Se escribe en memoria la información recibida");
							offset = ( config.MARCO_SIZE * frame ) + data->offset;
							memcpy(memory+offset, buffer, data->tamanio);
							page->modif_flag = 1;
						}else{
							ret = NO_HAY_MEMORIA;
							log_error(LOG, "No se pudo cargar la información en memoria");
						}
					}else{
						ret = status;
					}
				}
			}else{
				log_error(LOG, "Error. Proceso %d no inicializado", pid);
				ret = ERROR;
			}
		}else{
			//Se encontró en la TLB. Se actualiza tiempo de referencia, info y flag de modificación
			log_info(LOG, "Se encontró en la TLB. Se bloquea para actualización de TIMEREF y bit modificado");
			pthread_mutex_lock( &mutex_tlb );
				tlb_entry->ref_time = get_actual_time_integer();
				tlb_entry->modif_flag = 1;
			pthread_mutex_unlock( &mutex_tlb );
			log_info(LOG, "Se encontró en la TLB. Se desbloquea para actualización de TIMEREF y bit modificado");
			log_info(LOG, "Se encontró entrada en la TLB. Marco: %d", tlb_entry->marco);
			log_info(LOG, "Se carga en memoria la información obtenida en TLB");
			offset = ( tlb_entry->marco * config.MARCO_SIZE ) + data->offset;
			memcpy(memory+offset, buffer, data->tamanio);
			ret = OK;
			//busco página en tabla de páginas para setear el flag de modificado
			s_process* process_m = find_page_table_from_pid(pid);
			s_page_table* page_m = get_page_from_process(process_m, data->pagina);
			page_m->modif_flag = 1;
		}
	}else{
		usleep(config.RETARDO * 1000);
		log_info(LOG, "Se busca en la tabla de páginas");
		s_process* process = find_page_table_from_pid(pid);
		if(process != NULL){
			s_page_table* page = get_page_from_process(process, data->pagina);

			if(page->memory_flag == 1){
				log_info(LOG, "La página %d se encuentra en memoria", data->pagina);
				//Está en memoria, obtengo el marco, actualizo memoria.
				offset = ( config.MARCO_SIZE * page->marco ) + data->offset;
				memcpy(memory+offset, buffer, data->tamanio);
				ret = OK;
				//Actualizo tiempo de referencia
			    page->ref_time = get_actual_time_integer();
			    page->modif_flag = 1;
			}else{
				//Page fault
				log_info(LOG, "Ocurrió un page fault. Se pide información a SWAP");
				//1) Pido información a SWAP
				int status = request_bytes_to_SWAP(data, pid, socketSwap, bytes);
				if (status == OK){
					log_info(LOG, "Se recibió el pedido de SWAP. Se graba información");
					//2) Actualizo información
					ret = save_request_bytes(data, pid, bytes, socketSwap, &frame, ESCRITURA);
					//3) Si actualicé la información que estaba en SWAP, escribo en memoria
					if (ret == OK){
						log_info(LOG, "Se escribe en memoria la información recibida");
						offset = ( config.MARCO_SIZE * frame ) + data->offset;
						memcpy(memory+offset, buffer, data->tamanio);
						page->modif_flag = 1;
					}else{
						ret = NO_HAY_MEMORIA;
						log_error(LOG, "No se pudo cargar la información en memoria");
					}
				}else{
					ret = status;
				}
			}
		}else{
			log_error(LOG, "Error. Proceso %d no inicializado", pid);
			ret = ERROR;
		}

	}

	pthread_mutex_unlock( &mutex_memory );
	log_info(LOG, "Se solicitaron almacenar bytes: se desbloquea memoria para la búsqueda");

	//Sleep que simula el retraso de ir a buscar en memoria
	usleep(config.RETARDO * 1000);
	free(bytes);
	return ret;
}

int save_request_bytes(struct s_req_bytes* data, int pid, char* bytes, int socketSwap, int* marco, e_operacion operacion){
	int ret;

	//Actualizo información
	log_info(LOG, "Se guarda información recibida. Se bloquea vector de frames");
	pthread_mutex_lock( &mutex_frames );
		//1) Verifico disponibilidad en memoria
		int frame = get_first_free_frame();

		log_info(LOG, "Se guarda información recibida. Se bloquea tabla de páginas");
		pthread_mutex_lock( &mutex_page_table );

		t_list* presentPages = get_present_pages_from_pid(pid);

		if( ( frame == -1 && list_size(presentPages) != 0 ) || list_size(presentPages) == config.MARCO_X_PROC){
			//1.1)No hay marcos libres, pero páginas presentes, o se alcanzó la cantidad máxima de marcos por proceso
			//1.1.1) Se busca víctima con el algoritmo configurado.
				log_info(LOG, "No hay marcos libres en memoria(con páginas presentes) o se alcanzó la cantidad máxima de marcos por proceso");
				s_page_table* page;
				if (string_equals_ignore_case(config.ALGORITMO, "CLOCK")){
					//1.1.1.1) Elijo víctima con el algoritmo CLOCK
					log_info(LOG, "Elijo víctima usando el algoritmo CLOCK");
					page = get_victim_using_clock(presentPages);
				}else{
					log_info(LOG, "Elijo víctima usando el algoritmo CLOCK MODIFICADO");
					//1.1.1.2) Elijo víctima con el algoritmo CLOCK MODIFICADO
					page = get_victim_using_clock_m(presentPages);
				}
				frame = page->marco;
				page->memory_flag = 0;

				log_info(LOG, "Limpio víctima de TLB, si está");
				if( config.ENTRADAS_TLB > 0) clear_page_in_tlb(pid, page->pagina);

				if(page->modif_flag == 1){
					log_info(LOG, "La página a reemplazar está modificada. Se envía actualización a SWAP");
					//Si la página está modificada, le tengo que avisar a SWAP de la modificación
					char* content = malloc(config.MARCO_SIZE);//string_new();
					int offset = ( config.MARCO_SIZE * frame );
					memcpy(content, memory+offset, config.MARCO_SIZE);
					int status = update_page_in_SWAP(socketSwap, pid, page->pagina, content);
					if (status == FALLO || status == ERROR){
						log_info(LOG, "No se pudo actualizar la información de la página en SWAP");
						//si no lo pude actualizar en SWAP, no actualizo la memoria tampoco
						frame = -1;
					}else{
						log_info(LOG, "Información de la página actualizada en SWAP correctamente");
						page->modif_flag = 0; //Actualicé en SWAP la página
						//Si estoy leyendo, significa que la memoria va a quedar igual que en SWAP, sino la voy a estar actualizando.
						/*if(operacion == LECTURA){
							page->modif_flag = 0;
						}else{
							page->modif_flag = 1;
						}*/
					}
					free(content);
				}
		}
		//LOGGEAR
			if(list_size(presentPages) == 0){
				log_info(LOG, string_from_format("No hay páginas presentes para el proceso %d", pid));
			}
		//LOGGEAR
		pthread_mutex_unlock( &mutex_page_table );
		log_info(LOG, "Se guarda información recibida. Se desbloquea tabla de páginas");

		//1.2) Se carga en memoria
		if (frame != -1){
			log_info(LOG, "Se guarda, finalmente, información en memoria");
			save_in_memory(socketSwap, pid, data->pagina, frame, bytes, operacion);
			*marco = frame;
			ret = OK;
		}else{
			log_info(LOG, "Error al grabar información");
			ret = ERROR;
		}
	pthread_mutex_unlock( &mutex_frames );
	log_info(LOG, "Se guarda información recibida. Se desbloquea vector de frames");

	return ret;
}

int update_page_in_SWAP(int socket, int pid, int pagina, char* buffer){
	int ret;
	int offset = 0;

	struct s_header header;

	header.id = ALMACENAR_BYTES;
	header.size = sizeof(pid) + sizeof(pagina) + config.MARCO_SIZE;//strlen(buffer);

	void* sendBuffer = malloc(sizeof(struct s_header) + header.size);

	serialize_from_int(sendBuffer, header.id, &offset);
	serialize_from_int(sendBuffer, header.size, &offset);
	serialize_from_int(sendBuffer, pid, &offset);
	serialize_from_int(sendBuffer, pagina, &offset);
	serialize_n_from_string(sendBuffer, buffer, config.MARCO_SIZE, &offset);

	//envío
	log_info(LOG, "Se actualiza información de página en SWAP. Se bloquea");
	pthread_mutex_lock( &mutex_swap );

	log_info(LOG, "Se envía actualización de información del PID: %d en página %d a SWAP. Contenido: %s", pid, pagina, string_substring(buffer, 0, config.MARCO_SIZE));
	int status = send(socket, sendBuffer, sizeof(struct s_header) + header.size, 0);
	if (status == ERROR){
		log_error(LOG, "Error al llamar al proceso SWAP");
		ret =  ERROR;
	}else{
		struct s_header respHeader;
		init_struct_header(&respHeader);
		status = recv(socket, &respHeader, sizeof(struct s_header), MSG_WAITALL);
		if (status == ERROR){
			log_error(LOG, "Error al recibir respuesta del proceso SWAP");
			ret = ERROR;
		}else{
			if (respHeader.id == OK){
				ret = OK;
				log_info(LOG, "Se recibió correctamente la confirmación del proceso SWAP");
			}else{
				log_error(LOG, "SWAP respondió FALLO.");
				ret = FALLO;
			}
		}
	}


	pthread_mutex_unlock( &mutex_swap );
	log_info(LOG, "Se actualiza información de página en SWAP. Se desbloquea");

	free(sendBuffer);
	return ret;
}
int request_bytes_to_SWAP(struct s_req_bytes* data, int pid, int socketSwap, char* resp){
	int ret;
	int offset = 0;
	struct s_header header;

	header.id = SOLICITAR_BYTES;
	header.size = sizeof(pid) + sizeof(data->pagina);

	void* sendBuffer = malloc(sizeof(struct s_header) + header.size);

	serialize_from_int(sendBuffer, header.id, &offset);
	serialize_from_int(sendBuffer, header.size, &offset);
	serialize_from_int(sendBuffer, pid, &offset);
	serialize_from_int(sendBuffer, data->pagina, &offset);

	//envío
	log_info(LOG, "Se le piden bytes a SWAP. Se bloquea el uso de la misma");
	pthread_mutex_lock( &mutex_swap );

	log_info(LOG, "Se pide información al proceso SWAP del PID: %d, página: %d", pid, data->pagina);
	int status = send(socketSwap, sendBuffer, sizeof(struct s_header) + header.size, 0);
	if (status == ERROR){
		log_error(LOG, "Error al llamar al proceso SWAP");
		ret =  ERROR;
	}else{
		struct s_header* respHeader=malloc(sizeof(struct s_header));
		init_struct_header(respHeader);
		status = recv(socketSwap, respHeader, sizeof(struct s_header), MSG_WAITALL);
		if (status == ERROR){
			log_error(LOG, "Error al recibir respuesta del proceso SWAP");
			ret = ERROR;
		}else{
			if (respHeader->id != FALLO){
				status = recv(socketSwap, resp, respHeader->size, 0);
				if (status == ERROR){
					log_error(LOG, "Error al recibir respuesta del proceso SWAP");
					ret = ERROR;
				}else{
					ret = OK;
					log_info(LOG, "Se recibió correctamente el pedido al proceso SWAP: %s", string_substring(resp, 0, data->tamanio));
				}
			}else{
				log_error(LOG, "SWAP respondió FALLO.");
				ret = FALLO;
			}
		}
	}

	pthread_mutex_unlock( &mutex_swap );
	log_info(LOG, "Se le piden bytes a SWAP. Se desbloquea el uso de la misma");

	free(sendBuffer);
	return ret;
}

int search_pid_from_cpu(int socket){

	bool search(void*parametro){
		s_cpu* entry = (s_cpu*) parametro;
		return entry->socket == socket;
	}

	pthread_mutex_lock( &mutex_cpu );
		s_cpu* cpu = list_find(list_cpu, search);
	pthread_mutex_unlock( &mutex_cpu );

	return cpu->pid;
}
int request_memory_to_SWAP(int socket, int pid, int cant_pag){

  struct s_header header;
  int ret;
  int offset = 0;

  //Pido memoria para reservar a SWAP
  init_struct_header(&header);
  header.id = RESERVAR_MEMORIA;
  header.size = sizeof(int) * 2;

  void* sendBuffer = malloc(sizeof(struct s_header) + 2*sizeof(int));

  serialize_from_int(sendBuffer, header.id, &offset);
  serialize_from_int(sendBuffer, header.size, &offset);
  serialize_from_int(sendBuffer, pid, &offset);
  serialize_from_int(sendBuffer, cant_pag, &offset);

  //envío
  log_info(LOG, "Se pide reserva de memoria a SWAP. Se bloquea");
  pthread_mutex_lock( &mutex_swap );
	  int status = send(socket, sendBuffer, sizeof(struct s_header) + header.size, 0);
	  if (status == -1){
		  ret =  ERROR;
	  }else{
		  if (status > 0){
			  struct s_header respHeader;
			  init_struct_header(&respHeader);
			  status = recv(socket, &respHeader, sizeof(struct s_header), MSG_WAITALL);
			  if (status == ERROR){
				  ret = ERROR;
			  }else{
				  ret = respHeader.id;
			  }
		  }else{
			  ret = ERROR;
		  }
	  }

  pthread_mutex_unlock( &mutex_swap );
  log_info(LOG, "Se pide reserva de memoria a SWAP. Se desbloquea");

  free(sendBuffer);
  return ret;

}

int send_fin_to_SWAP(int socket, int pid){

	int ret = 0;
	int offset = 0;
	int status = 0;

	struct s_header receiver;
	struct s_header senderHeader;

	senderHeader.id = FINALIZAR_PROGRAMA;
	senderHeader.size = sizeof(int);

	void* sendBuffer = malloc(sizeof(struct s_header) + sizeof(int));
	serialize_from_int(sendBuffer, senderHeader.id, &offset);
	serialize_from_int(sendBuffer, senderHeader.size, &offset);
	serialize_from_int(sendBuffer, pid, &offset);

	log_info(LOG, "Se pide finalizar programa. Se bloquea llamadas a SWAP");
	pthread_mutex_lock( &mutex_swap );

	status = send(socket, sendBuffer, sizeof(struct s_header) + sizeof(int), 0);
	if (status == ERROR) ret = ERROR;
	
	else{

		status = recv(socket, &receiver, sizeof(struct s_header), 0);
		if(status == -1){
			log_error(LOG, "Error: no se recibio el mensaje");
			ret = ERROR;
		}else{
			if(status ==  0) {
				log_error(LOG, "Error: se cayo el socket");
				ret = ERROR;
			}else{
				if (receiver.id == OK){
					log_info(LOG, string_from_format("Se elimino correctamente %d de swap", pid));
					ret = OK;

				}else{
					log_error(LOG, "Fallo en swap eliminando programa %d", pid);
					ret = FALLO;

				}
			}

		}
	}
	pthread_mutex_unlock( &mutex_swap );
	log_info(LOG, "Se pide finalizar programa. Se desbloquea llamadas a SWAP");

	free(sendBuffer);


	return ret;
}

int send_page_size_to_socket(int socket, int tam_pag){

	int ret;
	int offset = 0;
	int status = 0;

	struct s_header senderHeader;

	senderHeader.id = TAMANIO_PAGINA;
	senderHeader.size = sizeof(int);

	void* sendBuffer = malloc(sizeof(struct s_header) + sizeof(int));
	serialize_from_int(sendBuffer, senderHeader.id, &offset);
	serialize_from_int(sendBuffer, senderHeader.size, &offset);
	serialize_from_int(sendBuffer, tam_pag, &offset);

	status = send(socket, sendBuffer, sizeof(struct s_header) + sizeof(int), 0);
	if (status == ERROR){
		ret = ERROR;
	}else{
		ret = OK;
	}

	free(sendBuffer);
	return ret;
}

void build_pages_for_process(int pid, int cant_pag){

	int i;
	s_process* proceso = malloc(sizeof(s_process));

	proceso->pid = pid;
	proceso->pages = list_create();

	for (i=0; i < cant_pag; i++){

		s_page_table* page = malloc(sizeof(s_page_table));

		page->marco = 0;
		page->pagina = i;
		page->bituso = 0;
		page->memory_flag = 0;
		page->modif_flag = 0;
		page->ref_time = 0;

		list_add(proceso->pages, page);
	}

	pthread_mutex_lock( &mutex_page_table );
		list_add(process_tpage, proceso);
	pthread_mutex_unlock( &mutex_page_table );
}

int split_and_send_code(int socket, int pid, int tam_pag, char* codigo){
	//Se envía: INICIAR_PROGRAMA - SIZE: sizeof pid + sizeof pagina + tam_pag - PAYLOAD: pid|nropag|codigo
	int ret = OK;
	int index = 0;
	int flag = 0;   //Flag para ver cuántas páginas mandar.
	struct s_header header;
	header.id = ALMACENAR_BYTES;//INICIAR_PROGRAMA;

	void send_pages(void* parametro){
		if (flag == 0){
			s_page_table* page = (s_page_table*) parametro;

			char* code = malloc(tam_pag);//string_new();
			char* rest = string_repeat('\0',tam_pag);
			memcpy(code, rest, tam_pag);
			int offset = 0;

			if(string_length(string_substring_from(codigo, index)) >= tam_pag){
				memcpy(code, string_substring(codigo, index, tam_pag), tam_pag);
				index += tam_pag;
			}else{
				memcpy(code, string_substring_from(codigo, index), string_length(string_substring_from(codigo, index)));
				flag = 1;
			}

			header.size = sizeof(pid) + sizeof(page->pagina) + tam_pag;

			void* sendBuffer = malloc(sizeof(header) + header.size);

			serialize_from_int(sendBuffer, header.id, &offset);
			serialize_from_int(sendBuffer, header.size, &offset);
			serialize_from_int(sendBuffer, pid, &offset);
			serialize_from_int(sendBuffer, page->pagina, &offset);
			memcpy(sendBuffer + offset, code, tam_pag);

			//log_info(LOG, "Ver split: %s", string_substring(code, 0, tam_pag ));

			int status = send(socket, sendBuffer, sizeof(struct s_header) + header.size, 0);
			if (status == ERROR){
				ret = ERROR;
				flag = 1; //Dejo de procesar si me dió error al enviar alguna página
				log_error(LOG, string_from_format("Error al enviar página %d del proceso %d", page->pagina, pid));
			}else{
				struct s_header respHeader;
				init_struct_header(&respHeader);
				status = recv(socket, &respHeader, sizeof(struct s_header), MSG_WAITALL);
				if (status == ERROR){
					log_error(LOG, string_from_format("Error al recibir confirmación al enviar página %d del proceso %d", page->pagina, pid));
					ret = ERROR;
					flag = 1;
				}else{
					if (respHeader.id == OK){
						ret = OK;
						log_info(LOG, string_from_format("Se envió la página %d del proceso %d: %s", page->pagina, pid, string_substring(code, 0, tam_pag)));
					}else{
						log_error(LOG, string_from_format("Error al enviar página %d del proceso %d", page->pagina, pid));
						ret = FALLO;
						flag = 1;
					}
				}
			}

			free(code);
			free(sendBuffer);
		}
	}

	bool search_process(void* parametro){
		s_process* proceso = (s_process*) parametro;

		return proceso->pid == pid;
	}

	s_process* proceso = list_find(process_tpage, search_process);

	if(proceso != NULL){
		pthread_mutex_lock(&mutex_swap);
			list_iterate(proceso->pages, send_pages);
		pthread_mutex_unlock(&mutex_swap);
	}

	return ret;
}

void add_cpu_to_list(int socket){

	s_cpu* entry = malloc(sizeof(s_cpu));

	entry->socket = socket;
	entry->pid    = 0;

	pthread_mutex_lock( &mutex_cpu );
		list_add(list_cpu, entry);
	pthread_mutex_unlock( &mutex_cpu );
}

int change_cpu_program(int socket, int pid){

	int ret;

	s_cpu* cpu = search_cpu(socket);

	if(cpu != NULL){
		ret = cpu->pid;

		pthread_mutex_lock( &mutex_cpu );
			cpu->pid = pid;
		pthread_mutex_unlock( &mutex_cpu );

		//ret = OK;
	}else{
		ret = ERROR;
	}

	return ret;
}

s_cpu* search_cpu(int socket){

	bool find_cpu(void* parametro){
		s_cpu* entry = (s_cpu*) parametro;
		return entry->socket == socket;
	}

	s_cpu* cpu = list_find(list_cpu, find_cpu);

	return cpu;
}

void destroy_cpu(int socket){

	bool find_cpu(void* parametro){
			s_cpu* entry = (s_cpu*) parametro;
			return entry->socket == socket;
	}

	pthread_mutex_lock( &mutex_cpu );
		list_remove_and_destroy_by_condition(list_cpu, find_cpu, free);
	pthread_mutex_unlock( &mutex_cpu );

}

void dump_struct(int pid){
	char* file_path = string_duplicate(config.RUTA_DUMP);

	if(pid != 0){
		string_append(&file_path, string_from_format("Dump-Struct-PID%d.txt", pid));
	}else{
		string_append(&file_path, "Dump-Struct.txt");
	}

	FILE* file = fopen(file_path, "w");

	int flag = 0;
	int pid_global;

	void print_page(void* parametro){
		s_page_table* page = (s_page_table*) parametro;
		char* buffer = string_from_format("   %d    |    %d    |     %d      |     %d      |   %d   |   %d   \n",
										  pid_global, //Ahora veo
										  page->marco,
										  page->pagina,
										  page->bituso,
										  page->modif_flag,
										  page->memory_flag
										  );
		txt_write_in_file(file, buffer);
		txt_write_in_stdout(buffer);
	}

	void search_process(void* parametro){
		s_process* proceso = (s_process*) parametro;
		if (pid != 0 && proceso->pid == pid){
			if (flag == 0){
              pid_global = pid;
			  list_iterate(proceso->pages, print_page);
			  flag = 1;
			}
		}else{
			pid_global = proceso->pid;
			list_iterate(proceso->pages, print_page);
		}
	}

	if(file == NULL){
		log_error(LOG, "Error al abrir el archivo en el path %s", file_path);
	}else{
		char* header = "  PID   |   MARCO   |   PAGINA   |   BITUSO   |   M   |   P   \n";

		txt_write_in_file(file, header);
		txt_write_in_stdout(header);

		pthread_mutex_lock(&mutex_page_table);
			list_iterate(process_tpage, search_process);
		pthread_mutex_unlock(&mutex_page_table);
	}
}

void dump_content_all(){

	char* file_path = string_duplicate(config.RUTA_DUMP);
	string_append(&file_path, "Dump-Content.txt");
	FILE* file = fopen(file_path, "w");
	if(file == NULL){
			log_error(LOG, "Error al abrir el archivo en el path %s", file_path);
	}else{
		int i;

		pthread_mutex_lock(&mutex_memory);

		for(i=0;i<config.MARCOS;i++){
			int offset = ( config.MARCO_SIZE * i );
			char* content = malloc(config.MARCO_SIZE);

			int pid = search_pid_present_in_frame(i);
			memcpy(content, memory+offset, config.MARCO_SIZE);

			char* buffer = string_from_format("Marco %d | (PID:%d)\n %s \n",
											  i,
											  pid,
											  string_substring(content, 0, config.MARCO_SIZE));

			txt_write_in_file(file, buffer);
			txt_write_in_stdout(buffer);

			free(content);
		}

		pthread_mutex_unlock(&mutex_memory);
	}
}

void dump_content(int pid){
	char* file_path = string_duplicate(config.RUTA_DUMP);

	if(pid != 0){
		string_append(&file_path, string_from_format("Dump-Content-PID%d.txt", pid));
	}else{
		string_append(&file_path, "Dump-Content.txt");
	}

	FILE* file = fopen(file_path, "w");

	void print(void* parametro){
		s_page_table* page = (s_page_table*) parametro;

		int offset = ( config.MARCO_SIZE * page->marco );
		char* content = malloc(config.MARCO_SIZE);

		memcpy(content, memory+offset, config.MARCO_SIZE);

		char* buffer = string_from_format("Marco %d\n %s \n",
										  page->marco,
										  string_substring(content, 0, config.MARCO_SIZE));

		txt_write_in_file(file, buffer);
		txt_write_in_stdout(buffer);

		free(content);

	}

	if(file == NULL){
		log_error(LOG, "Error al abrir el archivo en el path %s", file_path);
	}else{
		pthread_mutex_lock(&mutex_page_table);

		t_list* presentPages = get_present_pages_from_pid(pid);

		if (presentPages != NULL){
			pthread_mutex_lock(&mutex_memory);

			list_iterate(presentPages, print);

			pthread_mutex_unlock(&mutex_memory);
		}else{
			txt_write_in_file(file, "No hay páginas en memoria para el proceso seleccionado\n");
			txt_write_in_stdout("No hay páginas en memoria para el proceso seleccionado\n");
		}

		pthread_mutex_unlock(&mutex_page_table);
	}
}

int check_request_is_valid(struct s_req_bytes* data, int pid){

	int ret;

	bool search_pid(void* parametro){
		s_process* proc = (s_process*) parametro;
		return proc->pid == pid;
	}

	bool check_page(void* parametro){
		s_page_table* page = (s_page_table*) parametro;
		return page->pagina == data->pagina;
	}

	//Chequeo offset, por las dudas.
	if((data->offset + data->tamanio) > config.MARCO_SIZE){
		log_error(LOG, "La solicitud es inválida");
		return FALLO;
	}

	s_process* proceso = list_find(process_tpage, search_pid);

	if(proceso != NULL){
		bool flag = list_any_satisfy(proceso->pages, check_page);
		if (flag == true){
			ret = OK;
			log_info(LOG, "La solicitud es válida");
		}else{
			ret = FALLO;
			log_error(LOG, "La solicitud es inválida");
		}
	}else{
		ret = FALLO;
		log_error(LOG, "La solicitud es inválida");
		log_error(LOG, "El proceso %d no se encuentra iniciado o ya fue finalizado", pid);
	}

	return ret;
}

void socket_error_response(int socketResponse, int id){

	struct s_header senderHeader;

	senderHeader.id = id;
	senderHeader.size = 0;

	void* sendBuffer = malloc(sizeof(struct s_header));
	serialize_from_header(senderHeader, sendBuffer);

	send(socketResponse, sendBuffer, sizeof(struct s_header), 0);

	free(sendBuffer);
}
