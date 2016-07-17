/*
 * Protocolo.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef SRC_PROTOLO_H_
#define SRC_PROTOLO_H_

//Basicos

#define ERROR						    0 //

#define OK  					    	5 //

#define FALLO							6 //

//Handshake 

#define SALUDO_CONSOLA 		  	       10 //

#define SALUDO_NUCLEO 			       11 //

#define SALUDO_CPU 	      		       12 //

#define SALUDO_UMC 			           13 //

#define SALUDO_SWAP 			       14 //

#define TAMANIO_PAGINA				   15 //

#define TAMANIO_QUANTUM				   16 //

#define TAMANIO_QUANTUMSLEEP		   17 //

#define IDS_IO						   18 //

#define IDS_SEM						   19 //

#define IDS_SHARED					   20 //

//funciones

#define ARCHIVO_ANSISOP                25 //

#define PCB_ESTRUCTURA 			       26 //

#define INICIAR_PROGRAMA               30 //

#define RESERVAR_MEMORIA			   31 //

#define SOLICITAR_BYTES				   32 //

#define ALMACENAR_BYTES 			   33 //

#define FINALIZAR_PROGRAMA 			   34 //

#define CAMBIO_PROCESO_ACTIVO		   35 //


//Avisos estado pcb segun cpu

#define FINALIZO		   50 // size = 0 OK, size != 0 error (eliminar_prog o eliminar_cpu).
#define IMPRIMIR 		   51 //
#define FIN_QUANTUM		   52 //
#define IO		           53 //

#define SEM_WAIT		   54 //
#define SEM_SIG 		   55 //
#define SEM_GET			   99
#define SEM_QUEUE		   98

#define SHARVAR_GET		   56 // id en payload
#define SHARVAR_SET		   57 // nuevo valor + id en payload

//Aviso de cpu a nucleo para eliminarse

#define CPU_FIN  		   70 // CPU deja de ejecutar, nucleo lo borra de su lista

//Errores de cpu a nucleo

#define ELIMINAR_PROG      1  // Fallo en programa, cpu sigue.
#define ELIMINAR_CPU       2  // Fallo en cpu, programa sigue.

//Avisos estado programa a consola

#define FINALIZO			50 // size = 0 OK, size != 0 error (errores por definir)
#define IMPRIMIR 			51 //

//Avisos de error de umc a cpu

#define PEDIDO_INVALIDO 	-10 // Se pidieron bytes por fuera de lo asignado
#define NO_HAY_MEMORIA 		-11 // Se quiso asignar fuera de la memoria asignada



	struct s_protocol{
		//Header
		int id;
		int size;
		//Payload
		char* payload;
	}__attribute((packed))__;

	struct s_header{
		//Header
		int id;
		int size;
	}__attribute((packed));




#endif /* SRC_PROTOLO_H_ */
