/*
 * PCB.h
 *
 *  Created on: 24/5/2016
 *      Author: Lajew
 */

#ifndef SRC_PCB_H_
#define SRC_PCB_H_

#include <parser/parser.h>
#include <parser/metadata_program.h>
#include <commons/log.h>
#include "PCB.h"
#include <commons/collections/list.h>
#include "Nucleo.h"

//////Lista de estados RR
	t_list*  newState;
	t_list*  readyState;
	t_list*  execState;
	t_list** inputOutputState;
	t_list*  finishedState;
	t_list*  listaConsolas;
	t_list*  listaCPUs;

	typedef struct{
		int idSocketConsola;
		int pid_activo;
	}t_socketConsola;


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

	typedef struct{
		int idSocketCpu;
		//t_pcb* pcb;
		int pid;
		int boolEstado;
	}__attribute__((packed))t_socketCpu;

typedef struct INDICE_STACK{

			int   retVar[3]; 	// Posicion de memoria donde se almacenara el resultado  [ Pag | Offset | Size ]
			int   retPos;	 	// PC de retorno
	t_dictionary* argDicc;		// Lista de argumentos recibidos por la funcion
	t_dictionary* varDicc;	 	// Variables locales a la funcion

	} __attribute__((packed)) t_indice_stack;


	void*	serializar_indiceStack(int* retVar, int retPos, t_dictionary argDicc, t_dictionary varDicc);
	void 	deserializar_indiceStack();

	void*  serializarPCB(t_pcb* pcb,int* pcb_size);
	void   deserializarPCB(void* serializado, t_pcb* pcb);

	u_int32_t deserialize_u_int32_t(void* buffer,int* offset);

	t_intructions* deserialize_to_instructions(void* buffer,t_size tamanio,int* offset);

	void   initialize_pcb(t_pcb* pcb);
	t_pcb* crear_pcb_desde_codigo(void* codigo, int consolaSock, int umcSock, int tamanio_pagina, int paginas_stack);

	void   serialize_from_t_size(void* buffer, size_t value, int* offset);
	void   deserialize_to_t_size(void* buffer, size_t* value, int* offset);

	void   send_pcb(int socket, t_pcb* pcb);
	t_pcb* receive_pcb(int socket, t_pcb* pcb);

	t_socketCpu* buscarCPU(int socket);
	void	agregarCpu(int socket);
	void 	agregarConsola(int socket);
	void 	sacarConsola(int socket);
	void asignar_pid_a_consola(int sock, int pid);
	int obtener_pid_activo_consola(int sock);
	int obtener_consola_de_pid(int pid);
	t_socketConsola* buscarConsola_porSocket(int sock);

	t_socketConsola* 	buscarConsola(int socket);
	int 	search_free_execState(int sockCpu);
	bool 	pcbSearchCpu(void * arg);
	int 	sockSearchCpu;
	int 	receive_int_positive(int socketj);
	void 	finalizar_por_pid(int socketServer, int pid);

	void create_list_states_RR(s_conf* config);
	void create_list_ios(s_conf* config);
	void agregarPCBaIO(s_io* IOinvolucrado, t_pcb* pcb, int rafagas);
	void move_from_ready_to_exec();
	void move_from_exec_to_finish(int socketClient,int socketServer, struct s_header recvHeader, t_pcb* pcb);
	void move_exec_to_ready(int socket, t_pcb* pcb);
	void turn_on_sockCpuExec(int socket);
	void move_from_exec_to_finish_error(int socketClientj,int serverSockUMC, char* stringError, t_pcb* pcb);


#endif /* SRC_PCB_H_ */
