
#include "Protocolo.h"
#include <signal.h>
#include "PCB.h"

t_pcb* pcb;
t_log*  LOG;
s_conf config;
int estadoPcb;

int tamanio_pagina; //recibe de umc

int  quantum;		//recibe de nucleo
int  quantumSleep;
char* idsIO;
char* idsSem;
char* sharVars;

int NucleoSock, UmcSock;

t_dictionary* argDicc;
t_dictionary* varDicc;
char* listaArg;
char* listaVar;			//asumo como maximo 100 variables por contexto


t_list * listaIndicesStack;
t_indice_stack* indiceStack;

int flagInt; // flag para la interrupcion SIGUSR1

void   controladorSIGUSR1 (int signalID);
void   ejecutar_pcb(t_pcb* pcb, t_log* LOG);
char*  pedir_codigo(t_intructions instruccion, t_log* LOG);
t_indice_stack* nuevo_indice_stack(t_dictionary* argDicc, t_dictionary* varDicc);
void initialize_indice_stack(t_indice_stack* struct_indice_stack);
void* serializar_indice_stack(t_indice_stack* struct_indice_stack, char* listaArg, char* listaVar, u_int32_t* tamanio);
void deserializar_indice_stack(t_indice_stack* struct_indice_stack, void* serializado, char* listaArg, char* listaVar);
void deserializar_lista_indiceStack(t_list * lista, t_indice_stack* indice_stack, void* serializado, char* listaArg, char* listaVar, u_int32_t indice_contexto);
void* serializar_lista_indiceStack  (t_list * lista, t_indice_stack* indice_stack, char* listaArg, char* listaVar, u_int32_t indice_contexto, u_int32_t* tamanio);




	AnSISOP_funciones functions = {
				.AnSISOP_definirVariable		= &AnSISOP_definirVariable,
				.AnSISOP_obtenerPosicionVariable= &AnSISOP_obtenerPosicionVariable,
				.AnSISOP_dereferenciar			= &AnSISOP_dereferenciar,
				.AnSISOP_asignar				= &AnSISOP_asignar,
				.AnSISOP_obtenerValorCompartida = &AnSISOP_obtenerValorCompartida,
				.AnSISOP_asignarValorCompartida = &AnSISOP_asignarValorCompartida,
				.AnSISOP_irAlLabel              = &AnSISOP_irAlLabel,
				.AnSISOP_llamarConRetorno       = &AnSISOP_llamarConRetorno,
				.AnSISOP_finalizar              = &AnSISOP_finalizar,
				.AnSISOP_retornar               = &AnSISOP_retornar,
				.AnSISOP_imprimir				= &AnSISOP_imprimir,
				.AnSISOP_imprimirTexto			= &AnSISOP_imprimirTexto,
				.AnSISOP_entradaSalida          = &AnSISOP_entradaSalida,
		};

	AnSISOP_kernel kernel_functions = {
				.AnSISOP_wait                   = &AnSISOP_wait,
				.AnSISOP_signal                 = &AnSISOP_signal,
		};




