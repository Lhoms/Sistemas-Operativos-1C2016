/*
 * Swap.h
 *
 *  Created on: 24/4/2016
 *      Author: Lajew
 */

#ifndef SRC_SWAP_H_
#define SRC_SWAP_H_
	#include <commons/bitarray.h>
	#include "Socket.h"
	#include "serialize.h"
	#include "Protocolo.h"
	#include <sys/mman.h>


	#define BACKLOG 10
	#define PACKAGESIZE 1024
	#define NO_DISPONIBLE 0
	#define NECESITA_COMPACTACION -2
	#define NO_TIENE_SUFICIENTES_PAGINAS -1
	#define ESPACIO_OCUPADO 1
	#define ESPACIO_LIBRE 0

	typedef struct{
	  char* puerto_escucha;
      char* nombre_swap;
      char* cant_pag;
      char* tam_pag;
      char* retardo_compactacion;
	  char* retardo_acceso;
	}s_conf;

	typedef struct nodo_lista_doblemente_enlazada
	{
		pid_t pid;
		int num_pag;
		int pos_en_swap;
		struct nodo_lista_doblemente_enlazada *siguiente;
		struct nodo_lista_doblemente_enlazada *anterior;

	}nodo_lista;

	t_log*  LOG;
	s_conf config;

	s_conf get_config(char* path, t_log* LOG);

	char* recibirArchivo(int SocketFD, t_log* LOG);

	int StringToNum(char palabra[]);

	int agregar_elemento_lista (nodo_lista **lista, pid_t nuevo_pid, int nuevo_num_pag, int nuevo_pos_swap);

	int quitar_elemento_lista (nodo_lista **lista, pid_t pid_quitar, t_log* LOG);

	void mostrar_lista(nodo_lista **lista);

	void init_struct_protocol(struct s_protocol* protocol);

	void init_struct_header(struct s_header* header);

	void serialize_from_protocol(struct s_protocol protocol, void* buffer);

	void serialize_from_header(struct s_header header, void* buffer);

	void socket_response(int socket,int id);

	void socket_ok_response(int socket);

	void socket_fallo_response(int socket);

	int tiene_suficientes_paginas(t_bitarray* bitmap,int cant_pag_pedidas);

	int reservar_memoria(t_bitarray* bitmap,nodo_lista **estructura_control,pid_t pid,int primer_espacio_libre,int cant_pag_pedidas);

	int finalizar_programa(t_bitarray* bitmap,nodo_lista **estructura_control,pid_t pid);

	int espacio_pagina0(nodo_lista **lista,pid_t pid);

	int espacio_pagina_pedida(nodo_lista **lista,pid_t pid,int num_pag_pedida);

	int almacenar_bytes(char *puntero_archivo,nodo_lista **estructura_control,pid_t pid,int num_pagina,char* recvMessage);

	int enviar_pagina(char *puntero_archivo,int socketCliente,nodo_lista **estructura_control,pid_t pid,int num_pagina);

	int compactacion(t_bitarray* bitmap,nodo_lista **estructura_control,char *puntero_binario);

	int primer_espacio_libre(t_bitarray* bitmap);

	int primer_espacio_ocupado_fragmentado(t_bitarray* bitmap);

	int compactar_un_proceso(t_bitarray* bitmap,nodo_lista **estructura_control,char *puntero_binario,int espacio_libre,int espacio_mover);

	pid_t buscar_pid_segun_espacio(nodo_lista **lista,int espacio_mover);

	int cant_paginas_segun_pid(nodo_lista **lista,pid_t pid);

	int mover_paginas_proceso(char *puntero_binario,int espacio_libre,int espacio_mover,int cant_paginas_mover);

	int marcar_como_libre(t_bitarray* bitmap,int espacio_mover,int cant_paginas_mover);

	int marcar_como_ocupado(t_bitarray* bitmap,int espacio_libre,int cant_paginas_mover);

	int editar_inicio_swap(nodo_lista **lista,pid_t pid,int espacio_libre);

	void mostrar_bitmap(t_bitarray* bitmap);



#endif /* SRC_SWAP_H_ */
