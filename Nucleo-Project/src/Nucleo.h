/*
 * Nucleo.h
 *
 *  Created on: 22/4/2016
 *      Author: utnso
 */

#ifndef SRC_CONFIGURACIONNUCLEO_
#define SRC_CONFIGURACIONNUCLEO_
	#include <parser/metadata_program.h>
    #include <parser/parser.h>
	#include <commons/log.h>
	#include <commons/config.h>
	#include <commons/string.h>
	#include <commons/collections/queue.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <unistd.h>
	#include "Protocolo.h"
	#include <pthread.h>
	#include <semaphore.h>
	#include <sys/inotify.h>
	#include <errno.h>

	#define BACKLOG 10
	#define PACKAGESIZE 1024
	#define ERROR		-1
	#define BUFFSIZE 19
	#define EVENT_SIZE  (sizeof(struct inotify_event))
	#define BUF_LEN     (1024 * (EVENT_SIZE + 16))



	typedef struct{
		char* puerto_prog;
		char* ip_umc;
		char* puerto_umc;
		//char* ip_cpu;
		char* puerto_cpu;
		int quantum;
		int quantum_sleep;
		char** io_id;
		char** io_sleep;
		char** sem_id;
		char** sem_val;
		char** shared_vars;
		int  stack_size;}s_conf;

	typedef struct{
		char* idIO;
		int sleep;
		pthread_mutex_t sem;
		t_queue* queue;
	}s_io;

	typedef struct{
		int valor;
	}t_shar_var;

	typedef struct{
		int valor;
		t_queue* queue;
	}t_sem;


	t_list* listIO;
	s_conf config;
	s_conf		get_config(char* path);
	t_dictionary* shared_vars;
	t_dictionary* semaforos;
	int 	serverSockUMC;
	int		pid_count;

	int 		realize_handshake(int socket);

	void 		serialize_from_header(struct s_header header, void* buffer);

	int 		send_quantum_size_to_socket(int socket, int tam_quantum);
	int 		send_quantum_sleep_to_socket(int socket, int tam_qSleep);

	void 		socket_ok_response(int socket);
	void 		socket_fallo_response(int socket);
	int			receive_response(int socket);
	void		delete_from_exec_and_asign(int socket);

	int 		search_secuencial_index_string(char* searchString,char** listString);
	char* 		receive_string(int socket,int size);
	int			receive_tamanio_pagina(int socket, int* tamanio_pagina);
	void		receive_imprimir(int socket,int size);
	void 		imprimir_en_consola(int socketConsola, char* printString);
	struct s_protocol  receive_ansisop(int socket,int tamanio);
	void*		serialize_init_program(int pid, int cant_pag , char* codigo);
	int 		send_arrays_string_to_socket(int socket,char** array,int  headerId);
	void 		serialize_from_protocol(struct s_protocol protocol, void* buffer);
	int 		buscar_PID_activo_de_CPU(int sock);
	void 		delete_from_exec(int pid);
	void 		desocupar_cpu(int sock);
	void 		sacarCPU(int socket);


	//IO
	int 		move_to_ioList(int socket,int tipoIo);
	void* 		receive_io(int socket, int tamanioIdIO,char** IO_ID);
	void* 		serverIO(void* buffer);
	void* 		threads_IO(void* arg);
	s_io*		buscarIO(char* id);
	pthread_mutex_t* serverIOM;
	sem_t bufferReadThread;

	t_sem* 		get_semaforo(char* nombre);
	void 		wait_semaforo(char* nombre, t_sem* semaforo);
	void 		signal_semaforo(char* nombre);
	int 		recibir_accion_semaforo(int sock, char* key, t_sem* semaforo);
	void 		set_shared_var(char* nombre, int valor);
	int 		get_shared_var(char* nombre);

	void 		completar_diccionario_semaforos();
	void 		completar_diccionario_shared_vars();

	void 		send_value_to_socket(int socket, int id, int value);
	void 		manejar_caida_cpu(int sock, int sockUMC);

	//Inotify
	void* monitorConfig(void* arg);
	//Planificador RR
	void* planificadorRR(void* arg);

	pthread_mutex_t monitorConfigMutex;
	pthread_mutex_t mutex_ready;
	pthread_mutex_t mutex_cpu;
	pthread_mutex_t mutex_pid_count;

	pthread_mutexattr_t mcattr;

#endif /* SRC_CONFIGURACIONNUCLEO_ */