int main(int argc, char* argv[]){

	signal (SIGUSR1, controladorSIGUSR1);
	flagInt = 0;

	int status;

	listaIndicesStack = list_create();

	argDicc = dictionary_create();
	varDicc = dictionary_create();

	listaArg = malloc(30);
	listaVar = malloc(100);

	pcb = malloc(sizeof(t_pcb));initialize_pcb(pcb);


	LOG = log_create("Cpu.log","CPU.c",0,1);
	log_info(LOG,"\n////////////////////////////////////////////////////////////////////////comienza módulo.");
	log_info(LOG, "Se lee archivo de configuración");


	if (argc != 2){
		log_error(LOG, "No se indicó la ruta del archivo de configuración");
		return EXIT_FAILURE;
	}else{config = get_config(LOG);}


	log_info(LOG,"Creando sockets");


	//creacion de socket cliente para nucleo
	NucleoSock = create_socket_cliente(config.ip_nucleo, config.puerto_nucleo, LOG);
	log_info(LOG, "Socket conectado Nucleo");

	//creacion de socket cliente para umc
	UmcSock = create_socket_cliente(config.ip_umc, config.puerto_umc, LOG);
	log_info(LOG, "Socket conectado con UMC");



	//Saludo a UMC y espero el tamaño de pagina :D
	log_info(LOG, "handshake con UMC");
	realize_handshake(UmcSock, LOG);
	status = receive_tamanio_pagina(UmcSock, &tamanio_pagina, LOG);
		if (status < 0)return EXIT_FAILURE;

	log_info(LOG, string_from_format("Se recibio el tamaño de página %d", tamanio_pagina));


	//Saludo a nucleo y se espera en orden: IDS_SEM, IDS_IO, IDS_SHARED, QUANTUM y QUANTUM_SLEEP
	log_info(LOG, "handshake con nucleo");

	realize_handshake(NucleoSock, LOG);

	idsSem   =	receive_string_de_ids(NucleoSock, IDS_SEM,    LOG);
	idsIO    =	receive_string_de_ids(NucleoSock, IDS_IO,     LOG);
	sharVars =	receive_string_de_ids(NucleoSock, IDS_SHARED, LOG);


	//status = receive_quantum(NucleoSock, &quantum, LOG);
	//	if (status < 0)return EXIT_FAILURE;
	//log_info(LOG, string_from_format("Se recibio el tamaño del quantum: %d", quantum));

	status = receive_quantumSleep(NucleoSock, &quantumSleep, LOG);
		if (status < 0)return EXIT_FAILURE;

	log_info(LOG, string_from_format("Se recibio un quantum sleep de: %d", quantumSleep));


	//Se espera y recibe PCB de nucleo
	log_info(LOG, "Se espera pcb");

	while (receive_pcb(NucleoSock, pcb, LOG) == NULL);

	log_info(LOG, string_from_format("Se recibio pcb numero %d", pcb->ID));

	indiceStack = nuevo_indice_stack(argDicc, varDicc);

	quantum = (pcb->quantum);log_info(LOG, "Se recibio el tamaño del quantum: %d", quantum);
	quantumSleep = (pcb->quantum_sleep);log_info(LOG, "Se recibio el tamaño del quantum sleep: %d", quantumSleep);

	if(pcb->indice_stack_size != 0){
		log_info(LOG, "se deserializa indice de stack");
		deserializar_lista_indiceStack(listaIndicesStack, indiceStack, pcb->indice_stack, listaArg, listaVar, pcb->indice_contexto);
		}

	do{
		estadoPcb = -1;

		printf("------------ Pid: %d ------------\n", pcb->ID);

		log_info(LOG, "Se notifica cambio a proceso %d", pcb->ID);
		notificar_cambio_proceso_activo(UmcSock, pcb->ID, LOG);

		log_info(LOG, "Se procede a ejecutar PCB");
		ejecutar_pcb(pcb, LOG);


		if (estadoPcb == -1){
			estadoPcb = FIN_QUANTUM;
			log_info(LOG, "Se termino el quantum");
			notificar_estado_pcb(NucleoSock, estadoPcb, LOG);
		}

		pcb->indice_stack = serializar_lista_indiceStack  (listaIndicesStack, indiceStack, listaArg, listaVar, pcb->indice_contexto, &(pcb->indice_stack_size));


		send_pcb(NucleoSock, pcb, LOG);		//devuelvo a nucleo sin importar estado


		//reviso el flag y le aviso a nucleo si cpu sigue ejecutando o no

		if (flagInt == 1) {
			socket_send_end(NucleoSock);
			printf("Dejo de ejecutar.\n");
			exit(0);
		}
		else socket_ok_response(NucleoSock);


		log_info(LOG, "Se espera nuevo pcb");

		while (receive_pcb(NucleoSock, pcb, LOG) == NULL);

		log_info(LOG, string_from_format("Se recibio pcb numero %d", pcb->ID));

		free(indiceStack);
		indiceStack = nuevo_indice_stack(argDicc, varDicc);

		quantum = (pcb->quantum);log_info(LOG, "Se recibio el tamaño del quantum: %d", quantum);
		quantumSleep = (pcb->quantum_sleep);log_info(LOG, "Se recibio el tamaño del quantum sleep: %d", quantumSleep);

		if(pcb->indice_stack_size != 0){
			log_info(LOG, "se deserializa indice de stack");
			deserializar_lista_indiceStack(listaIndicesStack, indiceStack, pcb->indice_stack, listaArg, listaVar, pcb->indice_contexto);}

	}while(1);


	list_destroy_and_destroy_elements(listaIndicesStack, free);
	log_info(LOG, "finaliza");

//	close(NucleoSock); deberia?

//	close(UmcSock);	   deberia?

	log_destroy(LOG);
	printf("LOG destruído\n");

	dictionary_destroy(argDicc);
	dictionary_destroy(varDicc);

	free(listaArg);
	free(listaVar);

	return EXIT_SUCCESS;

	}

