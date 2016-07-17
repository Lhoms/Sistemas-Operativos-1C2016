/*
 * PCB.h
 *
 *  Created on: 24/5/2016
 *      Author: Lajew
 */

#ifndef SRC_PCB_H_
#define SRC_PCB_H_

#include "CPU.h"
#include <parser/parser.h>
#include <parser/metadata_program.h>

typedef struct PCB{

		u_int32_t		ID; 		 			  // ID de PCB
		t_size			quantum;				  // Tamaño del quantum
		t_size		    quantum_sleep;			  // Tiempo de sleep
		t_size        	PC; 		 		  	  // Program Counter
		t_size	 		cant_Paginas; 		   	  // Cantidad de paginas

		t_size 		    instrucciones_size;		  // Cantidad de instrucciones
		t_intructions* 	p_instrucciones;       	  // Puntero al inicio del indice de codigo

		t_size			etiquetas_size;			  // Tamaño del mapa serializado de etiquetas
		char*			p_etiquetas;			  // Puntero a la serializacion de las etiquetas

		t_puntero 		p_stack;	 			  // Puntero al inicio del stack
		t_puntero 		p_stackActual; 			  // Puntero indice del stack

		u_int32_t		indice_contexto;		  // Numero de contexto donde esta (Main 0)
		u_int32_t		indice_stack_size;		  // Tamaño del indice de stack
		void*			indice_stack;			  // Estructura indice del stack.
												  //// En orden: tamaños[4] + retVar[3 ]+ retPos + argDicc + varDicc. (si tamaño = 0, no existe)

	} __attribute__((packed)) t_pcb;

typedef struct INDICE_STACK{

			int   retVar; 		// Posicion de memoria donde se almacenara el resultado  [ Pag + Offset + Size ]
			int   retPos;	 	// PC de retorno
	t_dictionary* argDicc;		// Lista de argumentos recibidos por la funcion
	t_dictionary* varDicc;	 	// Variables locales a la funcion

	} __attribute__((packed)) t_indice_stack;



	void*  serializarPCB(t_pcb* pcb,int* pcb_size);
	void   deserializarPCB(void* serializado, t_pcb* pcb);

	void   initialize_pcb(t_pcb* pcb);

	void   serialize_from_t_size(void* buffer, size_t value, int* offset);
	void   deserialize_to_t_size(void* buffer, size_t* value, int* offset);

	void   send_pcb(int socket, t_pcb* pcb, t_log* LOG);
	t_pcb* receive_pcb(int socket, t_pcb* pcb, t_log* LOG);

#endif /* SRC_PCB_H_ */
