/*
 * UMC.h
 *
 *  Created on: 25/4/2016
 *      Author: Lajew
 */

#ifndef SRC_SWAP_H_
#define SRC_SWAP_H_

	#include <commons/log.h>
	#include <commons/config.h>
	#include <commons/string.h>
	#include <commons/txt.h>
	#include <commons/collections/list.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <unistd.h>
	#include "serialize.h"
	#include "Protocolo.h"
	#include <pthread.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include "Socket.h"
	#include <string.h>
	//#include "Memory.h"
	#include "TimeFunctions.h"

	#define PUERTO_E 5002
	#define BACKLOG 10
	#define PACKAGESIZE 1024

	typedef struct{
	  char* PUERTO;
	  char* IP_SWAP;
	  char* PUERTO_SWAP;
	    int MARCOS;
		int MARCO_SIZE;
		int MARCO_X_PROC;
      char* ALGORITMO;
		int ENTRADAS_TLB;
		int RETARDO;
	  char* RUTA_DUMP;
	}s_conf;

	typedef struct s_init_program{
			int pid;
			int cant_pag;
			char* codigo;
		};

	typedef struct s_req_bytes{
		int pagina;
		int offset;
		int tamanio;
	};


	typedef struct s_arg_thread{
		int socket;
		int socketSwap;
		int handshakeSwap;
	};

	typedef struct{
		int socket;    //Socket e indentificador de CPU.
		int pid;       //Proceso activo por el CPU
	}s_cpu;

	typedef enum{
		ESCRITURA,
		LECTURA,
	}e_operacion;

	s_conf config;
	t_log*  LOG;
	t_list* list_cpu;
	pthread_mutex_t mutex_config;// = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mutex_cpu;
	pthread_mutex_t mutex_swap;

	s_conf get_config(char* path, t_log* LOG);

	void init_struct_protocol(struct s_protocol* protocol);

	void init_struct_header(struct s_header* header);

	void serialize_from_protocol(struct s_protocol protocol, void* buffer);

	void socket_fallo_response(int socket);

	void socket_ok_response(int socket);

	void socket_bytes_response(int socket, int tamanio, char* bytes);

	int realize_handshake(int socket, t_log* LOG);

	void deserialize_init_program(struct s_init_program* data, int size_content, void* buffer);

	void deserialize_req_bytes(struct s_req_bytes* data, void* buffer);

	void deserialize_save_bytes(struct s_req_bytes* data, char*bytes, int size, void* buffer);

	int  request_bytes(struct s_req_bytes* data, int pid, int socketSwap, char* resp);

	int  save_bytes(struct s_req_bytes* data, int pid, int socketSwap, char* buffer);

	int  request_bytes_to_SWAP(struct s_req_bytes* data, int pid, int socketSwap, char* resp);

	int  request_memory_to_SWAP(int socket, int pid, int cant_pag);

	int  update_page_in_SWAP(int socket, int pid, int pagina, char* buffer);

	int  save_request_bytes(struct s_req_bytes* data, int pid, char* bytes, int socket, int* marco, e_operacion operacion);

	int  send_fin_to_SWAP(int socket, int pid);

	void add_cpu_to_list(int socket);

	int  search_pid_from_cpu(int socket);

	int  change_cpu_program(int socket, int pid);

	s_cpu* search_cpu(int socket);

	void destroy_cpu(int socket);

	int  send_page_size_to_socket(int socket, int tam_pag);

	void build_pages_for_process(int pid, int cant_pag);

	int  split_and_send_code(int socket, int pid, int tam_pag, char* codigo);

	void* main_server(void* arg);

	void* threads_server(void* arg);

	void  dump_struct(int pid);

	void  dump_content(int pid);

	void  dump_content_all();

	int   check_request_is_valid(struct s_req_bytes* data, int pid);

	void  socket_error_response(int socketReponse, int id);
#endif /* SRC_SWAP_H_ */

#ifndef SRC_MEMORY_H_
#define SRC_MEMORY_H_

#include "UMC.h"
#include "TimeFunctions.h"

typedef struct t_tlb{
	  int marco;
	  int pagina;
	  int pid;
	  int modif_flag; 		//(1 si, 0 no)  -> Se modificó o no
	  int memory_flag;		//(1 si, 0 no)  -> Si está en Memoria principal o no
	  int ref_time;   		//Tiempo de referencia para LRU
	}s_tlb;

typedef struct t_page_table{
	  int marco;
	  int pagina;
	  int bituso;
	  int puntero;			//"puntero" a la página actual
	  int modif_flag; 		//(1 si, 0 no)  -> Se modificó o no
	  int memory_flag;		//(1 si, 0 no)  -> Si está en Memoria principal o no
	  int ref_time;   		//Tiempo de referencia para LRU
	}s_page_table;

typedef struct t_process{
	  int pid;
	  t_list* pages;
	}s_process;

	char* memory;
	t_list* tlb;
	t_list* process_tpage;
	int* 	frames;			//(1 ocupado, 0 libre)

	pthread_mutex_t mutex_tlb;
	pthread_mutex_t mutex_memory;
	pthread_mutex_t mutex_page_table;
	pthread_mutex_t mutex_frames;

	int   create_struct_memory();
	void  destroy_struct_memory();
	void  save_in_memory(int socketSwap, int pid, int pagina, int frame, char* bytes, e_operacion oper);
	void  create_mp();
	void  destroy_mp();
	void  create_tlb();
	void  destroy_tlb();
	void  clear_tlb_from_pid(int pid);
	s_tlb* find_process_in_tlb(int pid);
	s_tlb* find_process_page_in_tlb(int pid, int page);
	s_page_table* get_page_from_process(s_process* process, int page);
	void  init_tlb();
	void  add_page_to_tlb(int socketSwap, s_page_table* page, int pid, e_operacion oper);
	void  create_page_table();
	void  destroy_page_table();
	void  set_page_as_present(int socketSwap, int pid, int frame, int pagina, s_page_table* page, e_operacion oper);
	int   destroy_page_table_from_pid(int pid);
	s_process* find_page_table_from_pid(int pid);
	void  create_frames();
	void  destroy_frames();
	void  init_frames();
	int   get_first_free_frame();
	int   clear_all_memory_from_pid(int pid);
	t_list* get_present_pages();
	t_list* get_present_pages_from_pid(int pid);
	s_page_table*  get_victim_using_clock(t_list* pages);
	s_page_table*  get_victim_using_clock_m(t_list* pages);
	void  set_modif_all_pages();
	int   search_pid_present_in_frame(int frame);
	void clear_pointer_to_pages(t_list* pages);
	void clear_page_in_tlb(int pid, int pagina);

#endif /* SRC_MEMORY_H_ */

