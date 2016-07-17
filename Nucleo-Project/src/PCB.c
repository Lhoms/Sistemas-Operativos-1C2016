/*
 * PCB.c
 *
 *  Created on: 24/5/2016
 *      Author: utnso
 */

#include "Protocolo.h"
#include "PCB.h"
#include "serialize.h"
#include "Socket.h"
#include "Nucleo.h"



void* serializarPCB(t_pcb* pcb,int* pcb_size){

	size_t tamanio_iCodigo = 8 * pcb->instrucciones_size;


	(*pcb_size) =  11*sizeof(int) + tamanio_iCodigo + pcb->etiquetas_size + pcb->indice_stack_size;

	void* serializado = malloc(*pcb_size);	int offset = 0;


	serialize_from_int(serializado, pcb->ID, &offset);							// ID

	serialize_from_int(serializado, pcb->quantum, &offset);						// Quantum

	serialize_from_int(serializado, pcb->quantum_sleep, &offset);				// Quantum Sleep

	serialize_from_int(serializado, pcb->PC, &offset);							// PC

	serialize_from_int(serializado, pcb->cant_Paginas, &offset);				// Cantidad de paginas


	serialize_from_int(serializado, pcb->instrucciones_size, &offset);			// Cantidad de instrucciones

	memcpy(serializado + offset, pcb->p_instrucciones, tamanio_iCodigo);		// Indice de Codigo
	offset += tamanio_iCodigo;


	serialize_from_int(serializado, pcb->etiquetas_size, &offset);				// Tamaño del iEtiquetas

	memcpy(serializado + offset, pcb->p_etiquetas, pcb->etiquetas_size);		// iEtiquetas
	offset += (pcb->etiquetas_size);


	serialize_from_int(serializado, pcb->p_stack, &offset);						// Puntero al inicio del stack

	serialize_from_int(serializado, pcb->p_stackActual, &offset);				// Puntero indice del stack

	serialize_from_int(serializado, pcb->indice_contexto, &offset);				// Indice del contexto

	serialize_from_int(serializado, pcb->indice_stack_size, &offset);			// Tamaño indice de stack

	memcpy(serializado + offset, pcb->indice_stack, pcb->indice_stack_size);	// estructura serializada del indice de stack
	offset += (pcb->indice_stack_size);

	return serializado;

	}


void deserializarPCB(void* serializado, t_pcb* pcb){

	initialize_pcb(pcb);

	int offset = 0;

	deserialize_to_t_size(serializado, &pcb->ID, &offset);					  		// Carga ID

	deserialize_to_t_size(serializado, &pcb->quantum, &offset);					  	// Carga Quantum

	deserialize_to_t_size(serializado, &pcb->quantum_sleep, &offset);				// Carga Quantum Sleep

	deserialize_to_t_size(serializado, &pcb->PC, &offset);			  				// Carga PC

	deserialize_to_t_size(serializado, &pcb->cant_Paginas,&offset);				 	// Carga Cantidad de Paginas


	deserialize_to_t_size(serializado, &pcb->instrucciones_size, &offset);			// Carga Cantidad de Instrucciones

	size_t tamanio_iCodigo = sizeof(t_intructions) * (pcb->instrucciones_size);		// Calcula y carga indice de codigo
	pcb->p_instrucciones = malloc(tamanio_iCodigo);
	memcpy( pcb->p_instrucciones, serializado + offset, tamanio_iCodigo);
	offset += tamanio_iCodigo;

	deserialize_to_t_size(serializado, &pcb->etiquetas_size, &offset);				// Tamaño del iEtiquetas


	pcb->p_etiquetas = malloc(pcb->etiquetas_size);									// Calcula y carga indice de etiquetas
	memcpy( pcb->p_etiquetas, serializado + offset, pcb->etiquetas_size);
	offset += pcb->etiquetas_size;


	deserialize_to_t_size( serializado, &pcb->p_stack,&offset);						// Puntero al inicio del stack

	deserialize_to_t_size( serializado, &pcb->p_stackActual, &offset);			    // Puntero indice del stack

	deserialize_to_t_size( serializado, &pcb->indice_contexto, &offset);			// Numero del contexto actual

	deserialize_to_t_size( serializado, &pcb->indice_stack_size, &offset);			// Tamaño indice stack

	pcb->indice_stack = malloc(pcb->indice_stack_size);								// estructura indice stack serializada
	memcpy( pcb->indice_stack, serializado + offset, pcb->indice_stack_size);
	offset += pcb->indice_stack_size;


	}


