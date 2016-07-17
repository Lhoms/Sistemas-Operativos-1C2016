#include "Nucleo.h"
#include "serialize.h"
#include "PCB.h"
#include "Socket.h"

s_conf get_config(char* path){
	//INICIALIZO VARIABLES Y ESTRUCTURAS
	s_conf config_lcl;
	t_config* config_file;
	int cantConfig = 0;
	int i = 0;
	char* logMessage = string_new();

	log_info(LOG, "Leyendo archivo de configuración");
	
	//IMPORTACON DE ARCHIVO DE CONFIGURACION
	config_file = config_create(path);
	cantConfig = config_keys_amount(config_file);
	if (cantConfig == 0){
		log_error(LOG, "Archivo de configuración vacío");
		return config_lcl; //Para que por lo menos no tire segmentation fault.
	}
	config_lcl.puerto_prog = string_duplicate(config_get_string_value(config_file, "PUERTO_PROG"));
	if (!string_is_empty(config_lcl.puerto_prog)){
		logMessage = string_from_format("PUERTO_PROG: %s", config_lcl.puerto_prog);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró puerto prog");
	}
	config_lcl.ip_umc = string_duplicate(config_get_string_value(config_file, "IP_UMC"));
	if (!string_is_empty(config_lcl.ip_umc)){
		logMessage = string_from_format("IP UMC: %s", config_lcl.ip_umc);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró ip umc");
	}
	config_lcl.puerto_umc = string_duplicate(config_get_string_value(config_file, "PUERTO_UMC"));
	if (!string_is_empty(config_lcl.puerto_umc)){
		logMessage = string_from_format("PUERTO_UMC: %s", config_lcl.puerto_umc);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró puerto umc");
	}
	config_lcl.puerto_cpu = string_duplicate(config_get_string_value(config_file, "PUERTO_CPU"));
	if (!string_is_empty(config_lcl.puerto_cpu)){
		logMessage = string_from_format("PUERTO_CPU: %s", config_lcl.puerto_cpu);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró puerto cpu");
	}
	config_lcl.quantum = config_get_int_value(config_file, "QUANTUM");
	logMessage = string_from_format("QUANTUM: %d", config_lcl.quantum);
	log_info(LOG, logMessage);
	/*if (!string_is_empty(config.quantum)){
		logMessage = string_from_format("QUANTUM: %s", config.quantum);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró quantum");
	}*/
	config_lcl.quantum_sleep = config_get_int_value(config_file, "QUANTUM_SLEEP");
	logMessage = string_from_format("QUANTUM_SLEEP: %d", config_lcl.quantum_sleep);
	log_info(LOG, logMessage);
	/*if (!string_is_empty(config.quantum_sleep)){
		logMessage = string_from_format("QUANTUM_SLEEP: %s", config.quantum_sleep);
		log_info(LOG, logMessage);
	}else{
		log_error(LOG, "No se encontró quantum sleep");
	}*/
	config_lcl.sem_id = config_get_array_value(config_file, "SEM_ID");
	config_lcl.sem_val = config_get_array_value(config_file, "SEM_INIT");
	if (!string_is_empty(config_lcl.sem_id[0])){
		log_info(LOG, "SEMAFOROS");

		for (i=0; config_lcl.sem_id[i] != '\0';i++){
			log_info(LOG, config_lcl.sem_id[i]);
			log_info(LOG, config_lcl.sem_val[i]);
		}
	}else{
		log_error(LOG, "No se encontraron semáforos");
	}
	config_lcl.io_id = config_get_array_value(config_file, "IO_IDS");
	config_lcl.io_sleep = config_get_array_value(config_file, "IO_SLEEP");
	if (!string_is_empty(config_lcl.io_id[0])){
		log_info(LOG, "IOs");

		for (i=0; config_lcl.io_id[i] != '\0';i++){
			log_info(LOG, config_lcl.io_id[i]);
			log_info(LOG, config_lcl.io_sleep[i]);
		}
	}else{
		log_error(LOG, "No se encontraron dispositivos de entrada/salida");
	}

	config_lcl.shared_vars = config_get_array_value(config_file, "SHARED_VARS");
	if (!string_is_empty(config_lcl.shared_vars[0])){
		log_info(LOG, "SHARED VARS");

		for (i=0; config_lcl.shared_vars[i] != '\0';i++){
			log_info(LOG, config_lcl.shared_vars[i]);
		}
	}else{
		log_warning(LOG, "No se encontraron variables compartidas");
	}

	config_lcl.stack_size = config_get_int_value(config_file, "STACK_SIZE");
	/*if (!string_is_empty(config.stack_size)){*/
		logMessage = string_from_format("STACK_SIZE: %d", config_lcl.stack_size);
		log_info(LOG, logMessage);
	/*}else{
		log_error(LOG, "No se encontró stack size");
	*/

	config_destroy(config_file);
	return config_lcl;
}

