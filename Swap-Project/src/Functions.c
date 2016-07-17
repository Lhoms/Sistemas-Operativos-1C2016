/*
 * Functions.c
 *
 *  Created on: 24/4/2016
 *      Author: Lajew
 *      Descrp: Funciones de SWAP
 */

#include "Swap.h"

s_conf get_config(char* path, t_log* LOG){

	s_conf config;
	t_config* config_file;
	int cantConfig = 0;
	char* logMessage = string_new();

	//CONFIG DESDE PATH
	config_file = config_create(path);
	cantConfig = config_keys_amount(config_file);

	//VERIFICA CONFIG
	if (cantConfig == 0)
	{
		log_error(LOG, "Archivo de configuración vacío");
	}

	//SETEO DE PROPIEDADES
	config.puerto_escucha = string_duplicate(config_get_string_value(config_file, "PUERTO_ESCUCHA"));
	if (!string_is_empty(config.puerto_escucha))
	{
		logMessage = string_from_format("PUERTO ESCUCHA: %s", config.puerto_escucha);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "No se encontró puerto escucha");
	}

	config.nombre_swap = string_duplicate(config_get_string_value(config_file, "NOMBRE_SWAP"));
	if (!string_is_empty(config.nombre_swap))
	{
		logMessage = string_from_format("NOMBRE SWAP: %s", config.nombre_swap);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "No se encontró nombre SWAP");
	}

	config.cant_pag = string_duplicate(config_get_string_value(config_file, "CANTIDAD_PAGINAS"));
	if (!string_is_empty(config.cant_pag))
	{
		logMessage = string_from_format("CANTIDAD PAGINAS: %s", config.cant_pag);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "No se encontró cantidad de paginas");
	}

	config.tam_pag = string_duplicate(config_get_string_value(config_file, "TAMANIO_PAGINA"));
	if (!string_is_empty(config.tam_pag))
	{
		logMessage = string_from_format("TAMANIO PAGINAS: %s", config.tam_pag);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "No se encontró tamaño paginas");
	}

	config.retardo_compactacion = string_duplicate(config_get_string_value(config_file, "RETARDO_COMPACTACION"));
	if (!string_is_empty(config.retardo_compactacion))
	{
		logMessage = string_from_format("RETARDO COMPACTACION: %s", config.retardo_compactacion);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "No se encontró retardo de compactación");
	}

	config.retardo_acceso = string_duplicate(config_get_string_value(config_file, "RETARDO_ACCESO"));
	if (!string_is_empty(config.retardo_acceso))
	{
		logMessage = string_from_format("RETARDO ACCESO: %s", config.retardo_acceso);
		log_info(LOG, logMessage);
	}
	else
	{
		log_error(LOG, "No se encontró retardo de compactación");
	}

	config_destroy(config_file);
	return config;
}

/*
char* recibirArchivo(int SocketFD, t_log* LOG){
	//INCIALIZACION VARIABLES
	char buffer[PACKAGESIZE];
	int recibido;

	//RECIBO PAQUETE
	while((recibido = recv(SocketFD, (void*)buffer, (sizeof buffer)+1, 0)) != 0){

			log_info(LOG, "Paquete recibido. Contenido:");
			buffer[strlen(buffer)]='\0';
			log_info(LOG, buffer);

		}
	return buffer;
}*/



int StringToNum(char palabra[]) {
  int caracter, signo, offset, num;

  if (palabra[0] == '-') {  // Consideracion para numeros negativos
    signo = -1;
  }

  if (signo == -1) { // El numero empieza en la siguiente posicion
    offset = 1;
  }
  else {
    offset = 0;
  }

  num = 0;

  for (caracter = offset; palabra[caracter] != '\0'; caracter++) {
    num = num * 10 + palabra[caracter] - '0'; //'0' vale 48, '1' 49 y asi sucesivamente. Resto '0' para quitar ese offset
  }

  if (signo == -1) {
    num = -num;
  }

  return num;
}