void initialize_pcb(t_pcb* pcb){

	pcb->ID = 0;
	pcb->quantum = 0;
	pcb->quantum_sleep=0;
	pcb->PC = 0;
	pcb->cant_Paginas = 0;
	pcb->instrucciones_size = 0;
	pcb->p_instrucciones = NULL;
	pcb->etiquetas_size = 0;
	pcb->p_etiquetas = NULL;
	pcb->p_stack = 0;
	pcb->p_stackActual = 0;
	pcb->indice_contexto = 0;
	pcb->indice_stack_size = 0;
	pcb->indice_stack = NULL;

	}

void serialize_from_t_size(void* buffer, size_t value, int* offset){
	int offset_loc = *offset;

	memcpy(buffer + offset_loc, &value, sizeof(size_t));

	*offset = offset_loc + sizeof(size_t);
}

void deserialize_to_t_size(void* buffer, size_t* value, int* offset){

	int offset_loc = *offset;

	memcpy(value, buffer + offset_loc, sizeof(size_t));

	*offset = offset_loc + sizeof(size_t);
}

void send_pcb(int socket, t_pcb* pcb){

	int pcb_size;
	void* senderBuffer;

	void* pcbBuffer = serializarPCB(pcb, &pcb_size);

	struct s_header senderPcb;
	senderPcb.id = PCB_ESTRUCTURA;
	senderPcb.size = pcb_size;


	senderBuffer = malloc(sizeof(struct s_header) + pcb_size);

	serialize_from_header(senderPcb, senderBuffer);

	int offsett = sizeof(struct s_header);
	memcpy(senderBuffer + offsett , pcbBuffer , pcb_size);

	log_info(LOG, "Se envia pcb");
	int status = send(socket, senderBuffer, sizeof(struct s_header) + pcb_size, 0);

	if(status == -1) {log_error(LOG, "Error: no se envio pcb");        close(socket);}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket"); close(socket);}
	else{
		log_info(LOG, "Pcb enviado correctamente");
		//list_add(execState, pcb);
	}

}

t_pcb* receive_pcb(int socket, t_pcb* pcb){

	int 	 status;
	void*    receiverBuffer;
	struct 	 s_header receiverPcb;
  	initialize_pcb(pcb);


	log_info(LOG, "Se espera pcb");

  	status = recv(socket, &receiverPcb, sizeof(struct s_header), 0);
	if(status == -1) {log_error(LOG, "Error: no se recibio pcb");		;}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	manejar_caida_cpu(socket, serverSockUMC);;}

	else{
		if (receiverPcb.id == PCB_ESTRUCTURA){
			log_info(LOG, "Se recibio mensaje");
			receiverBuffer = malloc(receiverPcb.size);

			status = recv(socket, receiverBuffer, receiverPcb.size, 0);
		  	if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);}
		  	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(socket);}

		  	deserializarPCB(receiverBuffer, pcb);

		  	log_info(LOG, "El PCB recibido pertenece al PID: %d", pcb->ID);
		  	free(receiverBuffer);
			return pcb;
		}
	}

	log_error(LOG, "Error, se esperaba pcb");
	return pcb;
}

//extern pthread_mutex_t* serverIOM;