void ejecutar_pcb(t_pcb* pcb, t_log* LOG){

	log_info(LOG, "Se comienza a ejecutar");

	int i;
	for (i = 0; i < quantum; i++) {


		t_intructions instruccion;

		log_info(LOG, "Se calcula siguiente instruccion a ejecutar");

		instruccion=pcb->p_instrucciones[pcb->PC];

		log_info(LOG, string_from_format("se pide instruccion start: %d  offset: %d", instruccion.start, instruccion.offset));

		pcb->PC++;	//avanza el pc

		char* codigo = pedir_codigo(instruccion, LOG);

		if (quantum > 0){
		analizadorLinea(codigo, &functions, &kernel_functions);

		free(codigo);

		log_info(LOG, "Se ejecuto instruccion: %d Quantum restante: %d",(pcb->PC-1), (quantum-(i+1) < 0)?0:quantum-(i+1) );

		log_info(LOG, string_from_format("QuantumSleep de: %d", quantumSleep));
		usleep(quantumSleep*1000);  //pasaje de mili a micro-segundos
		log_info(LOG, string_from_format("finalizo quantum sleep"));

		printf("  ----------------------------\n");

		}
	}


}




///	analizadorLinea(string_duplicate("a = a + b"), &functions, &kernel_functions);   //temporal

char* pedir_codigo(t_intructions instruccion, t_log* LOG){


	int pagina  = instruccion.start / tamanio_pagina;
	int offset  = instruccion.start - (pagina * tamanio_pagina);
	int tamanio = instruccion.offset;

	char* data; // = malloc(tamanio+1);


	data = pedir_bytes_a_umc(pagina, offset, tamanio);


	if (quantum > 0)
	log_info(LOG, "se ejecuta: %s", data);

	return data;
}

char* pedir_bytes_a_umc(int pagina, int offset, int tamanio){

	int status = 0;
	char* data = malloc(tamanio+1);
	char* error = string_new();

	if (tamanio > (tamanio_pagina - offset) ){

		int   tamActual = tamanio_pagina - offset;
		int tamRestante = tamanio - tamActual;
		int tamAnterior = 0;

		while(tamActual > 0){

			void* buffer = malloc(tamActual);

			status = pedir_bytes_por_socket(UmcSock, pagina, offset, tamActual, buffer, error, LOG);

			if(status == OK){

				memcpy(data+tamAnterior, buffer, tamActual);

				log_info(LOG, string_from_format("Se pidio pagina: %d offset: %d tamaño: %d",pagina, offset, tamActual));

				pagina++;
				offset = 0;
				tamAnterior = tamAnterior + tamActual;
				tamActual   = tamRestante;
				tamRestante = 0;
				free(buffer);

				if(tamActual > tamanio_pagina){

					tamRestante = tamActual - tamanio_pagina;
					tamActual   = tamanio_pagina;
					}
				}

			else if(status == FALLO) {
				quantum = 0;
				estadoPcb = FINALIZO;
				log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Error recibiendo bytes");
				notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error recibiendo bytes"), LOG);
				break;
			}
			else if(status == PEDIDO_INVALIDO) {
				quantum = 0;
				estadoPcb = FINALIZO;
				log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Se hizo un pedido invalido en memoria");
				notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Pedido invalido en memoria"), LOG);
				break;
			}
			else if(status == NO_HAY_MEMORIA) {
				quantum = 0;
				estadoPcb = FINALIZO;
				log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "No hay memoria disponible");
				notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("No hay memoria disponible"), LOG);
				break;
			}

		}
	}

	else{ //si (tamanio <= tamanio_pagina - offset)

		status = pedir_bytes_por_socket(UmcSock, pagina, offset, tamanio, data, error, LOG);

		if(status == OK) log_info(LOG, string_from_format("Se pidio pagina: %d offset: %d tamaño: %d",pagina, offset, tamanio));

		else if(status == FALLO) {
			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Error recibiendo bytes");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error recibiendo bytes"), LOG);
		}
		else if(status == PEDIDO_INVALIDO) {
			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Se hizo un pedido invalido en memoria");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Pedido invalido en memoria"), LOG);
		}
		else if(status == NO_HAY_MEMORIA) {
			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "No hay memoria disponible");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("No hay memoria disponible"), LOG);
		}

	}


	data[tamanio]='\0';
	return data;
	}