/*
 * Funcion que agrega los datos de un proceso a la lista de control.
 * Devuelve true en caso de poder agregarlo, sino devuelve false
 */
int agregar_elemento_lista (nodo_lista **lista, pid_t nuevo_pid, int nuevo_num_pag, int nuevo_pos_swap)
{
	nodo_lista *nodo_nuevo = NULL;	//nodo a agregar a la lista
	nodo_lista *nodo_aux = NULL;	//nodo para recorrer la lista

	nodo_aux = *lista;
	nodo_nuevo = (nodo_lista*) malloc (sizeof(nodo_lista));
	nodo_nuevo->siguiente = nodo_nuevo->anterior = NULL;

	if(nodo_nuevo == NULL)
	{
		//error al asignar memoria
		printf("Error al asignar memoria al agregar elemento a lista de control\n");
		return (false);
	}

	nodo_nuevo->pid = nuevo_pid;
	nodo_nuevo->num_pag = nuevo_num_pag;
	nodo_nuevo->pos_en_swap = nuevo_pos_swap;

	if(*lista == NULL)
	{
		*lista = nodo_nuevo;	//caso lista vacia, entonces es el primer nodo
		//printf("primer programa agregado, con pid %d\n",nuevo_pid);
		return (true);
	}
	else	//la lista ya tiene elementos, agrego el nuevo elemento al final de la lista
	{
		while(nodo_aux->siguiente != NULL) //while hasta llegar al final
		{
			nodo_aux = nodo_aux->siguiente; // voy apuntando a los siguientes elementos
		}
		//se llego al final de la lista

		nodo_nuevo->siguiente = nodo_aux->siguiente;//apunta a null porque ahora es el ultimo
		nodo_nuevo->anterior = nodo_aux; //el ultimo viejo ahora es el anterior al nuevo nodo
		nodo_aux->siguiente = nodo_nuevo; //el ultimo viejo debe apuntar al nuevo ultimo

		//printf("Se agrego el proceso con pid %d\n",nuevo_pid);
		return(true);
	}
}

	/*
	 * Funcion que quita un proceso de la lista de control.
	 * Devuelve la cantidad de elementos quitados
	 */