void create_list_states_RR(s_conf* config){
	/*int i=0;
	int dispositivos;
	pthread_mutex_t mutexDispositivo=PTHREAD_MUTEX_INITIALIZER;

	for(i=0;(config->io_id)[i]!=NULL;i++);
	dispositivos=i;

	serverIOM=malloc(sizeof(pthread_mutex_t)*dispositivos);
	for(i=0;i<dispositivos;i++){
		serverIOM[i]=mutexDispositivo;
	}

	//Inicializo el array de listas de estados io de pcbs
	inputOutputState=malloc(dispositivos*sizeof(t_list*));
	i=0;
	for(;i<dispositivos;i++){
		inputOutputState[i]=list_create();
	}*/

	//Creo las demás listas
	newState=list_create();
	readyState=list_create();
	execState=list_create();
	finishedState=list_create();
	listaConsolas=list_create();
	listaCPUs=list_create();

	//Inicializo como bloqueados ambos mutex.
	pthread_mutex_init(&mutex_ready, NULL);
	pthread_mutex_lock(&mutex_ready);

	pthread_mutex_init(&mutex_cpu, NULL);
	pthread_mutex_lock(&mutex_cpu);

}

t_pcb* crear_pcb_desde_codigo(void* codigo, int consolaSock, int umcSock, int tamanio_pagina, int paginas_stack){

	pthread_mutex_lock(&mutex_pid_count);

	log_info(LOG, "Se recibió un nuevo programa desde la consola %d", consolaSock);
	log_info(LOG, "El próximo PID disponible es: %d", pid_count + 1);

	t_pcb* pcbNuevo = malloc(sizeof(t_pcb));
	t_medatada_program* metadata;

	//Cálculo de la cantidad de paginas a pedir
	int tamanio_codigo = strlen(codigo) + 1;

	int cant_pag_codigo  = tamanio_codigo/tamanio_pagina + (tamanio_codigo%tamanio_pagina!=0? 1:0);
	int cant_pag_pedidas = cant_pag_codigo + paginas_stack;

	//Envio de mensaje a UMC
	log_info(LOG,string_from_format("Se envia a UMC %d:\nPID:%d\nCantidad Paginas Solicitadas:%d\n y ansisop antes recibido.",umcSock, pid_count + 1,cant_pag_pedidas));
	char* buffer = malloc (sizeof(struct s_header) + sizeof(int) + sizeof(int) + strlen(codigo) +1 );
	char* bufferCode = malloc (sizeof(int) + sizeof(int) + strlen(codigo) + 1);

	struct s_header senderHeader;
		senderHeader.id = INICIAR_PROGRAMA;
		senderHeader.size = sizeof(int) + sizeof(int) + strlen(codigo) + 1;

	serialize_from_header(senderHeader, buffer);
	//bufferCode = serialize_init_program(consolaSock , cant_pag_pedidas ,codigo);
	bufferCode = serialize_init_program(pid_count + 1 , cant_pag_pedidas ,codigo);

	memcpy(buffer+sizeof(struct s_header), bufferCode, sizeof(int) + sizeof(int) + strlen(codigo) + 1);

	send(umcSock, buffer, sizeof(struct s_header) + sizeof(int) + sizeof(int) + strlen(codigo) + 1, 0);

	int estado = receive_response(umcSock);
	if(estado != ERROR && estado != FALLO){

		log_info(LOG, string_from_format("Programa %d almacenado en swap perfectamente", pid_count + 1));

		//creacion  de pcb
		log_info(LOG, "Se crea PCB para el PID %d", pid_count + 1);
		initialize_pcb(pcbNuevo);
		metadata = metadata_desde_literal(codigo);

		//carga de pcb
		(pcbNuevo->ID) 					=  pid_count + 1;					     		// Se obtiene próximo PID
		(pcbNuevo->quantum)				=  config.quantum;								// Tamaño del quantum actual
		(pcbNuevo->PC)  				=  (metadata->instruccion_inicio);	     		// El numero de la primera instruccion (Begin)
		(pcbNuevo->instrucciones_size)  =  (metadata->instrucciones_size);				// Cantidad de instrucciones
		(pcbNuevo->p_instrucciones)     =  (metadata->instrucciones_serializado); 		// Instrucciones del programa
		(pcbNuevo->etiquetas_size) 		=  (metadata->etiquetas_size);					// Tamaño del mapa serializado de etiquetas
		(pcbNuevo->p_etiquetas)  		=  (metadata->etiquetas);						// La serializacion de las etiquetas
		(pcbNuevo->indice_stack_size)  	=  0;											// Indice tamaño 0
		(pcbNuevo->indice_stack) 	 	=  NULL;										// No tiene indice de stack, lo agrega cpu
		(pcbNuevo->cant_Paginas) 		=  cant_pag_pedidas;							// Paginas pedidas
		(pcbNuevo->indice_contexto) 	=  0;											// Inicia en main
		(pcbNuevo->p_stack) 			=  cant_pag_codigo*tamanio_pagina;				// Stack inicia en la siguiente pagina al fin del codigo
		(pcbNuevo->p_stackActual) 		=  (pcbNuevo->p_stack);							// Puntero al stack en umc

		log_info(LOG, "Se actualiza contador de PID");
		pid_count += 1;

		log_info(LOG, "Se asigna PID: %d a Consola %d", pid_count, consolaSock);
		asignar_pid_a_consola(consolaSock, pid_count);

	}else{
		log_error(LOG, "Falló inicialización de programa");
		initialize_pcb(pcbNuevo);
	}

	pthread_mutex_unlock(&mutex_pid_count);

	free(buffer);
	free(bufferCode);

	return pcbNuevo;
}