void enviar_bytes_a_umc(int pagina,int offset,int tamanio,void* data){

	int status = 0;
	char* error = string_new();

	if (tamanio > tamanio_pagina - offset){

		int   tamActual = tamanio_pagina - offset;
		int tamRestante = tamanio - tamActual;
		int tamAnterior = 0;


		while(tamActual > 0){

			status = enviar_bytes_por_socket(UmcSock, pagina, offset, tamActual, data+tamAnterior, error, LOG);

			if (status == OK){

			log_info(LOG, "se almacena en pagina %d con offset %d y tamaño %d", pagina, offset, tamActual);

			pagina++;
			offset = 0;
			tamAnterior = tamAnterior + tamActual;
			tamActual   = tamRestante;
			tamRestante = 0;

			if(tamActual > tamanio_pagina){

				tamRestante = tamActual - tamanio_pagina;
				tamActual   = tamanio_pagina;
				}

			}

			else {

				if(status == FALLO){

				quantum = 0;
				estadoPcb = FINALIZO;
				log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Fallo almacenar bytes");
				notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Fallo almacenar bytes"), LOG);
				return;
				}
				else if(status == PEDIDO_INVALIDO){

				quantum = 0;
				estadoPcb = FINALIZO;
				log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Stack Overflow");
				notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Stack Overflow"), LOG);
				return;
				}
				else if(status == NO_HAY_MEMORIA){

				quantum = 0;
				estadoPcb = FINALIZO;
				log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "No hay memoria");
				notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("No hay memoria"), LOG);
				return;
				}
			}
		}
	}

	else{ //si (tamanio <= tamanio_pagina - offset)

		status = enviar_bytes_por_socket(UmcSock, pagina, offset, tamanio, data, error, LOG);

		if (status == OK) log_info(LOG, "se almacena en pagina %d con offset %d y tamaño %d", pagina, offset, tamanio);

		else {

			if(status == FALLO){

			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Fallo almacenar bytes");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Fallo almacenar bytes"), LOG);
			}
			else if(status == PEDIDO_INVALIDO){

			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "Stack Overflow");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Stack Overflow"), LOG);
			}
			else if(status == NO_HAY_MEMORIA){

			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se deja de ejecutar por %s y se envia aviso de finaliza programa", "No hay memoria");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("No hay memoria"), LOG);
			}
		}

	}

}

t_indice_stack* nuevo_indice_stack(t_dictionary* argDicc, t_dictionary* varDicc){

	t_indice_stack* struct_indice_stack = malloc(6*sizeof(int));

	struct_indice_stack->argDicc = argDicc;
	struct_indice_stack->varDicc = varDicc;

	initialize_indice_stack(struct_indice_stack);

	return struct_indice_stack;

}

void initialize_indice_stack(t_indice_stack* struct_indice_stack){

	struct_indice_stack->retVar    = -1;
	struct_indice_stack->retPos    = -1;

	dictionary_clean(varDicc);
	dictionary_clean(argDicc);

}