int quitar_elemento_lista (nodo_lista **lista, pid_t pid_quitar, t_log* LOG)
{
		int cant_quitada = 0;
		nodo_lista *nodo_borrar = NULL;
		nodo_lista *nodo_aux = NULL;
		if(*lista == NULL)
		{
			//La lista ya esta vacia, no se puede quitar elementos
			printf ("No se quito el proceso de pid %d de la lista, porque la lista ya estaba vacia\n",pid_quitar);
			return cant_quitada;
		}
		nodo_aux = *lista;
		while((nodo_aux != NULL)&&(nodo_aux->pid != pid_quitar))
		{
			nodo_aux = nodo_aux->siguiente; //se recorre la lista hasta encontrar el elemento o llegar al final
		}
		if(nodo_aux == NULL)
		{
			printf("El proceso de pid %d no estaba en la lista para quitarlo\n",pid_quitar);
			return cant_quitada;
		}

		if((nodo_aux->anterior == NULL)&&(nodo_aux->siguiente == NULL))
		{
			//se quita el unico elemento de la lista
			free(nodo_aux);//libero la memoria
			printf("Se quito el proceso de pid %d\n",pid_quitar);
			*lista=NULL; //la lista ahora esta vacia
			printf("Lista de control vacia");
			cant_quitada++;
			return cant_quitada;

		}
		if(nodo_aux->anterior == NULL)
		{
			//el elemento a quitar es el primer elemento de la lista pero no el unico
			//quito tambien los siguientes elementos consecutivos que tengan el pid a quitar
			do
			{
			nodo_borrar = nodo_aux;	//apunto al elemento a borrar
			
			if(nodo_aux->siguiente == NULL) //Ya borro otras paginas y ahora es el ultimo elemento de la lista
			{
				free(nodo_borrar);
				cant_quitada++;
				*lista=NULL; //la lista ahora esta vacia
				printf("Lista de control vacia\n");
				break;
			}
			
			(nodo_aux->siguiente)->anterior = NULL; //campo anterior del siguiente elemento va a apuntar a null porque va a ser el primer elemento
			nodo_aux = nodo_aux->siguiente;
			*lista = nodo_aux; //el siguiente elemento de la lista ahora es el primer elemento
			free (nodo_borrar);	//libero la memoria del nodo a borrar
			cant_quitada++;
			 }while(nodo_aux->pid == pid_quitar);
			printf("Se quito el proceso de pid %d\n",pid_quitar);

			return cant_quitada;
		}
		if(nodo_aux->siguiente == NULL)
		{
			//el elemento a quitar es el ultimo pero no el unico
			//como es el ultimo, no hay mas elementos con el mismo pid
			nodo_borrar = nodo_aux; //apunto al elemento a eliminar
			nodo_aux->anterior->siguiente = NULL; //el anterior elemento ahora va a ser el ultimo
			nodo_aux = nodo_aux->anterior;
			free(nodo_borrar);
			printf("Se quito el proceso de pid %d\n",pid_quitar);
			cant_quitada++;
			return cant_quitada;
		}
		//Si llego aca, entonces el elemento a borrar esta entre dos nodos existentes
		//Debo resolver sus punteros
		//hay que quitar tambien los siguientes elementos si tienen el mismo pid
		do
		{
		nodo_borrar = nodo_aux; //apunto al elemento a eliminar
		
		if(nodo_aux->siguiente == NULL) //Ya borro otras paginas y ahora es el ultimo elemento de la lista
		{
			nodo_aux->anterior->siguiente = nodo_aux->siguiente;
			free(nodo_borrar);
			cant_quitada++;
			break;
		}
		
		nodo_aux->anterior->siguiente = nodo_aux->siguiente; //el anterior elemento apunta al siguiente
		nodo_aux->siguiente->anterior = nodo_aux->anterior; //el siguiente elemento apunta al anterior
		nodo_aux = nodo_aux->siguiente; //apunto al siguiente elemento por si hay que seguir borrando
		free(nodo_borrar);
		cant_quitada++;
		}while(nodo_aux->pid == pid_quitar);
		printf("Se quito el proceso de pid %d\n",pid_quitar);
		return cant_quitada;


}