void asignar_pid_a_consola(int sock, int pid){

	bool search(void* parametro){
		t_socketConsola* act = (t_socketConsola*) parametro;
		return act->idSocketConsola == sock;
	}
	t_socketConsola* consola = list_find(listaConsolas, search);

	if(consola!=NULL){
		consola->pid_activo = pid;
	}
}

int obtener_pid_activo_consola(int sock){

	bool search(void* parametro){
		t_socketConsola* act = (t_socketConsola*) parametro;
		return act->idSocketConsola == sock;
	}
	t_socketConsola* consola = list_find(listaConsolas, search);

	if(consola!=NULL){
		return consola->pid_activo;
	}else{
		return -1;
	}
}

int obtener_consola_de_pid(int pid){

	bool search(void* parametro){
		t_socketConsola* act = (t_socketConsola*) parametro;
		return act->pid_activo == pid;
	}
	t_socketConsola* consola = list_find(listaConsolas, search);

	if(consola!=NULL){
		return consola->idSocketConsola;
	}else{
		return -1;
	}
}

/*int search_free_execState(int sockConsola){
	int j;
	int size_list=list_size(execState);
	int socketCpu;

	for(j=0;j<size_list;j++){
		t_socketCpu* sockList=list_get(execState,j);
		if(sockList->boolEstado==0 && !list_is_empty(readyState)){
			sockList->pcb=list_remove(readyState,0);
			sockList->boolEstado=1;
			socketCpu=sockList->idSocketCpu;
			log_info(LOG,string_from_format("Se encuentra cpu %d libre para ansisop de PID %d",socketCpu,sockList->pcb->ID));
			return socketCpu;
		}
	}
	if(list_is_empty(readyState)){
		log_info(LOG,"Esta vacio ready");
	}else{
		log_info(LOG,"No hay cpus libres");
	}

	return 0;
}*/

bool pcbSearchCpu(void * arg){
		t_socketCpu* cpu=(t_socketCpu*)arg;

		return (bool)(cpu->idSocketCpu==sockSearchCpu);
}

int receive_int_positive(int socket){
	int a;

	int status = recv(socket, &a, sizeof(int), 0);
	if(status == -1){
		log_error(LOG, string_from_format("Error: no se recibio valor entero"));
		a=-1;
	}else
		if(status ==  0){
			log_error(LOG,string_from_format("Error: se cayo el socket:%d",socket));
			close(socket);
			a=-1;
		}else
			log_info(LOG, string_from_format("Se recibio valor entero:%d",a));

	return a;
}