void* serializar_indice_stack(t_indice_stack* struct_indice_stack, char* listaArg, char* listaVar,u_int32_t* tamanio){

	int cantidades[4];

	if (struct_indice_stack->retVar != -1) cantidades[0] = 1; //verifica si hay variable donde retornar
		else cantidades[0] = 0;

	if (struct_indice_stack->retPos != -1)	  cantidades[1] = 1; //verifica si hay posicion donde retornar
		else cantidades[1] = 0;

	int t = dictionary_size(struct_indice_stack->argDicc);
	if (t != 0 )							  cantidades[2] = t; //verifica si hay argumentos recibidos
		else cantidades[2] = 0;

	t = dictionary_size(struct_indice_stack->varDicc);
	if (t != 0 )							  cantidades[3] = t; //verifica si hay variables locales
		else cantidades[3] = 0;



	(*tamanio) =  4*sizeof(int) + cantidades[0]*sizeof(int) + cantidades[1]*sizeof(int) + cantidades[2]*5 + cantidades[3]*5; // 5 = char+int
	void* buffer = malloc (*tamanio);
	int offset = 0;

	memcpy(buffer+offset, cantidades, 4*sizeof(int));	offset += 4*sizeof(int);

	if(cantidades[0]){
	memcpy(buffer+offset, &struct_indice_stack->retVar, sizeof(int)); offset += sizeof(int);}

	if(cantidades[1]){
	memcpy(buffer+offset, &struct_indice_stack->retPos, sizeof(int)); offset += sizeof(int);}

	void* nameString;
	int   direccion;


	while(cantidades[2]--){			//aca copio argumentos en la forma [ char id | int direccion ]

		nameString = string_from_format("%c", listaArg[cantidades[2]]);

		memcpy(buffer+offset, nameString, 1); offset += sizeof(char);

		direccion =(int) dictionary_get( (struct_indice_stack->argDicc), nameString);

		memcpy(buffer+offset, &direccion, 4); offset += sizeof(int);

		free(nameString);
	}

	while(cantidades[3]--){			//aca copio variables en la forma [ char id | int direccion ]

		nameString = string_from_format("%c", listaVar[cantidades[3]]);

		memcpy(buffer+offset, nameString, 1); offset += sizeof(char);

		direccion =(int) dictionary_get( (struct_indice_stack->varDicc), nameString);

		memcpy(buffer+offset, &direccion, 4); offset += sizeof(int);

		free(nameString);
	}


	dictionary_clean(struct_indice_stack->argDicc);
	dictionary_clean(struct_indice_stack->varDicc);
	initialize_indice_stack(struct_indice_stack);


	return(buffer);


}


void deserializar_indice_stack(t_indice_stack* struct_indice_stack, void* serializado, char* listaArg, char* listaVar){

	initialize_indice_stack(struct_indice_stack);
	dictionary_clean(struct_indice_stack->argDicc);
	dictionary_clean(struct_indice_stack->varDicc);

	int cantidades[4];
	memcpy(cantidades, serializado, 4*sizeof(int)); int offset =  4*sizeof(int);

	if(cantidades[0]){
	memcpy(&struct_indice_stack->retVar, serializado+offset, sizeof(int)); offset += sizeof(int);}

	if(cantidades[1]){
	memcpy(&struct_indice_stack->retPos, serializado+offset, sizeof(int)); offset += sizeof(int);}

	char nameAux;
	int  contAux;
	char* strAux;

	while(cantidades[2]--){

		memcpy(&nameAux, serializado+offset, sizeof(char)); offset += sizeof(char);
		memcpy(&contAux, serializado+offset, sizeof(int));  offset += sizeof(int);

		listaArg[dictionary_size(struct_indice_stack->argDicc)] = nameAux;
		strAux = string_from_format("%c", nameAux);

		dictionary_put(struct_indice_stack->argDicc, strAux,(void*)contAux);

		free(strAux);
	}

	while(cantidades[3]--){

		memcpy(&nameAux, serializado+offset, sizeof(char)); offset += sizeof(char);
		memcpy(&contAux, serializado+offset, sizeof(int));  offset += sizeof(int);

		listaVar[dictionary_size(struct_indice_stack->varDicc)] = nameAux;
		strAux = string_from_format("%c", nameAux);

		dictionary_put(struct_indice_stack->varDicc, strAux,(void*)contAux);

		free(strAux);
	}


}


