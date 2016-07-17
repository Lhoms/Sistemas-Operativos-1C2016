/*
 * UMC.c
 *
 *  Created on: 24/4/2016
 *      Author: Lajew
 *      Descrp: Proceso UMC
 */

#include "UMC.h"

int main(int argc, char* argv[]){

	char comando[80];

	//Config + Log + inicializo mutex
	pthread_mutexattr_t mattr;
	pthread_mutex_init(&mutex_config, &mattr);
	pthread_mutex_init(&mutex_tlb, &mattr);
	pthread_mutex_init(&mutex_cpu, &mattr);
	pthread_mutex_init(&mutex_swap, &mattr);
	pthread_mutex_init(&mutex_page_table, &mattr);
	pthread_mutex_init(&mutex_frames, &mattr);
	pthread_mutex_init(&mutex_memory, &mattr);

	LOG = log_create("UMC.log","UMC.c",0,1);

	if (argc != 2){
		log_error(LOG, "No se indicó la ruta del archivo de configuración");
		return EXIT_FAILURE;
	}
		else
			config = get_config(argv[1], LOG);

	//Validaciones
	if (string_is_empty(config.IP_SWAP) ||
		string_is_empty(config.PUERTO)  ||
		string_is_empty(config.PUERTO_SWAP))
	{
		return EXIT_FAILURE;
	}

	//Creación de estructuras de memoria a utilizar.
	if(create_struct_memory()){
		log_error(LOG, "Error al crear espacio de memoria principal");
		return EXIT_FAILURE;
	}else{
		list_cpu = list_create();
	}


	//Creo hilo para atender solicitudes y comunicarme con SWAP
	void* arg_server;
	pthread_attr_t attr;
	pthread_t server_thread;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&server_thread, &attr, &main_server, arg_server);

	pthread_attr_destroy(&attr);

	//Consola de UMC
	printf("*******************************************************************\n");
	printf("*                     Consola UMC Grupo Lajew                     *\n");
	printf("*******************************************************************\n");
	printf("*                       Comando ayuda: help                       *\n");
	printf("*******************************************************************\n");

	do{
		printf("UMC-UTN-GrupoLAJEW<>~: ");
		fgets(comando,80,stdin);

		//Función de retardo
		if (!strcmp(comando, "retardo\n")){
			char retardo[80] = "";
			printf("\nUMC-UTN-GrupoLAJEW<>~/Retardo: ");
			fgets(retardo,80,stdin);

			//Saco el \n
			char* retardo_val = string_substring(retardo, 0, string_length(retardo)-1);

			if(!string_equals_ignore_case(retardo_val, "0")){

				pthread_mutex_lock( &mutex_config );
					config.RETARDO = atoi(retardo_val); //Bendito seas ATOI
				pthread_mutex_unlock( &mutex_config );
			}
		}else{
		}

		//Función de dump
		if (!strcmp(comando, "dump -struct\n")){
			printf("\nUMC-UTN-GrupoLAJEW<>~/Dump-Struct: **Eligió mostrar un reporte de las estructuras de memoria. Elija un PID(0 para mostrar toda la memoria)**");
			char pid[80];
			printf("\nUMC-UTN-GrupoLAJEW<>~/Dump-Struct/PID: ");
			fgets(pid, 80, stdin);

			int pid_nro = atoi(pid);

			dump_struct(pid_nro);
		}else{

			if (!strcmp(comando, "dump -content\n")){
				printf("\nUMC-UTN-GrupoLAJEW<>~/Dump-Content: **Eligió mostrar un reporte del contenido de memoria. Elija un PID(0 para mostrar toda la memoria)**");
				char pid[80];
				printf("\nUMC-UTN-GrupoLAJEW<>~/Dump-Content/PID: ");
				fgets(pid, 80, stdin);

				int pid_nro = atoi(pid);

				if(pid_nro != 0){
					dump_content(pid_nro);
				}else{
					dump_content_all();
				}
			}
		}

		//Función de flush
		if (!strcmp(comando, "flush -tlb\n")){
			log_info(LOG, "Se requiere realizar un flush de la TLB");
			if( config.ENTRADAS_TLB > 0){
				clean_tlb();
				log_info(LOG, "Flush realizado. TLB limpia!");
				printf("TLB limpia!\n");
			}else{
				log_info(LOG, "TLB no activa para flush");
				printf("TLB no activa\n");
			}
		}else{

			if (!strcmp(comando, "flush -memory\n")){
				log_info(LOG, "Se requiere setear todas las páginas en modificado");
				usleep(config.RETARDO * 1000);
				set_modif_all_pages();
				log_info(LOG, "Todas las páginas fueron seteadas como modificadas.");
				printf("Páginas actualizadas con bit de modificación\n");
			}
		}

		if (!strcmp(comando, "help\n")){
			printf("\n");
			printf("**** La consola de éste proceso comprende los siguientes comandos: \n");
			printf("     - retardo: Éste comando modifica el valor en milisegundos del retardo para acceso a memoria \n");
			printf("     - flush -<parametro>: Dependiendo el valor del parámetro se realizan las siguientes acciones: \n");
			printf("             .memory: Éste comando marca todas las páginas presentes en memoria como modificadas \n");
			printf("                      Ejemplo: flush -memory \n");
			printf("             .tlb:    Éste comando limpia la TLB \n");
			printf("                      Ejemplo: flush -tlb \n");
			printf("     - dump  -<parametro>: Dependiendo del valor del parámetro se muestra un listado de: \n");
			printf("             .struct:  Éste comando muestra un reporte de las estructuras de memoria de uno o todos los procesos\n");
			printf("             .content: Éste comando muestra un reporte del contenido de memoria\n");
			printf("     - exit: Salir\n");
			printf("****\n");
			printf("\n");
		}

	}while(strcmp(comando, "exit\n"));

	destroy_struct_memory();
	//Destruyo LOG
	log_destroy(LOG);
	printf("LOG destruído");
	pthread_exit(NULL);
	return EXIT_SUCCESS;

}