/*void move_from_ready_to_exec(t_log* LOG){
	int j=0;
	int size_list=list_size(execState);
	t_socketCpu* sockList;
	int next=1;

	//Recorro lista execState, viendo cual esta libre, si hay alguna cpu
	//asigno y envio pcb
	log_info(LOG,"Se busca cpu en execState para ejecutar algún ansisop de ready");

	for(j=0;j<size_list && next;j++){
		sockList=list_get(execState,j);
		if(sockList->boolEstado==0 && !list_is_empty(readyState)){
			sockList->pcb=list_remove(readyState,0);
			sockList->boolEstado=1;
			next=0;

			log_info(LOG,string_from_format("Se encuentra cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
			send_pcb(sockList->idSocketCpu,sockList->pcb);
		}
	}
	if(list_is_empty(readyState))
		log_info(LOG,"No hay ansisop que ejecutar");
	else
		log_info(LOG,"No se encontro cpu para el ansisop");

}*/

void desocupar_cpu(int sock){

	bool searchCPU(void* parametro){
		t_socketCpu* act = (t_socketCpu*) parametro;
		return act->idSocketCpu == sock;
	}

	t_socketCpu* cpu = list_find(listaCPUs, searchCPU);

	if(cpu != NULL){
		cpu->pid = 0;
		cpu->boolEstado = 0;
		pthread_mutex_unlock(&mutex_cpu);
		log_info(LOG, "Desocupo cpu %d", cpu->idSocketCpu);
	}
}

void delete_from_exec(int pid){

	bool searchPcb(void* parametro){
		t_pcb* act = (t_pcb*) parametro;
		return act->ID == pid;
	}

	//Saco de la lista de EXEC
	log_info(LOG, "Elimino el PCB con PID %d de exec", pid);
	list_remove_and_destroy_by_condition(execState, searchPcb, free);

}

void move_exec_to_ready(int socket, t_pcb* pcb){

	//Saco de Exec
	delete_from_exec(pcb->ID);

	//Agrego el nuevo a la lista de READY
	log_info(LOG, "Agrego PID %d a lista Ready", pcb->ID);
	list_add(readyState, pcb);
	pthread_mutex_unlock(&mutex_ready);

	//Desocupo CPU.
	desocupar_cpu(socket);

	/*sockSearchCpu=socket;
	t_pcb* pcbRecv=malloc(sizeof(t_pcb));
	t_socketCpu* sockList;
	log_info(LOG,("Se activa RR FIN DE QUANTUM"));

	//envio la llegada correcta del mensaje
	socket_ok_response(socket);

	receive_pcb(socket,pcbRecv);
	log_info(LOG,string_from_format("Se recibe pcb de cpu: %d PID:%d",socket,pcbRecv->ID));

	log_info(LOG,"CPU FINALIZO?");
	if(receive_response(socket)!=OK){
		list_remove_by_condition(execState,&pcbSearchCpu);
		log_info(LOG,string_from_format("Se eliminó un nodo en execState: %d execs",list_size(execState)));
		sockSearchCpu=search_free_execState(socket);
	}

	log_info(LOG,string_from_format("Se busca cpu %d en lista execState",socket));
	sockList=(t_socketCpu*)list_find(execState,&pcbSearchCpu);

	if(sockList!=NULL){//COMPROBAR QUE DEVUELVE NULL
		sockList->pcb=pcbRecv;
		sockList->boolEstado=1;
		log_info(LOG,string_from_format("Pongo a ejecutar en %d:a %d",sockList->idSocketCpu,pcbRecv->ID));
		send_pcb(sockList->idSocketCpu,pcbRecv);

		while(!list_is_empty(readyState) && (sockSearchCpu=search_free_execState(socket))){
			t_socketCpu* sockAux=list_find(execState,&pcbSearchCpu);
			sockAux->pcb=list_remove(readyState,0);
			sockAux->boolEstado=1;
			log_info(LOG,string_from_format("Se asigna a cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
			send_pcb(sockAux->idSocketCpu,sockAux->pcb);
		}
	}else{
				log_info(LOG,"No se encuentra cpu desocupada para atender algun ansisop");
			}
	*/
}