void* serializar_lista_indiceStack  (t_list * lista, t_indice_stack* indice_stack, char* listaArg, char* listaVar, u_int32_t indice_contexto, u_int32_t* tamanio){

	void* serializado;

	if (indice_contexto == 0){

		void* buffer;
		u_int32_t tam;
		int offset = sizeof(int);


		buffer = serializar_indice_stack(indiceStack, listaArg, listaVar, &tam);

		tam += sizeof(int);

		serializado = malloc( tam );

		memcpy(serializado, &tam, sizeof(int));
		memcpy(serializado + offset, buffer, ( tam - offset) );

		(*tamanio) = tam;


		free(buffer);
		}

	else if (indice_contexto > 0){

		int i;
		void* buffer;
		u_int32_t tam;
		int offset = 0;
		int tamanio_total = 0;

		serializado = malloc(1);


		for( i = 0 ; i != indice_contexto ; i++ ){



			buffer = list_get(lista, i);

			memcpy(&tam, buffer, sizeof(int));

			tamanio_total += tam;

			serializado = realloc (serializado, tamanio_total);
				if (serializado == NULL) log_info(LOG, "Error en realloc");

			memcpy(serializado + offset, buffer, tam);
			offset += tam;

			free(buffer);

			}


		buffer = serializar_indice_stack(indiceStack, listaArg, listaVar, &tam);

		tam += sizeof(int);
		tamanio_total += tam;

		serializado = realloc (serializado, tamanio_total);
			if (serializado == NULL) log_info(LOG, "Error en realloc");


		memcpy(serializado + offset, &tam, sizeof(int));	offset += sizeof(int);

		memcpy(serializado + offset, buffer, tam - sizeof(int));

		(*tamanio) = tamanio_total;

		list_clean(lista);

		free(buffer);

		}

	else{
		quantum = 0;
		estadoPcb = FINALIZO;
		log_info(LOG, "Error en serializar lista de indices de stack");
		notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error en pcb."), LOG);
	}



	return serializado;

}


void deserializar_lista_indiceStack(t_list * lista, t_indice_stack* indice_stack, void* serializado, char* listaArg, char* listaVar, u_int32_t indice_contexto){


	if (indice_contexto == 0){

		  deserializar_indice_stack(indice_stack, serializado + sizeof(int), listaArg, listaVar);

		}

	else if (indice_contexto > 0){

		int i;
		int tamanio;
		char* buffer;
		int offset = 0;

		for ( i = 0; i != indice_contexto ; i++ ){

			memcpy(&tamanio, serializado + offset, sizeof(int)); offset = offset + sizeof(int);

			buffer = malloc(tamanio);


			memcpy( buffer, &tamanio, sizeof(int));

			memcpy( buffer + sizeof(int),serializado+offset, tamanio - sizeof(int));

			list_add( lista, buffer); offset = offset + tamanio - sizeof(int);


			}

		offset = offset + sizeof(int);

		deserializar_indice_stack(indice_stack, serializado + offset, listaArg, listaVar);

		}

	else{
		quantum = 0;
		estadoPcb = FINALIZO;
		log_info(LOG, "Error en deserializar lista de indices de stack");
		notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error en pcb."), LOG);
	}


}

   ////Primitivas////

t_puntero AnSISOP_definirVariable(t_nombre_variable identificador_variable) {
	printf("Se define variable %c \n", identificador_variable);

	log_info(LOG, string_from_format("Se procede a definir variable: %c en direccion: %u", identificador_variable, pcb->p_stackActual));

	listaVar[dictionary_size(indiceStack->varDicc)] = identificador_variable;

	char* key = string_substring(string_from_format("%c", identificador_variable),0,1);

	dictionary_put(varDicc , key, (void*)(pcb->p_stackActual));
	t_puntero puntero = (pcb->p_stackActual);


	pcb->p_stackActual = pcb->p_stackActual + sizeof(int);


	free(key);
	return puntero;


}

t_puntero AnSISOP_obtenerPosicionVariable(t_nombre_variable identificador_variable) {

	t_puntero posicion = -1;
	printf("Se obtiene posicion de variable %c\n", identificador_variable);

	log_info(LOG, string_from_format("Se procede a obtener posicion variable de: %c", identificador_variable));

	char* key = string_substring(string_from_format("%c", identificador_variable),0,1);


	if( dictionary_has_key(varDicc, key) ){
		posicion = dictionary_get(varDicc, key);
		log_info(LOG, string_from_format("%c es valida, se retorna la direccion: %u", identificador_variable, posicion));
		}
	else log_info(LOG, string_from_format("variable %c no es valida", identificador_variable));

	free(key);

	return posicion;

}