void create_list_ios(s_conf* conf){

	int i;

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	listIO = list_create();

	for (i=0; conf->io_id[i] != '\0';i++){
		s_io* entry = malloc(sizeof(s_io) + string_length(conf->io_id[i]));

		entry->idIO = string_duplicate(conf->io_id[i]);
		entry->sleep = atoi(conf->io_sleep[i]);
		entry->queue = queue_create();

		//Lo inicializo bloqueado para que no empiece a buscar con la lista vacía.
		//entry->sem = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(&(entry->sem), NULL);
		pthread_mutex_lock(&(entry->sem));

		list_add(listIO, entry);

		//Creo un hilo por cada IO;
		pthread_t threads;
		pthread_create(&threads, &attr, &threads_IO, (void*)conf->io_id[i]);
	}
}

void* planificadorRR(void* arg){

	bool searchFreeCpu(void* parametro){
		t_socketCpu* act = (t_socketCpu*) parametro;
		return act->boolEstado == 0;
	}

	while(1){
		//Si ya está bloqueado, es porque no hay nada en la queue
		pthread_mutex_lock(&mutex_ready);

		t_pcb* pcb = list_remove(readyState, 0);
		log_info(LOG, "Se toma el PCB: %d de la lista de Ready para enviar", pcb->ID);

		if(buscarConsola(pcb->ID) != NULL){
			pthread_mutex_lock(&mutex_cpu);

			log_info(LOG, "Hay una CPU libre, se envía PCB");
			t_socketCpu* freeCpu = list_find(listaCPUs, searchFreeCpu);

			pthread_mutex_lock(&monitorConfigMutex);

				pcb->quantum = config.quantum;
				pcb->quantum_sleep = config.quantum_sleep;

			pthread_mutex_unlock(&monitorConfigMutex);

			send_pcb(freeCpu->idSocketCpu, pcb);

			//Paso PCB a lista de Exec
			log_info(LOG, "Paso PCB: %d a la lista de Exec", pcb->ID);
			list_add(execState, pcb);

			//Marco CPU como ocupada
			log_info(LOG, "Se marca CPU %d como ocupada", freeCpu->idSocketCpu);
			freeCpu->boolEstado = 1;
			freeCpu->pid        = pcb->ID;
		}else{
			log_info(LOG, "El PCB de PID: %d pertenece a un proceso no activo, se elimina", pcb->ID);
			free(pcb);
		}

		if(list_size(readyState) != 0){
			pthread_mutex_unlock(&mutex_ready);
		}

		if(list_any_satisfy(listaCPUs, searchFreeCpu) == true){
			pthread_mutex_unlock(&mutex_cpu);
		}

	}
}

s_io* buscarIO(char* id){

	bool search(void* parametro){
		s_io* entry = (s_io*) parametro;
		return string_equals_ignore_case(entry->idIO, id);
	}

	s_io* entry_io = list_find(listIO, search);

	return entry_io;
}

void agregarPCBaIO(s_io* Io, t_pcb* pcb, int rafagas){

	int offset = 0;

	//Serializo Ráfagas + PCB
	void* bufferIO = serializarPCB(pcb, &offset);

	bufferIO = realloc(bufferIO, offset + sizeof(int));

	serialize_from_int(bufferIO, rafagas, &offset);

	queue_push(Io->queue, bufferIO);

	pthread_mutex_unlock(&(Io->sem));
}