/*void delete_from_exec_and_asign(int socket){

	sockSearchCpu=socket;
	t_pcb* pcbRecv=malloc(sizeof(t_pcb));
	t_socketCpu* sockList;

	list_remove_by_condition(execState,&pcbSearchCpu);
	log_info(LOG,string_from_format("Se eliminó un nodo en execState: %d execs",list_size(execState)));
	sockSearchCpu=search_free_execState(socket);

	log_info(LOG,string_from_format("Se busca cpu %d en lista execState",socket));
	sockList=(t_socketCpu*)list_find(execState,&pcbSearchCpu);

	if(sockList!=NULL){//COMPROBAR QUE DEVUELVE NULL
		sockList->pcb=pcbRecv;
		sockList->boolEstado=1;
		log_info(LOG,string_from_format("Pongo a ejecutar en %d:a %d",sockList->idSocketCpu,pcbRecv->ID));
		send_pcb(sockList->idSocketCpu,pcbRecv);

		while(!list_is_empty(readyState) && (sockSearchCpu=search_free_execState(socket))){
			t_socketCpu* sockAux=list_find(execState,&pcbSearchCpu);
			sockAux->pcb=list_remove(readyState,0);
			sockAux->boolEstado=1;
			log_info(LOG,string_from_format("Se asigna a cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
			send_pcb(sockAux->idSocketCpu,sockAux->pcb);
		}
	}else{
			log_info(LOG,"No se encuentra cpu desocupada para atender algun ansisop");
		}

}*/