t_valor_variable AnSISOP_dereferenciar(t_puntero direccion_variable) {

	printf("Se obtiene valor de variable en direccion %u\n", direccion_variable);

	int nro_pag = direccion_variable / tamanio_pagina;
	int offset  = direccion_variable - (nro_pag * tamanio_pagina);

	void* receiver = pedir_bytes_a_umc(nro_pag, offset, sizeof(int));
	t_valor_variable valor;

	memcpy(&valor, receiver, sizeof(int));

	log_info(LOG, string_from_format("Se obtuvo valor: %d en dir: %u", valor, direccion_variable));

	free(receiver);

	return valor;

}


void AnSISOP_asignar(t_puntero direccion_variable, t_valor_variable valor) {

	printf("Se asigna valor %d a direccion %u\n", valor, direccion_variable);

	log_info(LOG, string_from_format("Se asigna valor: %d en: %u", valor, direccion_variable));

	int nro_pag = direccion_variable / tamanio_pagina;
	int offset  = direccion_variable - (nro_pag * tamanio_pagina);

	enviar_bytes_a_umc(nro_pag, offset, sizeof(int), &valor);
}


t_valor_variable AnSISOP_obtenerValorCompartida(t_nombre_compartida variable) {

	t_valor_variable valor;
	char* nombre = string_duplicate(variable);

	nombre = separarID(nombre);
	string_trim(&nombre);

	printf("Se pide valor de variable compartida: %s\n", nombre);

	valor = get_value_sharvar(NucleoSock, nombre, LOG);

	free(nombre);
	return valor;
}


t_valor_variable AnSISOP_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor) {

	t_valor_variable valorSeteado;

	char* nombre = string_duplicate(variable);

	nombre = separarID(nombre);
	string_trim(&nombre);

	printf("Se asigna valor %d a variable compartida %s\n", valor, variable);

	set_value_sharvar(NucleoSock, nombre, valor, LOG);
	valorSeteado = get_value_sharvar(NucleoSock, nombre, LOG);

	if(valor == valorSeteado) log_info(LOG, "asignacion exitosa de variable %s con valor %d", nombre, valor);
	else {
			log_error(LOG, "fallo asignacion de variable %s, se asigno %d y se seteo %d", nombre, valor, valorSeteado);
			quantum = 0;
			estadoPcb = FINALIZO;
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error en uso de variables compartidas."), LOG);
			free(nombre);
		}

	free(nombre);
	return valorSeteado;
}


void AnSISOP_irAlLabel(t_nombre_etiqueta t_nombre_etiqueta) {

	t_nombre_etiqueta = separarID(t_nombre_etiqueta);

	printf("Salta a etiqueta %s\n", t_nombre_etiqueta);

	pcb->PC = metadata_buscar_etiqueta(t_nombre_etiqueta, pcb->p_etiquetas, pcb->etiquetas_size);

	if (pcb->PC != -1)	log_info(LOG, "se salta a la etiqueta %s, se setea pc en %d", t_nombre_etiqueta, pcb->PC);

	else{
		quantum = 0;
		estadoPcb = FINALIZO;
		log_info(LOG, "La etiqueta %s no existe", t_nombre_etiqueta);
		notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error en label."), LOG);
	}


}

void AnSISOP_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {

	void* buffer;
	u_int32_t tamanio;
	void* serializado;

	etiqueta = separarID(etiqueta);

	printf("Llama a funcion: %s \n", etiqueta);


	buffer = serializar_indice_stack(indiceStack, listaArg, listaVar, &tamanio);


	serializado = malloc( tamanio + sizeof(int) );

	memcpy(serializado, &tamanio, sizeof(int) );
	memcpy(serializado + sizeof(int), buffer, tamanio );

	tamanio += sizeof(int);

	list_add_in_index(listaIndicesStack, pcb->indice_contexto , serializado);
	(pcb->indice_contexto)++;


	(indiceStack->retPos)	 = (pcb->PC);
	(indiceStack->retVar)    = donde_retornar;


	log_info(LOG, "se llama a la funcion %s", etiqueta);

	AnSISOP_irAlLabel(etiqueta);

	free(buffer);
}

void AnSISOP_finalizar(void) {

	printf("Finaliza pid %d\n", pcb->ID);

	estadoPcb = FINALIZO;
	quantum = 0;
	notificar_estado_pcb(NucleoSock, estadoPcb, LOG);
}