void* threads_IO(void* arg){

	char* id = (char*) arg;

	char* log_name = string_duplicate(id);
	string_append(&log_name, ".log");

	t_log* log_lcl = log_create(log_name,"Nucleo.c",0,1);
	log_info(log_lcl, "Inicio de LOG para dispositivo %s", id);

	while(1){

		s_io* IOinvolucrado = buscarIO(id);

		//Si ya está bloqueado, es porque no hay nada en la queue
		pthread_mutex_lock(&(IOinvolucrado->sem));
		//pthread_mutex_lock(IOinvolucrado->sem);

		log_info(log_lcl, "Se recibe pedido al dispositivo");
		void* bufferIO = queue_pop(IOinvolucrado->queue);

		log_info(log_lcl, "Se deserializa pedido");
		t_pcb* pcb = malloc(sizeof(t_pcb));
		deserializarPCB(bufferIO, pcb);

		log_info(log_lcl, "Se recibió el PCB del PID: %d", pcb->ID);

		if(buscarConsola(pcb->ID) != NULL){

			int rafagas;
			//Calculo tamaño del pcb para saber el offset donde está el int de ráfagas.
			int offset = 11*sizeof(int) + (8 * pcb->instrucciones_size) + pcb->etiquetas_size + pcb->indice_stack_size;
			//Deserializo ráfagas
			deserialize_to_int(bufferIO, &rafagas, &offset);

			log_info(log_lcl, "Se recibieron las ráfagas: %d", rafagas);

			//Ahora sí, hago sleep.
			usleep(rafagas * IOinvolucrado->sleep * 1000);

			log_info(log_lcl, "Se pasa PCB a cola de Ready");
			//Paso a ready el pcb
			list_add(readyState, pcb);
			pthread_mutex_unlock(&mutex_ready);

		}else{
			log_info(log_lcl, "El PCB de proceso %d ya no se encuentra activo, no se realiza retardo de IO", pcb->ID);
			free(pcb);
		}
		//free(pcb);
		free(bufferIO);

		if(queue_size(IOinvolucrado->queue) != 0){
			pthread_mutex_unlock(&(IOinvolucrado->sem));
		}
	}
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

void agregarCpu(int socket){

	t_socketCpu* cpu=malloc(sizeof(t_socketCpu));
	//t_socketCpu* paraLog;

	cpu->idSocketCpu = socket;
	cpu->pid = 0;
	cpu->boolEstado=0;

	list_add(listaCPUs,cpu);
	pthread_mutex_unlock(&mutex_cpu);

	//paraLog=list_get(execState,list_size(execState)-1);
	//log_info(LOG,string_from_format("Se agrega a EXEC cpu:%d",paraLog->idSocketCpu));
	log_info(LOG,string_from_format("Se conecta y se agrega a lista CPU: %d",socket));
	log_info(LOG,string_from_format("Hay conectadas %d cpus",list_size(listaCPUs)));
	/*if(!list_is_empty(readyState)){
		cpu->pcb=list_remove(readyState,0);
		cpu->boolEstado=1;
		send_pcb(socket,cpu->pcb);
		log_info(LOG,string_from_format("Se pone en ejecución ansisop:%d",cpu->pcb->ID));
	}*/
}

void sacarCPU(int socket){
        bool searchFreeCpu(void* parametro){
		t_socketCpu* act = (t_socketCpu*) parametro;
		return act->boolEstado == 0;
	}

	bool search(void* parametro){
			t_socketCpu* act = (t_socketCpu*) parametro;
			return act->idSocketCpu == socket;
	}

	t_socketCpu* cpu = list_remove_by_condition(listaCPUs, search);

	if(cpu!=NULL){
	   log_info(LOG, "Se elimina CPU %d de la lista", cpu->idSocketCpu);
	   int libre = cpu->boolEstado;
           free(cpu);

	   //Si no hay más CPU en la lista, bloqueo.
	   if(libre == 0 && list_any_satisfy(listaCPUs, searchFreeCpu) == false){
		   pthread_mutex_lock(&mutex_cpu);
	   }
	}
}

t_socketCpu* buscarCPU(int socket){

	bool search(void* parametro){
		t_socketCpu* act = (t_socketCpu*) parametro;
		return act->idSocketCpu == socket;
	}

	return list_find(listaCPUs, search);
}
void serialize_from_protocol(struct s_protocol protocol, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, protocol.id, &offset);
	serialize_from_int(buffer, protocol.size, &offset);
	serialize_from_string(buffer, protocol.payload, &offset);

}
void agregarConsola(int socket){

	t_socketConsola* Consola = malloc(sizeof(t_socketConsola));

	Consola->idSocketConsola= socket;
	Consola->pid_activo	= -1;
	list_add(listaConsolas, Consola);

}

void sacarConsola(int socket){

	bool search(void* parametro){
		t_socketConsola* cons = (t_socketConsola*) parametro;
		return cons->idSocketConsola == socket;
	}

	log_info(LOG, "Se elimina consola %d", socket);
	list_remove_and_destroy_by_condition(listaConsolas, search, free);

}

void limpiarConsola(int socket){

	bool search(void* parametro){
		t_socketConsola* cons = (t_socketConsola*) parametro;
		return cons->idSocketConsola == socket;
	}

	t_socketConsola* consola = list_find(listaConsolas, search);

	if(consola!=NULL){
		log_info(LOG, "Se desasigna PID %d de Consola %d", consola->pid_activo, consola->idSocketConsola);
		consola->pid_activo = -1;
	}

}