void move_from_exec_to_finish_error(int socketClient,int serverSockUMC, char* stringError, t_pcb* pcb){
	log_error(LOG,string_from_format("Recibo error de %d",socketClient));
	//t_pcb* pcb=malloc(sizeof(t_pcb));

	//socket_ok_response(socketClient);

	//char* stringError=string_itoa(error);
	//recvHeader.size=strlen(stringError)+1;

	void* sendBuffer=malloc(strlen(stringError)+1+sizeof(struct s_header));

	//Borro el PCB de Exec.
	delete_from_exec(pcb->ID);

	//Desocupo la CPU
	desocupar_cpu(socketClient);
//	sockSearchCpu=socketClient;
//	t_socketCpu* sockList=list_find(execState,&pcbSearchCpu);

	//receive_pcb(socketClient,pcb);
	list_add(finishedState,pcb);
	log_info(LOG,"Se agregó a finishedState el PID %d", pcb->ID);

	struct s_header recvHeader;

	//Chequeo que el mensaje no sea para una consola que se cerró
	t_socketConsola* consola = buscarConsola(pcb->ID);
	if(consola != NULL){
		log_info(LOG,"Le envio que finalizó a consola");
		//envio finalizo a consola y a umc
		recvHeader.id = FINALIZO;
		recvHeader.size= strlen(stringError) + 1;
		serialize_from_header(recvHeader,sendBuffer);
		int offset = sizeof(struct s_header);
		serialize_from_string(sendBuffer, stringError, &offset);
		send(consola->idSocketConsola,sendBuffer,sizeof(struct s_header) + strlen(stringError) + 1,0);

		limpiarConsola(consola->idSocketConsola);
	}

	log_info(LOG,"Le envio que finalizó a UMC");
	//envio a umc
	void* buffer = malloc(sizeof(struct s_header));

	recvHeader.id=FINALIZAR_PROGRAMA;
	recvHeader.size=sizeof(pcb->ID);

	serialize_from_header(recvHeader,buffer);

	int offset=sizeof(struct s_header);

	serialize_from_int(buffer,pcb->ID,&offset);

	send(serverSockUMC,buffer,offset,0);

	receive_response(serverSockUMC);

	free(buffer);
	free(sendBuffer);
	free(stringError);
	//veo si es error para cpu o para consola
/*	switch(recvHeader.id){
		case ELIMINAR_PROG:
			//envio error a consola para que no siga esperando su ejecución
			//se pone a libre el estado de la cpu
			sockList->boolEstado=0;
			sockList->pcb=pcb;

			//se recibe pcb y se manda a consola para que termine su ejecución
			//Chequeo que el mensaje no sea para una consola que se cerró
			int offset=sizeof(struct s_header);
			if(buscarConsola(pcb->ID) != NULL){
				serialize_from_header(recvHeader,sendBuffer);
				serialize_from_string(sendBuffer,stringError,&offset);
				send(pcb->ID,sendBuffer,offset,0);
			}

			//se envia finalizar a umc
			offset=sizeof(struct s_header);
			recvHeader.id=FINALIZAR_PROGRAMA;
			recvHeader.size=sizeof(int);
			serialize_from_header(recvHeader,sendBuffer);
			serialize_from_int(sendBuffer,pcb->ID,&offset);
			send(serverSockUMC,sendBuffer,offset,0);
			receive_response(serverSockUMC);

			list_remove(finishedState,0);
			log_info(LOG,"Se busca en readyState si hay ansisop para ejecutar");
			//Como tengo que asignar a esa cpu un ansisop no necesito recorrer la lista de exec
			//asigno y envio pcb
			if(!list_is_empty(readyState)){
				sockList->pcb=list_remove(readyState,0);
				sockList->boolEstado=1;
				log_info(LOG,string_from_format("Se asigna a cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
				send_pcb(sockList->idSocketCpu,sockList->pcb);
			}else{
				log_info(LOG,"No hay ansisop que ejecutar");
			}
			while(!list_is_empty(readyState) && (sockSearchCpu=search_free_execState(socketClient))){
				t_socketCpu* sockAux=list_find(execState,&pcbSearchCpu);
				sockAux->pcb=list_remove(readyState,0);
				sockAux->boolEstado=1;
				log_info(LOG,string_from_format("Se asigna a cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
				send_pcb(sockAux->idSocketCpu,sockAux->pcb);
			}

			break;
		case ELIMINAR_CPU:
			list_remove(finishedState,0);
			sockSearchCpu=sockList->idSocketCpu;
			list_remove_by_condition(execState,&pcbSearchCpu);
			log_info(LOG,string_from_format("Se eliminó un nodo en execState: %d execs",list_size(execState)));
			sockSearchCpu=search_free_execState(socketClient);
			sockList=(t_socketCpu*)list_find(execState,&pcbSearchCpu);
			if(sockList!=NULL){//COMPROBAR QUE DEVUELVE NULL
				sockList->pcb=pcb;
				sockList->boolEstado=1;
				log_info(LOG,string_from_format("Pongo a ejecutar en %d:a %d",sockList->idSocketCpu,pcb->ID));
				send_pcb(sockList->idSocketCpu,pcb);

				while(!list_is_empty(readyState) && (sockSearchCpu=search_free_execState(socketClient))){
					t_socketCpu* sockAux=list_find(execState,&pcbSearchCpu);
					sockAux->pcb=list_remove(readyState,0);
					sockAux->boolEstado=1;
					log_info(LOG,string_from_format("Se asigna a cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
					send_pcb(sockAux->idSocketCpu,sockAux->pcb);
				}
			}else{
				log_info(LOG,"No se encuentra cpu desocupada para atender %d",pcb->ID);
				list_add(readyState,pcb);
			}
			break;
	}

	free(pcb);*/
}