void AnSISOP_retornar(t_valor_variable retorno) {


	printf("Retorna: %d en: %d\n", retorno, (indiceStack->retVar) );


	AnSISOP_asignar( (indiceStack->retVar), retorno);

	log_info(LOG,"Se cambia pc a instruccion: %d", pcb->PC);
	(pcb->PC) = (indiceStack->retPos);


	log_info(LOG,"Se pide contexto %d  a listaIndicesStack", pcb->indice_contexto - 1);

	void* buffer = list_get(listaIndicesStack, --pcb->indice_contexto);

	list_remove(listaIndicesStack, pcb->indice_contexto);

	{int a;
	memcpy(&a, buffer, sizeof(int));
	log_info(LOG, "tamaño :: %d", a);} //temporal para comprobar que el serializado no llega como deberia


	log_info(LOG,"Se deserializa contexto %d", pcb->indice_contexto);
	deserializar_indice_stack(indiceStack, buffer+sizeof(int), listaArg, listaVar);

	log_info(LOG,"Finaliza retornar");

	free(buffer);

}

void AnSISOP_imprimir(t_valor_variable valor_mostrar) {

	printf("Imprimir valor: %d\n", valor_mostrar);

	char* string = string_from_format("%d", valor_mostrar);

	imprimir_en_consola(NucleoSock, string, LOG);

	free(string);

}

void AnSISOP_imprimirTexto(char* texto) {

	printf("imprimir texto: %s \n", texto);

	char* string = string_from_format("%s", texto);

	imprimir_en_consola(NucleoSock, string, LOG);

	free(string);
}

void AnSISOP_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo) {

	printf("Entrada Salida en dispositivo %s durante %d \n", dispositivo, tiempo);
	quantum = 0;

	dispositivo = separarID(dispositivo);

	if ( strstr(idsIO, dispositivo) != NULL){
	estadoPcb = IO;

	notificar_io_pcb(NucleoSock, estadoPcb, dispositivo, tiempo, LOG);

	log_info(LOG, string_from_format("Se envio PCB %d a dispositivo %s por %d", pcb->ID, dispositivo, tiempo));
	}
	else{
		quantum = 0;
		estadoPcb = FINALIZO;
		log_info(LOG, "Se pidio IO en un dispositivo que no existe: %s", dispositivo);
		notificar_estado_error_pcb(NucleoSock, estadoPcb, "Error de IO.", LOG);
	}
}

void AnSISOP_wait(t_nombre_semaforo identificador_semaforo) {

	identificador_semaforo = separarID(identificador_semaforo);

	printf("Wait en semaforo %s\n", identificador_semaforo);

	if ( strstr(idsSem, identificador_semaforo) != NULL){

		int valorSem = notificar_semaforo_pcb(NucleoSock, SEM_GET, identificador_semaforo, LOG);

		if ( valorSem > 0 ){

			send_wait(NucleoSock, LOG);

			}

		else{
			quantum = 0;
			estadoPcb = SEM_QUEUE;
			notificar_estado_pcb(NucleoSock, estadoPcb, LOG);
			log_info(LOG, "Se envia pcb %d a wait por semaforo %s", pcb->ID, identificador_semaforo);
			}

		}
		else{
			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se pidio wait en un semaforo que no existe");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error en semaforos"), LOG);
		}
}

void AnSISOP_signal(t_nombre_semaforo identificador_semaforo) {

	identificador_semaforo = separarID(identificador_semaforo);

	printf("Signal en semaforo %s\n", identificador_semaforo);

	if ( strstr(idsSem, identificador_semaforo) != NULL){

		notificar_semaforo_pcb(NucleoSock, SEM_SIG, identificador_semaforo, LOG);

		log_info(LOG, "Se envio Signal al semaforo %s desde pcb %d", identificador_semaforo, pcb->ID);
		}
		else{
			quantum = 0;
			estadoPcb = FINALIZO;
			log_info(LOG, "Se pidio signal en un semaforo que no existe");
			notificar_estado_error_pcb(NucleoSock, estadoPcb, string_duplicate("Error en semaforos"), LOG);
		}
}



void controladorSIGUSR1 (int signalID){

	switch(signalID){

		case SIGUSR1:
			printf("CPU se desconectara luego de esta ejecucion\n");
			flagInt = 1;
			break;

	}


}
