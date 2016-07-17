/*
 * PCB.c
 *
 *  Created on: 24/5/2016
 *      Author: utnso
 */

#include "Protocolo.h"
#include "PCB.h"
#include "serialize.h"

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

void send_pcb(int socket, t_pcb* pcb, t_log* LOG){

	int pcb_size;
	void* senderBuffer;

	void* pcbBuffer = serializarPCB(pcb, &pcb_size);

	struct s_header senderPcb;
	senderPcb.id = PCB_ESTRUCTURA;
	senderPcb.size = pcb_size;

	senderBuffer = malloc(sizeof(struct s_header) + pcb_size);

	serialize_from_header(senderPcb, senderBuffer);

	int offset = sizeof(struct s_header);
	memcpy(senderBuffer + offset , pcbBuffer , pcb_size);

	log_info(LOG, "Se envia pcb");
	int status = send(socket, senderBuffer, sizeof(struct s_header) + pcb_size, 0);

	if(status == -1) {log_error(LOG, "Error: no se envio pcb");        close(socket);}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket"); close(socket);}
	else log_info(LOG, "Pcb enviado correctamente");
}

t_pcb* receive_pcb(int socket, t_pcb* pcb, t_log* LOG){

	int 	 status;
	void*    receiverBuffer;
	struct 	 s_header receiverPcb;
  	initialize_pcb(pcb);

	//log_info(LOG, "Se espera pcb");

  	status = recv(socket, &receiverPcb, sizeof(struct s_header), 0);
	if(status == -1) {log_error(LOG, "Error: no se recibio pcb");		close(socket);}
	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(socket);}

	else{
		if (receiverPcb.id == PCB_ESTRUCTURA){
			log_info(LOG, "Se recibio mensaje");
			receiverBuffer = malloc(receiverPcb.size);

			status = recv(socket, receiverBuffer, receiverPcb.size, 0);
		  	if(status == -1) {log_error(LOG, "Error: no se recibio el mensaje");close(socket);}
		  	else if(status ==  0) {log_error(LOG, "Error: se cayo el socket");	close(socket);}

		  	deserializarPCB(receiverBuffer, pcb);

		  	free(receiverBuffer);

		  	if (pcb->ID == 0) return NULL;
		  	else return pcb;
		}
	}

	//log_error(LOG, "Error, se esperaba pcb");
	return NULL;
}