void move_from_exec_to_finish(int socketClient,int socketServer,struct s_header recvHeader, t_pcb* pcb){

	log_info(LOG,string_from_format("Finalizó por alguna razon proceso %d", pcb->ID));
	void* buffer=malloc(sizeof(struct s_header)+sizeof(int));


	//Busco el nodo de exec que pertenece a ese cpu
//	sockSearchCpu=socketClient;
	//t_socketCpu* sockList=(t_socketCpu*)list_find(execState,&pcbSearchCpu);

	//lo apago
	//sockList->boolEstado=0;
	//agrego a finishedState

	//Borro el PCB de Exec.
	delete_from_exec(pcb->ID);

	//Desocupo la CPU
	desocupar_cpu(socketClient);
	/*log_info(LOG,"CPU FINALIZO?");
	if(receive_response(socketClient)!=OK){
		list_remove_by_condition(execState,&pcbSearchCpu);
		log_info(LOG,string_from_format("Se eliminó un nodo en execState: %d execs",list_size(execState)));
		sockSearchCpu=search_free_execState(socketClient);
	}*/

	list_add(finishedState,pcb);
	log_info(LOG,"Se agregó a finishedState el PID %d", pcb->ID);

	//Chequeo que el mensaje no sea para una consola que se cerró
	t_socketConsola* consola = buscarConsola(pcb->ID);
	if(consola != NULL){
		log_info(LOG,"Le envio que finalizó a consola");
		//envio finalizo a consola y a umc
		recvHeader.id = FINALIZO;
		recvHeader.size=0;
		serialize_from_header(recvHeader,buffer);
		send(consola->idSocketConsola,buffer,sizeof(struct s_header),0);

		limpiarConsola(consola->idSocketConsola);
	}

	log_info(LOG,"Le envio que finalizó a UMC");
	//envio a umc
	recvHeader.id=FINALIZAR_PROGRAMA;
	recvHeader.size=sizeof(pcb->ID);
	serialize_from_header(recvHeader,buffer);
	int offset=sizeof(struct s_header);
	serialize_from_int(buffer,pcb->ID,&offset);
	send(socketServer,buffer,offset,0);
	receive_response(socketServer);

	/*
	log_info(LOG,"Se busca en readyState si hay ansisop para ejecutar");
	//Como tengo que asignar a esa cpu un ansisop no necesito recorrer la lista de exec
	//asigno y envio pcb
	if(!list_is_empty(readyState)){
			sockList->pcb=list_remove(readyState,0);
			sockList->boolEstado=1;
			log_info(LOG,string_from_format("Se asigna a cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
			send_pcb(sockList->idSocketCpu,sockList->pcb);
	}else{
		log_info(LOG,"No hay ansisop que ejecutar");
	}
	//recorro el resto de la lista en busqueda de mas cpus para ejecutar
	while(!list_is_empty(readyState) && (sockSearchCpu=search_free_execState(socketClient))){
		t_socketCpu* sockAux=list_find(execState,&pcbSearchCpu);
		sockAux->pcb=list_remove(readyState,0);
		sockAux->boolEstado=1;
		log_info(LOG,string_from_format("Se asigna a cpu %d y ansisop de consola %d para ejecutar",sockList->idSocketCpu,sockList->pcb->ID));
		send_pcb(sockAux->idSocketCpu,sockAux->pcb);
	}
	free(sockList->pcb);
	log_info(LOG,"Se elimina de la lista de fineshedState");
	list_remove(finishedState,0);
	free(pcb);*/

	free(buffer);
}
void finalizar_por_pid(int socketServer, int pid){

	struct s_header sendHeader;

	log_info(LOG,"Le envio que finalizó a UMC");

	//envio a umc
	sendHeader.id=FINALIZAR_PROGRAMA;
	sendHeader.size=sizeof(pid);

	void* buffer = malloc(sizeof(struct s_header) + sizeof(int));

	serialize_from_header(sendHeader,buffer);	int offset=sizeof(struct s_header);
	serialize_from_int(buffer,pid,&offset);

	send(socketServer,buffer,offset,0);

	receive_response(socketServer);

	free(buffer);
}

u_int32_t deserialize_u_int32_t(void* buffer,int* offset){
	u_int32_t number;

	memcpy(&number,buffer+*offset,sizeof(u_int32_t));
	*offset=*offset+sizeof(u_int32_t);

	return number;
}

t_intructions* deserialize_to_instructions(void* buffer,t_size tamanio,int* offset){
	t_intructions* instructions_serialize=malloc(tamanio*sizeof(t_intructions));

	memcpy(instructions_serialize,buffer+*offset,tamanio*sizeof(t_intructions));
	*offset=*offset+tamanio*sizeof(t_intructions);

	return instructions_serialize;
}