void mostrar_lista(nodo_lista **lista)
{
	int i;
	nodo_lista *nodo_aux = NULL;
	if(*lista == NULL)
	{
		printf("La lista esta vacia\n");
	}
	nodo_aux = *lista;
	printf("Datos de los procesos en la estructura de control\n");
	for(i=1;nodo_aux != NULL; i++, nodo_aux=nodo_aux->siguiente)
	{
		printf("Pid: %d, Num. Pagina: %d, Pos. Swap: %d\n",nodo_aux->pid,nodo_aux->num_pag, nodo_aux->pos_en_swap);
	}
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
/*
int recv_handshake(int socket, t_log* LOG){
	int ret;

	struct s_header sendHandshake;
	struct s_header recvHandshake;

	sendHandshake.id = SALUDO_SWAP;
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

}*/

void socket_response(int socket,int id){

	struct s_header senderHeader;

	senderHeader.id = id;
	senderHeader.size = 0;

	void* sendBuffer = malloc(sizeof(struct s_header));
	serialize_from_header(senderHeader, sendBuffer);

	send(socket, sendBuffer, sizeof(struct s_header), 0);

	free(sendBuffer);
}
/*
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
}*/

/*
 * Funcion que mira si hay suficientes paginas.
 * Si hay espacio suficiente retorna el indice a la primer pagina contigua libre.
 * En caso de que no haya espacio suficiente libre, retorna -2.
 * Si hay espacio suficiente pero esta fragmentado, retorna -1
 */

int tiene_suficientes_paginas(t_bitarray* bitmap,int cant_pag_pedidas)
{
    /*
     * 	typedef struct {
		char *bitarray;
		size_t size;
	} t_bitarray;
     * bool 		 bitarray_test_bit(t_bitarray*, off_t bit_index);
     * void		 bitarray_set_bit(t_bitarray*, off_t bit_index);
     * void		 bitarray_clean_bit(t_bitarray*, off_t bit_index);
     * size_t		 bitarray_get_max_bit(t_bitarray*);
     * void 		 bitarray_destroy(t_bitarray*);
     */

	//size_t cantidad_paginas = bitarray_get_max_bit(bitmap);

	int cantidad_paginas = StringToNum(config.cant_pag);

	int indice, paginas_contiguas, total_paginas_libres;

	for (indice=paginas_contiguas=total_paginas_libres=0;indice<cantidad_paginas;indice++)
	{
		if(bitarray_test_bit(bitmap, (off_t) indice)==0)
		{
			total_paginas_libres++;
			paginas_contiguas++;

			if(paginas_contiguas==cant_pag_pedidas)
			{
				//tiene suficientes paginas y no necesita compactacion

				//retorna el indice a la primer pagina de las paginas contiguas libres
				log_info(LOG, string_from_format("Se encontraron %d paginas contiguas desde la posicion de swap %d",cant_pag_pedidas,(indice+1-cant_pag_pedidas)));
				return (indice+1-cant_pag_pedidas);
			}
		}
		else
		{
			paginas_contiguas=0;
		}


	}

	//si llego aca, entonces no hay la cantidad de paginas contiguas necesarias

	if(total_paginas_libres>=cant_pag_pedidas)
	{
		return NECESITA_COMPACTACION;	//tiene las paginas pero necesita hacer compactacion
	}
	else
	{
		return NO_TIENE_SUFICIENTES_PAGINAS;	//no tiene suficientes paginas libres
	}



}

/*
 * Funcion que marca los espacios de swap como ocupados y asocia ese espacio en la estructura de control
 */

int reservar_memoria(t_bitarray* bitmap,nodo_lista **estructura_control,pid_t pid,int primer_espacio_libre,int cant_pag_pedidas)
{
	//primero marco las espacios como ocupados en el bitmap


	int indice_espacios = primer_espacio_libre;
	int result;
	for(;indice_espacios<primer_espacio_libre+cant_pag_pedidas;indice_espacios++)
	{
		bitarray_set_bit(bitmap, (off_t) indice_espacios);

	}

	log_info(LOG, string_from_format("Se marcaron como ocupados %d espacios de swap a partir de la posicion de swap %d para el proceso de pid %d",cant_pag_pedidas,primer_espacio_libre,pid));



	/*
	 * Ahora agrego las entradas a la estructura de control.
	 * La numeracion de paginas es local a cada pid.
	 * Asi que siempre la primer pagina es la 0.
	 */

	int nuevo_num_pag = 0;
	indice_espacios = primer_espacio_libre;

	//ver esto no es necesario el for

	for(;nuevo_num_pag<cant_pag_pedidas;nuevo_num_pag++,indice_espacios++)
	{
		result =  agregar_elemento_lista (estructura_control, pid, nuevo_num_pag, indice_espacios);

		if(result == false)
		{
			log_error(LOG, string_from_format("Error al agregar a la lista la pagina %d del pid %d", nuevo_num_pag, pid));
			return ERROR;
		}
	}

	printf("Se agregaron %d paginas para proceso con pid %d a partir de la posicion de swap %d\n",cant_pag_pedidas,pid,primer_espacio_libre);
	log_info(LOG, string_from_format("Se agregaron %d paginas para proceso con pid %d a partir de la posicion de swap %d",cant_pag_pedidas,pid,primer_espacio_libre));


	return (true);



}

int finalizar_programa(t_bitarray* bitmap,nodo_lista **estructura_control,pid_t pid_quitar)
{
	int primer_espacio,cantidad_quitar;
	primer_espacio = espacio_pagina_pedida(estructura_control,pid_quitar,0);//espacio_pagina0(estructura_control,pid_quitar);
	if(primer_espacio == ERROR)
	{
		//el pid no se encuentra en la estructura de control de control
		log_error(LOG, string_from_format("No se encontro el proceso de pid %d en la lista", pid_quitar));
		return ERROR;
	}

	//quito los elementos de la lista


	cantidad_quitar = quitar_elemento_lista (estructura_control, pid_quitar, LOG);

	//ahora marco los espacios como libres


	int indice_espacios = primer_espacio;

	for(;indice_espacios<primer_espacio+cantidad_quitar;indice_espacios++)
	{
		bitarray_clean_bit(bitmap, (off_t) indice_espacios);

	}

	log_info(LOG, string_from_format("Se han quitado las %d paginas del proceso de pid %d", cantidad_quitar, pid_quitar));

	return (true);


}

int espacio_pagina0(nodo_lista **lista,pid_t pid)
{

	//nodo_lista *nodo_borrar = NULL;
	nodo_lista *nodo_aux = NULL;
	if(*lista == NULL)
	{
		//La lista esta vacia
		//printf ("No se encontro el proceso de pid %d en la lista porque estaba vacia\n",pid);
		return ERROR;
	}
	nodo_aux = *lista;
	while((nodo_aux != NULL)&&(nodo_aux->pid != pid))
	{
		nodo_aux = nodo_aux->siguiente; //se recorre la lista hasta encontrar el elemento o llegar al final
	}
	if(nodo_aux == NULL)
	{
		//printf("El proceso de pid %d no estaba en la lista\n",pid);
		return ERROR;
	}
	//se encontro el primer o unico elemento que tiene ese pid
	//retorno la posicion de swap
	return nodo_aux->pos_en_swap;

}

int espacio_pagina_pedida(nodo_lista **lista,pid_t pid,int num_pag_pedida)
{

	//nodo_lista *nodo_borrar = NULL;
	nodo_lista *nodo_aux = NULL;
	if(*lista == NULL)
	{
		//La lista esta vacia
		printf ("No se encontro el proceso de pid %d en la lista porque estaba vacia\n",pid);
		log_error(LOG, string_from_format("No se encontro el proceso de pid %d en la lista porque estaba vacia", pid));
		return ERROR;
	}
	nodo_aux = *lista;
	while((nodo_aux != NULL)&&((nodo_aux->pid != pid)||(nodo_aux->num_pag != num_pag_pedida)))
	{
		nodo_aux = nodo_aux->siguiente; //se recorre la lista hasta encontrar el elemento o llegar al final
	}
	if(nodo_aux == NULL)
	{
		printf("No se encontro la pagina %d del proceso de pid %d porque no estaba en la lista\n",num_pag_pedida,pid);
		log_error(LOG, string_from_format("No se encontro la pagina %d del proceso de pid %d porque no estaba en la lista",num_pag_pedida, pid));
		return ERROR;
	}
	//se encontro el primer o unico elemento que tiene ese pid
	//retorno la posicion de swap
	return nodo_aux->pos_en_swap;

}

int almacenar_bytes(char *puntero_archivo,nodo_lista **estructura_control,pid_t pid,int num_pagina,char* recvMessage)
{
	//primero hay que buscar donde almacenar
	int offset,espacio_swap;//primer_espacio;
	int tam_pag = StringToNum(config.tam_pag);
	espacio_swap = espacio_pagina_pedida(estructura_control,pid,num_pagina);
	//primer_espacio = espacio_pagina_pedida(estructura_control,pid,num_pagina);//espacio_pagina0(estructura_control,pid);
	if(espacio_swap == ERROR)
	{
		printf("No se almacenan bytes por pid o numero de pagina incorrecto\n");
		log_error(LOG, string_from_format("No se encontro pagina %d del pid %d por pid incorrecto o numero de pagina inexistente",num_pagina,pid));
		log_info(LOG,"No se almacenan bytes");
		return ERROR; //no se encuentra ese pid en la lista
	}

	offset = sizeof(char)*tam_pag*espacio_swap;

	//almaceno los bytes
	memcpy(puntero_archivo+offset, recvMessage, tam_pag);
	log_info(LOG, string_from_format("Se guardo en el archivo binario el contenido de la pagina %d del proceso de pid %d",num_pagina,pid));
	return (true);




}

int enviar_pagina(char *puntero_archivo,int socketCliente,nodo_lista **estructura_control,pid_t pid,int num_pagina)
{
	//primero hay que buscar la pagina
	int offset,espacio_swap;//primer_espacio;
	int tam_pag = StringToNum(config.tam_pag);
	struct s_header header;
	char* sendMessage = malloc(sizeof(struct s_header)+tam_pag);



	header.id = SOLICITAR_BYTES;//INICIAR_PROGRAMA;
	header.size = tam_pag;
	serialize_from_header(header, sendMessage);
	espacio_swap = espacio_pagina_pedida(estructura_control,pid,num_pagina);
	//primer_espacio = espacio_pagina0(estructura_control,pid);
	if(espacio_swap == ERROR)
	{
		printf("No se encontro pagina solicitada por pid incorrecto o numero de pagina inexistente\n");
		log_error(LOG, string_from_format("No se encontro pagina %d del pid %d por pid incorrecto o numero de pagina inexistente",num_pagina,pid));
		free(sendMessage);
		return ERROR; //no se encuentra ese pid en la lista
	}

	//offset = sizeof(char)*tam_pag*(primer_espacio+num_pagina);
	offset = sizeof(char)*tam_pag*espacio_swap;

	//almaceno los bytes en la posicion despues del header
	memcpy(sendMessage+sizeof(struct s_header),puntero_archivo+offset, tam_pag);

	int status = send(socketCliente, sendMessage, sizeof(struct s_header) + header.size, 0);
	log_info(LOG, "Se envia contenido de pagina:");
	log_info(LOG, string_substring(puntero_archivo+offset, 0 , tam_pag));
	if (status == ERROR)
	{
		printf("Error en send al enviar pagina\n");
		log_error(LOG,"Error en send al enviar pagina");
		free(sendMessage);
		return ERROR;
	}

	free(sendMessage);


	return (true);

}

int compactacion(t_bitarray* bitmap,nodo_lista **estructura_control,char *puntero_binario)
{
	int espacio_libre, espacio_mover;

	int retardo = StringToNum(config.retardo_compactacion);

	espacio_mover = 0;

	printf("Compactando...\n");

	//Retrazo de tiempo segun archivo de configuracion

	usleep(retardo*1000);

	log_info(LOG, "Se inicia proceso de compactacion");

	while(espacio_mover != -1)
	{
		espacio_libre = primer_espacio_libre(bitmap);

		if(espacio_libre == -1)
		{
			break; //se llego al final y no se encontro espacio libre
		}


		espacio_mover = primer_espacio_ocupado_fragmentado(bitmap);


		if(espacio_mover != -1)
		{

			compactar_un_proceso(bitmap,estructura_control,puntero_binario,espacio_libre,espacio_mover);

		}
	}

	espacio_libre = primer_espacio_libre(bitmap);

	log_info(LOG, "Compactacion finalizada");

	return espacio_libre;



}

int primer_espacio_libre(t_bitarray* bitmap)
{

	int indice_espacios = 0;

	int maximo_espacios = StringToNum(config.cant_pag);
	//int maximo_espacios = bitarray_get_max_bit(bitmap);

	for(;indice_espacios<maximo_espacios;indice_espacios++)
	{
		if(bitarray_test_bit(bitmap, (off_t) indice_espacios) == ESPACIO_LIBRE)
		{
			return indice_espacios;
		}

	}

	return -1;



}

int primer_espacio_ocupado_fragmentado(t_bitarray* bitmap)
{

	int indice_espacios = 0;

	int maximo_espacios = StringToNum(config.cant_pag);
	//int maximo_espacios = bitarray_get_max_bit(bitmap);

	for(;indice_espacios<maximo_espacios;indice_espacios++)
	{
		if(bitarray_test_bit(bitmap, (off_t) indice_espacios) == ESPACIO_LIBRE)
		{
			break;
		}

	}

	if(indice_espacios>=maximo_espacios)
	{
		return -1;//se llego al final
	}

	indice_espacios++;//incremento y busco el siguiente espacio ocupado que es el fragmentado

	for(;indice_espacios<maximo_espacios;indice_espacios++)
	{
		if(bitarray_test_bit(bitmap, (off_t) indice_espacios) == ESPACIO_OCUPADO)
		{
			return indice_espacios;
		}

	}

	return -1;//se llego al final, ya no hay espacio ocupado fragmentado

}

/*
 * Funcion que mueve las paginas en swap de solo un proceso.
 * Realiza las acciones en el siguiente orden:
 * Mueve el contenido de los espacios de swap a la posicion deseada
 * Marca como libres los espacios viejos.
 * Marca como ocupados los nuevos espacios (necesita este orden por si necesita marcar un espacio viejo como ocupado).
 * Cambia los datos de posicion de swap por los nuevos en la estructura de control
 */

int compactar_un_proceso(t_bitarray* bitmap,nodo_lista **estructura_control,char *puntero_binario,int espacio_libre,int espacio_mover)
{
	pid_t pid = buscar_pid_segun_espacio(estructura_control,espacio_mover);

	int cant_paginas_mover = cant_paginas_segun_pid(estructura_control,pid);

	log_info(LOG, string_from_format("Compactando proceso de pid %d con %d paginas", pid, cant_paginas_mover));

	mover_paginas_proceso(puntero_binario,espacio_libre,espacio_mover,cant_paginas_mover);

	log_info(LOG, string_from_format("Se termino de mover el contenido de %d paginas del proceso de pid %d", cant_paginas_mover, pid));

	marcar_como_libre(bitmap,espacio_mover,cant_paginas_mover);	//se marca como libres los espacios de las paginas recien movidas

	log_info(LOG, "Se marcaron como libres los espacios que ocupaban las paginas");

	marcar_como_ocupado(bitmap,espacio_libre,cant_paginas_mover);

	log_info(LOG, "Se marcaron como ocupados los espacios que ahora ocupan las paginas movidas");

	editar_inicio_swap(estructura_control,pid,espacio_libre);

	log_info(LOG, "Se actualizo la estructura de control con la nueva posicion en swap que tienen las paginas");

	return (true);

}

pid_t buscar_pid_segun_espacio(nodo_lista **lista,int espacio_mover)
{
	nodo_lista *nodo_aux = NULL;
	if(*lista == NULL)
	{
		//La lista esta vacia
		printf ("No se encontro el proceso de espacio en swap %d en la lista porque estaba vacia\n",espacio_mover);
		return ERROR;
	}
	nodo_aux = *lista;
	while((nodo_aux != NULL)&&(nodo_aux->pos_en_swap != espacio_mover))
	{
		nodo_aux = nodo_aux->siguiente; //se recorre la lista hasta encontrar el elemento o llegar al final
	}
	if(nodo_aux == NULL)
	{
		printf("El proceso de espacio en swap %d no estaba en la lista\n",espacio_mover);
		return ERROR;
	}
	//se encontro el elemento que tiene esa posicion en swap
	//retorno el pid
	return nodo_aux->pid;
}

int cant_paginas_segun_pid(nodo_lista **lista,pid_t pid)
{
	nodo_lista *nodo_aux = NULL;
	int cant_paginas_proceso = 0;
	if(*lista == NULL)
	{
		//La lista esta vacia
		printf ("No se encontro el proceso de pid %d en la lista porque estaba vacia\n",pid);
		return ERROR;
	}
	nodo_aux = *lista;
	while((nodo_aux != NULL)&&(nodo_aux->pid != pid))
	{
		nodo_aux = nodo_aux->siguiente; //se recorre la lista hasta encontrar el elemento o llegar al final
	}
	if(nodo_aux == NULL)
	{
		printf("El proceso de pid %d no estaba en la lista\n",pid);
		return ERROR;
	}
	//se encontro el primer elemento que tiene ese pid
	//retorno la posicion de swap
	do
	{
		cant_paginas_proceso++;
		nodo_aux = nodo_aux->siguiente;
	}while((nodo_aux != NULL)&&(nodo_aux->pid == pid));
	return cant_paginas_proceso;
}

int mover_paginas_proceso(char *puntero_binario,int espacio_libre,int espacio_mover,int cant_paginas_mover)
{
	int tam_pag = StringToNum(config.tam_pag);

	int offset_destino, offset_origen;
	int cant_movida = 0;


	offset_destino = sizeof(char)*tam_pag*espacio_libre;
	offset_origen = sizeof(char)*tam_pag*espacio_mover;

	//almaceno los bytes

	for(;cant_movida<cant_paginas_mover;cant_movida++)
	{
		log_info(LOG, string_from_format("Se mueve pagina %d de la posicion swap %d a la posicion %d", cant_movida, offset_origen,offset_destino));
		memcpy(puntero_binario+offset_destino, puntero_binario+offset_origen, tam_pag);
		offset_destino = offset_destino + sizeof(char)*tam_pag;
		offset_origen = offset_origen + sizeof(char)*tam_pag;

	}

	return (true);

}

int marcar_como_libre(t_bitarray* bitmap,int espacio_mover,int cant_paginas_mover)
{
	int indice_espacios = espacio_mover;
	int maximo_espacios = StringToNum(config.cant_pag);
	//int maximo_espacios = bitarray_get_max_bit(bitmap);

	if(espacio_mover+cant_paginas_mover>maximo_espacios)
	{
		return ERROR;
	}

	for(;indice_espacios<espacio_mover+cant_paginas_mover;indice_espacios++)
	{
		bitarray_clean_bit(bitmap, indice_espacios);
	}

	return (true);


}

int marcar_como_ocupado(t_bitarray* bitmap,int espacio_libre,int cant_paginas_mover)
{
	int indice_espacios = espacio_libre;
	int maximo_espacios = StringToNum(config.cant_pag);
	//int maximo_espacios = bitarray_get_max_bit(bitmap);

	if(espacio_libre+cant_paginas_mover>maximo_espacios)
	{
		return ERROR;
	}

	for(;indice_espacios<espacio_libre+cant_paginas_mover;indice_espacios++)
	{
		bitarray_set_bit(bitmap, indice_espacios);
	}

	return (true);
}

int editar_inicio_swap(nodo_lista **lista,pid_t pid,int espacio_libre)
{
	nodo_lista *nodo_aux = NULL;
	int nueva_pos_swap = espacio_libre;
	if(*lista == NULL)
	{
		//La lista esta vacia
		printf ("No se encontro el proceso de pid %d en la lista porque estaba vacia\n",pid);
		return ERROR;
	}
	nodo_aux = *lista;
	while((nodo_aux != NULL)&&(nodo_aux->pid != pid))
	{
		nodo_aux = nodo_aux->siguiente; //se recorre la lista hasta encontrar el elemento o llegar al final
	}
	if(nodo_aux == NULL)
	{
		printf("El proceso de pid %d no estaba en la lista\n",pid);
		return ERROR;
	}
	//se encontro el primer elemento que tiene ese pid

	do
	{
		nodo_aux->pos_en_swap = nueva_pos_swap;
		nueva_pos_swap++;
		nodo_aux = nodo_aux->siguiente;
	}while((nodo_aux != NULL)&&(nodo_aux->pid == pid));
	return (true);
}

void mostrar_bitmap(t_bitarray* bitmap)
{
	int indice_espacios;

	int maximo_espacios = StringToNum(config.cant_pag);

	printf("bitmap: indice -> estado\n");
	printf("0 = libre; 1 = ocupado\n");

	for(indice_espacios=0;indice_espacios<maximo_espacios;indice_espacios++)
	{
		printf("%d -> %d\n",indice_espacios,bitarray_test_bit(bitmap, (off_t) indice_espacios));
	}
}
