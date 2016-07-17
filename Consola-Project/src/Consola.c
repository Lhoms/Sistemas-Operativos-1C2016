/*
 * Consola.c
 *
 *  Created on: 22/4/2016
 *      Author: Lajew
 *      Descrp: Proceso Consola
 */

#include "Consola.h"

int main(int argc, char *argv[]){
	//INICIALIZO ESTRUCTURAS Y VARIABLES
	s_conf config ;
	t_log*  LOG;
	int serverSock;
	char* comando = string_repeat('\0', 80);
    char* pathFile = string_new();
    struct s_protocol sender;
    struct s_protocol receiver;
    int status;

	//CREACION DE LOG
	LOG = log_create("Consola.log","Consola.c",0,1);
	log_info(LOG,"\n////////////////////////////////////////////////////////////////////////comienza módulo.");

	//SE LEVANTA EL CONFIG
	log_info(LOG, "Se lee archivo de configuración");
	config = get_config(LOG);

	//CONFIRMACION DE RECEPCION DE IP NUCLEO
	if (string_is_empty(config.ip_nucleo))
	{
		return EXIT_FAILURE;
	}

	//CREACION DE SOCKET CLIENTE
	serverSock = create_socket_cliente(config.ip_nucleo, config.puerto_nucleo, LOG);

	//Handshake para inicio de comunicaciones
	int handshake = realize_handshake(serverSock, LOG);
	if (handshake == FALLO){
	   log_error(LOG, "Error al conectarse al servidor");
	   return EXIT_FAILURE;
	}

	printf("*******************************************************************\n");
	printf("*                       Consola Grupo Lajew                       *\n");
	printf("*******************************************************************\n");
	printf("*                       Comando ayuda: help                       *\n");
	printf("*******************************************************************\n");

	do{
		//Inicializo la estructura de envío y recepción
		init_struct_protocol(&sender);
		init_struct_protocol(&receiver);

		free(comando);

		comando = string_repeat('\0', 80);

		printf("CONSOLA-UTN-GrupoLAJEW<>~: ");

		if(argc != 2){
		fgets(comando,80,stdin);
			}

		else{
			comando = string_duplicate(argv[1]);
			printf("\n%s\n", comando);
		}

		//if(string_starts_with(comando, "./")){
		if (strcmp(comando, "help\n")){
			//El comando es un script. Resto 3, 2: './'; 1: '\n'
			if(argc != 2){
			pathFile = string_substring(comando,0, string_length(comando)-1);}

			else{
			pathFile = string_substring(comando,0, string_length(comando)+1);}


			log_info(LOG, "Se ejecuta archivo %s", pathFile);

			sender = get_sender_from_file(pathFile);

			//Verifico que haya podido cargar bien la estructura
			if (sender.id != 0){
				log_info(LOG, "Script cargado con éxito. Se envía al núcleo");

				//sendBuffer = malloc(sizeof(s_protocol));
				void* sendBuffer = malloc(sizeof(int) + sizeof(int) + sender.size);
				serialize_from_protocol(sender, sendBuffer);

				//deserialize_to_protocol(&receiver, sendBuffer);
				status = send(serverSock, sendBuffer, sizeof(int) + sizeof(int) + sender.size, 0);
				if (status != ERROR){
					log_info(LOG, "Mensaje enviado correctamente. Esperando respuesta");

					do{
					//Espero una respuesta del mismo tipo que del envío.
					//Primero leo el header
						status = recv(serverSock, &receiver, sizeof(struct s_header), MSG_WAITALL);
						if (status != ERROR){
							//Los mensajes puede ser que finalizó el programa o algo para imprimir por pantalla.
							log_info(LOG, "Se recibe mensaje de núcleo");
							switch(receiver.id){
							case FINALIZO:
									 if(receiver.size == 0){
										 printf("Script procesado correctamente\n");
										 log_info(LOG, "Script procesado correctamente");
									 }else{
										 printf("Script falló con error:\n");

									     char* error = malloc(receiver.size);
									     status = recv(serverSock, error, receiver.size, MSG_WAITALL);
									     if (status != 0){
									    	 log_error(LOG, "Script falló con error: %s", string_substring(error,0,receiver.size));
									    	 printf("%s\n",  string_substring(error,0,receiver.size));
									     }

									     free(error);

									 }
									 status = 0; //Mato el DO.
									 break;
							case IMPRIMIR:
									 printf("Mensaje recibido\n");
									 char* message = malloc(receiver.size);
									 status = recv(serverSock, message, receiver.size, MSG_WAITALL);
									 if (status != 0){
										 log_info(LOG, "Se recibió el siguiente mensaje desde núcleo: %s", string_substring(message,0,receiver.size));
										 printf("%s\n", string_substring(message,0,receiver.size));
									 }

									 free(message);
									 break;
							default:
									log_error(LOG, "Error al iniciar programa");
									printf("No se pudo iniciar programa\n");
									status = 0;
							}

						}else{
							log_error(LOG, "Error al recibir respuesta");

						}

					}while(status != 0);
				}else{
					log_error(LOG, "Error al enviar programa al núcleo");
					printf("Error al enviar programa al núcleo");
				}
				//Libero memoria
				free(sendBuffer);
			}else{
				log_error(LOG, "Error al obtener el script");
			}
		}else{
			if (!strcmp(comando, "help\n")){
				printf("\n");
				printf("**** Para ingresar un script ansisop ingrese el comando <path archivo ansisop> \n");
				printf("\n");
			}else{
				printf("Comando inesperado. Para más ayuda ingrese el comando help");
			}
		}
	}while(strcmp(comando, "exit\n"));

	close(serverSock);

	log_destroy(LOG);
	return EXIT_SUCCESS;
}