t_socketConsola* buscarConsola(int pid){

	bool search(void* parametro){
		t_socketConsola* cons = (t_socketConsola*) parametro;
		return cons->pid_activo == pid;
	}

	t_socketConsola* consola = list_find(listaConsolas, search);

	return consola;

}

t_socketConsola* buscarConsola_porSocket(int sock){

	bool search(void* parametro){
		t_socketConsola* cons = (t_socketConsola*) parametro;
		return cons->idSocketConsola == sock;
	}

	t_socketConsola* consola = list_find(listaConsolas, search);

	return consola;

}


void serialize_from_header(struct s_header header, void* buffer){
	int offset = 0;

	serialize_from_int(buffer, header.id, &offset);
	serialize_from_int(buffer, header.size, &offset);
}

int realize_handshake(int socket){
	int ret;

	struct s_header sendHandshake;
	struct s_header recvHandshake;

	sendHandshake.id = SALUDO_NUCLEO;
	sendHandshake.size = 0;

	void* senderBufferHandshake = malloc(sizeof(struct s_header));

	serialize_from_header(sendHandshake, senderBufferHandshake);

	int result1 = send(socket, senderBufferHandshake, sizeof(struct s_header), 0);
	if(result1 == -1){
		log_error(LOG, "UMC no está conectado");
	}else{
		log_info(LOG, "Se recibe mensaje");
      	result1 = recv(socket, &recvHandshake, sizeof(struct s_header), MSG_WAITALL); //recv bloqueante
		if(result1 == -1)
		{
			log_error(LOG, "Error: no se recibio el mensaje de UMC");
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

int send_quantum_size_to_socket(int socket, int tam_quantum){

	int ret;
	int offset = 0;
	int status = 0;
	int result1;

	struct s_header senderHeader;
	struct s_header recvHeader;

	senderHeader.id = TAMANIO_QUANTUM;
	senderHeader.size = sizeof(int);

	void* sendBuffer = malloc(sizeof(struct s_header) + sizeof(int));
	serialize_from_int(sendBuffer, senderHeader.id, &offset);
	serialize_from_int(sendBuffer, senderHeader.size, &offset);
	serialize_from_int(sendBuffer, tam_quantum, &offset);

	status = send(socket, sendBuffer, sizeof(struct s_header) + sizeof(int), 0);
	if (status == ERROR){
		ret = ERROR;
	}else{
		log_info(LOG,string_from_format("Se envió TAMANIO_QUANTUM:%d",tam_quantum));
		      	result1 = recv(socket, &recvHeader, sizeof(struct s_header), MSG_WAITALL); //recv bloqueante
				if(result1 == -1)
				{
					log_error(LOG, "Error: no se recibió el mensaje de CPU");
				}else{
					if (recvHeader.id == OK){
						log_info(LOG,"Recibo OK");
					}else{
						log_error(LOG,"Recibo FALLO");
						ret = FALLO;
					}
				}
	}

	free(sendBuffer);
	return ret;
}

int send_quantum_sleep_to_socket(int socket, int tam_qSleep){

	int ret;
	int offset = 0;
	int status = 0;
	int result1;

	struct s_header senderHeader;
	struct s_header recvHeader;

	senderHeader.id = TAMANIO_QUANTUMSLEEP;
	senderHeader.size = sizeof(int);

	void* sendBuffer = malloc(sizeof(struct s_header) + sizeof(int));
	serialize_from_int(sendBuffer, senderHeader.id, &offset);
	serialize_from_int(sendBuffer, senderHeader.size, &offset);
	serialize_from_int(sendBuffer, tam_qSleep, &offset);

	status = send(socket, sendBuffer, sizeof(struct s_header) + sizeof(int), 0);
	if (status == ERROR){
		ret = ERROR;
	}else{
		log_info(LOG,string_from_format("Se envió QUANTUM_SLEEP:%d",tam_qSleep));
		result1 = recv(socket, &recvHeader, sizeof(struct s_header), MSG_WAITALL); //recv bloqueante
		if(result1 == -1)
			{
			log_error(LOG, "Error: no se recibió el mensaje de CPU");
		}else{
			if (recvHeader.id == OK){
				log_info(LOG, "Recibo OK");
				ret = OK;
			}else{
				log_error(LOG, "Recibo FALLO");
				ret = FALLO;
			}
		}
	}

	free(sendBuffer);
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

int receive_tamanio_pagina(int socket, int* tamanio_pagina){
	int ret;

	struct s_header recvBuffer;

      	int status = recv(socket, &recvBuffer, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);return ERROR;}
		else if(status ==  0) {log_error(LOG, "Error: se cayo umc");	    close(socket);return ERROR;}

		else{
			if (recvBuffer.id == TAMANIO_PAGINA){
				status = recv(socket, tamanio_pagina, recvBuffer.size, 0);
				log_info(LOG,string_from_format("Recibo TAMANIO PAGINA DE UMC SOCKET SERVER %d:%d",socket,*tamanio_pagina));
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

void* serialize_init_program(int pid, int cant_pag , char* codigo){
	int offset = 0;
	void* buffer = malloc (sizeof(int) + sizeof(int) + strlen(codigo) + 1 );

	serialize_from_int(buffer, pid, &offset);
	serialize_from_int(buffer, cant_pag, &offset);
	memcpy(buffer+offset, codigo, strlen(codigo)+1);

	return buffer;
}

int receive_response(int socket){
	int ret;

	struct s_header recvBuffer;

      	int status = recv(socket, &recvBuffer, sizeof(struct s_header), 0);
		if(status == -1) {log_error(LOG, "Error: no se respuesta");close(socket);return ERROR;}
		else if(status ==  0) {log_error(LOG, "Error: se cayo socket");	    close(socket);return ERROR;}

		else{
			if (recvBuffer.id == OK){
				log_info(LOG, "Se recibe respuesta");
				ret = OK;

			}else{
				log_error(LOG, "Falló");
				ret = FALLO;
			}
		}

	return ret;

}

struct s_protocol receive_ansisop(int socket,int tamanio_ansisop){

	struct s_protocol protocol_ansisop;
	protocol_ansisop.size=tamanio_ansisop;
	int offset=0;
	int status=0;


	log_info(LOG,string_from_format("Se recibe tamanio del string de ansisop: %d",protocol_ansisop.size));

	//RECIBO STRING DE ANSISOP
	void* recvBuffer=malloc(protocol_ansisop.size);
	status = recv(socket,recvBuffer,protocol_ansisop.size,0);
		if(status == -1) {
			log_error(LOG, "Error: no se recibe correctamente el ansisop");
			protocol_ansisop.id=ERROR;
			close(socket);return protocol_ansisop;
		}else
			if(status ==  0) {
				log_error(LOG, "Error: se cayo socket");
				protocol_ansisop.id=ERROR;
				close(socket);return protocol_ansisop;
		}else{
			offset=0;
			protocol_ansisop.payload=malloc(protocol_ansisop.size);
			deserialize_to_string(recvBuffer,protocol_ansisop.payload,protocol_ansisop.size,&offset);
			log_info(LOG,string_from_format("Se recibe ansisop:\n%s",protocol_ansisop.payload));
			protocol_ansisop.id=INICIAR_PROGRAMA;
		}
	free(recvBuffer);
	return protocol_ansisop;
}

int send_arrays_string_to_socket(int socket,char** array,int headerId){
	int i=0;
	int status,ret,result1;
	char* id;

	struct s_header recvHeader;
	struct s_protocol sendProtocolBuffer;

	sendProtocolBuffer.size=0;
	//Se inicializa el campo de string para cargar ids concatenados
	sendProtocolBuffer.payload=string_new();

	//Se concatenan los ids
	for(;array[i]!=NULL;i++){
		id=string_duplicate(array[i]);
		sendProtocolBuffer.size=sendProtocolBuffer.size+1+strlen(id);
		string_append(&(sendProtocolBuffer.payload),id);
		string_append(&(sendProtocolBuffer.payload),",");
	}
	sendProtocolBuffer.id=headerId;
	sendProtocolBuffer.payload[sendProtocolBuffer.size-1]='\0';
	void *sendBuffer=malloc(sendProtocolBuffer.size+sizeof(int)*2);
	serialize_from_protocol(sendProtocolBuffer, sendBuffer);

	status = send(socket, sendBuffer, sizeof(int)*2+sendProtocolBuffer.size, 0);
		if (status == ERROR){
			ret = ERROR;
		}else{
			log_info(LOG,string_from_format("Se envia array de tamanio: %d:\nString enviado: %s",headerId,sendProtocolBuffer.payload));
			result1 = recv(socket, &recvHeader, sizeof(struct s_header), MSG_WAITALL); //recv bloqueante
			if(result1 == -1)
				{
				log_error(LOG, "Error: no se recibio el OK");
				return FALLO;
			}else{
				if (recvHeader.id == OK){
					log_info(LOG, "Se envio correctamente el array");
					ret = OK;
				}else{
					log_error(LOG, "Envio de array falló");
					ret = FALLO;
				}
			}
		}
	free(sendBuffer);
	return ret;
}

char* receive_string(int socket,int size){
	//Contemplo que envia string con el /0 incluido y contabilizado
		char* string = malloc(size);

		recv(socket,string,size,0);
		log_info(LOG,string_from_format("Cargo el string: %s",string));

		return string;
}

int search_secuencial_index_string(char* searchString,char** listString){
	int i=0;

	for(;strcmp(listString[i],searchString);i++);

	if(listString[i]==NULL){
		log_info(LOG,string_from_format("No se encuentra: %s",searchString));
		i=-1;
	}else{
		log_info(LOG,string_from_format("Se encuentra:%s index:%d",listString[i],i));
	}
	return i;
}

/*void* receive_io(int socket, int tamanioIdIO,char** IO_ID){
	int indexIO, rafagasIO;
	void* buffer = malloc(sizeof(int)*2);

	indexIO=search_secuencial_index_string(receive_string(socket,tamanioIdIO),IO_ID);
	if(indexIO>=0){
		if((rafagasIO = receive_int_positive(socket))>=0){
			int offset=0;
			serialize_from_int(buffer,indexIO,&offset);
			serialize_from_int(buffer,rafagasIO,&offset);
		}
	}//hacer algo por fallo

	if(move_to_ioList(socket,indexIO)==0){
		rafagasIO=1;//Hacer algo por fallo
	}
return buffer;
}*/

/*int move_to_ioList(int socket,int tipoIo){
	//meto en una variable global el socket del cpu que quiero buscar en exec
	int i;
	sockSearchCpu=socket;
	log_info(LOG,string_from_format("Comienza busqueda de cpu %d en execState para pasarlo a lista de Dispositivos",socket));

	t_socketCpu* sockList=(t_socketCpu*)list_find(execState,&pcbSearchCpu);
	if(sockList!=NULL){
		log_info(LOG,string_from_format("Se encuentra nodo se cambia estado y se pasa a lista de dispositivos",socket));
		sockList->boolEstado=0;
		list_add(inputOutputState[tipoIo],sockList->pcb);
		i=1;
	}else{
		//hacer algo por este error
		log_info(LOG,string_from_format("No se encuentra nodo de %d",socket));
		i=0;
	}
	return i;
}*/

void* serverIO(void* buffer){
	printf("Thread IO served.\n");
	int offset=0;
	int tipoIO;
	deserialize_to_int(buffer,&tipoIO,&offset);
	int rafagasCpu;
	deserialize_to_int(buffer,&rafagasCpu,&offset);
	sem_post(&bufferReadThread);

	do{
		offset=0;
		pthread_mutex_lock(&serverIOM[tipoIO]);
		usleep((atoi(config.io_sleep[tipoIO]))*rafagasCpu*1000 );
		list_add(readyState,list_remove(inputOutputState[tipoIO],0));
		pthread_mutex_unlock(&serverIOM[tipoIO]);
	}while(!list_is_empty(inputOutputState[tipoIO]));
	return 0;
}

void receive_imprimir(int socket,int stringSize){

	//reservo el buffer de envio
	void* sendBuffer= malloc(stringSize+sizeof(struct s_header));
	char* printString=malloc(stringSize);

	recv(socket,printString,stringSize,0);

	struct s_header sendHeader;
	sendHeader.id=IMPRIMIR;
	sendHeader.size=stringSize;

	serialize_from_header(sendHeader,sendBuffer);
	int offset=sizeof(struct s_header);

	serialize_from_string(sendBuffer,printString,&offset);


	//busco el pid para enviar por socket en execState
	int pid_cpu = buscar_PID_activo_de_CPU(socket);
	int socketConsola = obtener_consola_de_pid(pid_cpu);

	log_info(LOG,string_from_format("Recibo de cpu %d para imprimir en consola %d(por PID %d): %s",socket, socketConsola, pid_cpu, printString));

	if(socketConsola != -1){
		int status = send(socketConsola,sendBuffer, sizeof(struct s_header)+stringSize, 0);
		if (status == ERROR){
			log_error(LOG,"Error:Envio para imprimir falló");
		}else{
			log_info(LOG,"Se envio imprimir a consola");
		}
	}else{
		log_error(LOG, "Error al encontrar PID activo para CPU: %d", socket);
	}

	free(sendBuffer);
	free(printString);
}

void imprimir_en_consola(int socketConsola, char* printString){

	//reservo el buffer de envio
	int stringSize = strlen(printString) + 1;
	void* sendBuffer= malloc(stringSize+sizeof(struct s_header));

	struct s_header sendHeader;
	sendHeader.id=IMPRIMIR;
	sendHeader.size=stringSize;

	serialize_from_header(sendHeader,sendBuffer); int offset=sizeof(struct s_header);

	memcpy(sendBuffer+offset, printString, stringSize);


	log_info(LOG,string_from_format("Envio para imprimir en consola %d: %s", socketConsola, printString));

		//Primero verifico que esa consola no se haya caído.
		if(buscarConsola_porSocket(socketConsola) != NULL){
			int status = send(socketConsola,sendBuffer, sizeof(struct s_header)+stringSize, 0);
			if (status == ERROR){
				log_error(LOG,"Error:Envio para imprimir falló");
			}else{
				log_info(LOG,"Se envio imprimir a consola");
			}
		}

	free(sendBuffer);
}

int buscar_PID_activo_de_CPU(int sock){

	bool search(void* parametro){
		t_socketCpu* act = (t_socketCpu*) parametro;
		return act->idSocketCpu == sock;
	}

	t_socketCpu* cpu = list_find(listaCPUs, search);

	if(cpu!=NULL){
		return cpu->pid;
	}else{
		return -1;
	}
}

void* monitorConfig(void* fileConfig){

do{
	int length, i = 0;
	int fdInotify;
	int wd;
	char bufferInotify[BUF_LEN];

	char* file = (char*)fileConfig;

	fdInotify = inotify_init();
	if (fdInotify< 0) {
		//log_error(LOG, "Error al iniciar Inotify");
	}else{
		//log_info(LOG, "Inotify iniciado");
	}
	wd = inotify_add_watch(fdInotify, ".",//file,
			IN_MODIFY | IN_CREATE | IN_DELETE);

	length = read(fdInotify, bufferInotify, BUF_LEN);
	if (length < 0) {
		log_error(LOG, "Error al leer cambios en fichero");
	}else{
		//log_info(LOG, "Se realizó una modificación en el fichero");
	}
	while (i < length) {
		struct inotify_event *event =(struct inotify_event *) &bufferInotify[i];
		if (event->len) {
			//Verifico que la modificación sea sobre el archivo que estoy monitoreando
			if(strstr(file,event->name)){
				if (event->mask & IN_CREATE) {
					log_info(LOG, "Se borró archivo de configuración: %s", event->name);
					} else if (event->mask & IN_DELETE) {
						log_info(LOG, "Se borró archivo de configuración: %s", event->name);
					} else if (event->mask & IN_MODIFY) {
						log_info(LOG, "Se modificó archivo de configuración: %s", event->name);
				//		if(strstr(event->name,"config.properties")){
							pthread_mutex_lock(&monitorConfigMutex);
								usleep(10000); //Le doy 10 milisegundos a que el archivo "se vuelva a crear"
								config = get_config(file);
							pthread_mutex_unlock(&monitorConfigMutex);
					//	}
					}
				}
		}
		i += EVENT_SIZE + event->len;
	}
	inotify_rm_watch(fdInotify,wd);
	close(fdInotify);
}while(1);


}

void completar_diccionario_semaforos(){

	int i = 0;
	t_sem* estructura;

	while( config.sem_id[i] != NULL ){

	estructura = malloc(sizeof(t_sem));

	estructura->valor = atoi(config.sem_val[i]);
	estructura->queue = queue_create();

	log_info(LOG, "inicializa semaforo %s con %d", config.sem_id[i], estructura->valor);
	dictionary_put(semaforos, config.sem_id[i], estructura);

	i++;
	}
}

void completar_diccionario_shared_vars(){

	int i = 0;
	t_shar_var* estructura;

	while( config.shared_vars [i] != NULL ){

	estructura = malloc(sizeof(t_shar_var));

	estructura->valor = 0;

	log_info(LOG, "inicializa variable %s con %d", config.shared_vars[i], estructura->valor);
	dictionary_put(shared_vars, string_duplicate(config.shared_vars[i]+sizeof(char)), estructura);

	i++;

	}

}

t_sem* get_semaforo(char* nombre){

	t_sem* estructura = dictionary_get(semaforos, nombre);
	log_info(LOG, "se obtiene valor sobre semáforo %s, valor actual: %d", nombre, estructura->valor);
	return estructura;

}
void wait_semaforo(char* nombre, t_sem* semaforo){ //char* nombre){

	/*t_sem* estructura = dictionary_get(semaforos, nombre);
	estructura->valor--;*/
	semaforo->valor--;

	log_info(LOG, "se hace wait sobre %s, valor actual: %d", nombre, semaforo->valor);

}

void signal_semaforo(char* nombre){

	t_sem* estructura = dictionary_get(semaforos, nombre);
	estructura->valor++;

	if(queue_size(estructura->queue) > 0){
		t_pcb* pcb = queue_pop(estructura->queue);

		if(buscarConsola(pcb->ID) != NULL){
			//Paso a ready el pcb
			list_add(readyState, pcb);
			pthread_mutex_unlock(&mutex_ready);
		}else{
			free(pcb);
			bool flag = false;
			//Voy sacando hasta encontrar uno que no se haya caído
			while((queue_size(estructura->queue) > 0) && flag == false){
				pcb = queue_pop(estructura->queue);
				if(buscarConsola(pcb->ID) != NULL){
					flag = true;
					//Paso a ready el pcb
					list_add(readyState, pcb);
					pthread_mutex_unlock(&mutex_ready);
				}else{
					free(pcb);
				}
			}
		}
		//Vuelvo atrás si saqué algo que sirva.
		if(pcb!=NULL){
			estructura->valor--;
		}
	}
	log_info(LOG, "se hace signal sobre %s, valor actual: %d", nombre, estructura->valor);
}

int recibir_accion_semaforo(int sock, char* key, t_sem* semaforo){

	int ret;
	struct s_header recvHeader;

	int status = recv(sock, &recvHeader, sizeof(struct s_header), MSG_WAITALL);
	if(status == ERROR){
		ret = status;
	}else{
		socket_ok_response(sock);

		switch(recvHeader.id){
			case SEM_QUEUE:

				log_info(LOG, "Se solicita encolamiento en semáforo %s", key);
				//Recibo el PCB
				t_pcb* pcbRecv=malloc(sizeof(t_pcb));
				receive_pcb(sock,pcbRecv);
				log_info(LOG, "Se recibió el PCB %d para encolar", pcbRecv->ID);

        	  	recv(sock, &recvHeader, sizeof(struct s_header), 0);
        	  		if (recvHeader.id == CPU_FIN){
        	  			sacarCPU(sock);
        	  			log_info(LOG,"CPU %d deja de ejecutar", sock);
        	  		}

        	  	delete_from_exec(pcbRecv->ID);

				queue_push(semaforo->queue, pcbRecv);
				log_info(LOG, "Se encoló perfectamente el PCB");

				desocupar_cpu(sock);
				break;
			case SEM_WAIT:
				wait_semaforo(key, semaforo);
				break;
		}
		//socket_ok_response(sock);
		ret = OK;
	}

	return ret;
}

void manejar_caida_cpu(int sock, int sockUMC){

	int pid_activo = buscar_PID_activo_de_CPU(sock);
	int value = obtener_consola_de_pid(pid_activo);

	if (value > 0){

		finalizar_por_pid(sockUMC, pid_activo);

		delete_from_exec(pid_activo);
		log_info(LOG, "Se elimino programa %d", pid_activo);

		imprimir_en_consola(value, "Se cayo la cpu que ejecutaba el ansisop.\nIntente nuevamente.\n");
		limpiarConsola(value);
		log_info(LOG, "Se limpia consola %d en lista consolas", value);
	}

	sacarCPU(sock);
	log_info(LOG, "Se cayó la CPU %d, se saca de lista cpus", sock);

}

void set_shared_var(char* nombre, int valor){

	t_shar_var* estructura = dictionary_get(shared_vars, nombre);

	estructura->valor = valor;

	log_info(LOG, "se setea %s con %d", nombre, estructura->valor);
}

int get_shared_var(char* nombre){

	t_shar_var* estructura = dictionary_get(shared_vars, nombre);

	log_info(LOG, "se obtuvo valor %d de %s", estructura->valor, nombre);
	return estructura->valor;

}

void send_value_to_socket(int socket, int id, int value){

	struct s_header senderHeader;

	senderHeader.id = id;
	senderHeader.size = sizeof(int);

	void* sendBuffer = malloc(sizeof(struct s_header) + sizeof(int));
	serialize_from_header(senderHeader, sendBuffer);
	memcpy(sendBuffer+sizeof(struct s_header), &value, sizeof(int));

	int status = send(socket, sendBuffer, sizeof(struct s_header) + sizeof(int), 0);

	if (status == -1) log_info(LOG, "fallo enviando valor");
	else if (status == 0) log_info(LOG, "fallo enviando valor, se cayo el socket");
	else
		log_info(LOG, "se envio perfectamente el valor %d al socket %d", value, socket);

	free(sendBuffer);
}




