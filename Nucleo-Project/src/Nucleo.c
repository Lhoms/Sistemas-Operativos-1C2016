/*
 * Nucleo.c
 *
 *  Created on: 22/4/2016
 *      Author: Lajew
 *      Descrp: Proceso Núcleo
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "Socket.h"
#include "serialize.h"
#include "PCB.h"

int main(int argc, char* argv[]){
	//VARIABLES y ESTRUCTURAS (LUEGO VA A ACHICARSE Y REDURI EL NRO DE VARIABLES)
	fd_set readset, tempset;
	int maxfd;
	int j, result, sockList;
	struct sockaddr_in addr;
	int listenSock;
	socklen_t addrlen = sizeof(addr);
	LOG = log_create("Nucleo.log","Nucleo.c",0,1);
	log_info(LOG,"\n////////////////////////////////////////////////////////////////////////comienza módulo.");
	int peersock;
	int status, value;
	void* buffer;
	int* intBuffer;
	struct s_header recvHeader;
	struct s_protocol protocol;
	int tamanio_pagina;
	listaConsolas=list_create();
	t_pcb *pcb ;


	//CONFIRMACION DE RECEPCION DE ARCHIVO CONFIGURACION
		if (argc != 2){
			log_error(LOG, "No se indicó la ruta del archivo de configuración");
			return EXIT_FAILURE;
		}else{
			config = get_config(argv[1]);
		}
		if (string_is_empty(config.puerto_prog)){
			return EXIT_FAILURE;
		}

	//creo y lleno semaforos y variables compartidas
	semaforos = dictionary_create();
	shared_vars = dictionary_create();

	completar_diccionario_semaforos();
	completar_diccionario_shared_vars();

	//CREO SOCKET DE ESCUCHA Y LO PONGO A ESCUCHAR
	listenSock = create_socket_servidor(config.puerto_prog);
	listen(listenSock, BACKLOG);
	
	//creo las listas de colas para algoritmo
	create_list_states_RR(&config);

	//creo las listas de colas para IO
	create_list_ios(&config);

	//INICIALIZO LOS HILOS DE ATENCION DE IO Y SEMAFOROS
	pthread_t pthreadIO;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	sem_init(&bufferReadThread,1,1);


	pthread_t inotifyThread;
	pthread_t RRthread;
	pthread_attr_t cattr;
	pthread_mutexattr_init(&mcattr);
	pthread_mutex_init(&monitorConfigMutex,&mcattr);
	pthread_attr_init(&cattr);
	pthread_attr_setdetachstate(&cattr,PTHREAD_CREATE_DETACHED);
	pthread_create(&inotifyThread, &cattr, &monitorConfig,(void*)argv[1]);
	pthread_create(&RRthread, &cattr, &planificadorRR,NULL);

	//INICIALIZO SET DE SOCKETS Y LE AGREGO EL SOCKET ESCUCHA
	FD_ZERO(&readset);
	FD_SET(listenSock, &readset);
	maxfd = listenSock;

	serverSockUMC = create_socket_cliente(config.ip_umc,config.puerto_umc);
	log_info(LOG, "ServerSockUMC conectado con el servidor");

	//SE REALIZA HANDSHAKE CON UMC
	realize_handshake(serverSockUMC);
	//SE RECIBE TAM DE PAGINA
	receive_tamanio_pagina(serverSockUMC, &tamanio_pagina);

	//Inicializo contador de PID
	pid_count = 0;

	do {
	   //COPIO EL READSET A UNO TEMPORAL
	   memcpy(&tempset, &readset, sizeof(tempset));
	   //TOMO CONEXIONES ENTRANTES
	   result = select(maxfd + 1, &tempset, NULL, NULL, NULL);
	   if (result == 0) {
	      printf("select() timed out!\n");
	   }
	   else if (result < 0) {
	      printf("Error in select()");
	   }
	   else if (result > 0) {
	      //ME FIJO SI HUBO ACTIVIDAD EN EL SOCKET ESCUCHA Y ACEPTO LAS CONEXIONES ENTRANTES SI LAS HUBO
	      if (FD_ISSET(listenSock, &tempset)) {
	         peersock =  accept(listenSock, (struct sockaddr	*)&addr,&addrlen);
	         if (peersock < 0) {
	            printf("Error in accept()");
	         }
	         else {
	         	//AGREGO EL SOCKET AL SET Y OBTENGO EL MAXIMO
	            FD_SET(peersock, &readset);
	            maxfd = (maxfd < peersock)?peersock:maxfd;
	         }
	        //SACO EL SOCKET ESCUCHA PARA NO TENERLO EN CUENTA A LA HORA DE RECORRER EL SET Y VER SI HUBO ACTIVIDAD
	         FD_CLR(listenSock, &tempset);
	      }
	      //RECORRO EL SET DE SOCKETS Y CHECKEO ACTIVIDAD EN CADA UNO DE ELLOS
	      for (j=0; j<maxfd+1; j++) {
	      	//SI HUBO ACTIVIDAD;HAGO RECV
	         if (FD_ISSET(j, &tempset)) {
	            do {
	               //result = recv(j, buffer, 19, 0);
	            	result = recv(j, &recvHeader, sizeof(struct s_header), 0);
	            } while (result == -1);

	            if (result > 0) {
	               //if (!string_is_empty(buffer))
	               if (recvHeader.id != 0)
	               {
	               	//ACA VA LA LOGICA PARA CADA PROCESO(EN ESTE CASO SIEMPRE RECIBO BUFFER; PROXIMAMENTE RECIBIRE ESTRUCTURAS DEPENDIENDO DE QUIEN HABLE Y QUE MANDE)
	            	  pthread_mutex_lock(&monitorConfigMutex);
	            	  switch(recvHeader.id){
	            	  	  case SALUDO_CPU:
	            	  		  socket_ok_response(j);
	            	  		  send_arrays_string_to_socket(j,config.sem_id,IDS_SEM);
	            	  		  send_arrays_string_to_socket(j,config.io_id,IDS_IO);
	            	  		  send_arrays_string_to_socket(j,config.shared_vars,IDS_SHARED);
	            	  		  //send_quantum_size_to_socket(j,config.quantum);
	            	  		  send_quantum_sleep_to_socket(j,config.quantum_sleep);

	            	  		  agregarCpu(j);
	            	  		  log_info(LOG,"Se logra enviar a cpu Quantum, QuantumSleep, IDS y agregar CPU a lista.");
	            	      break;

	            	  	  case SALUDO_CONSOLA:
	            	  		  socket_ok_response(j);
	            	  		  agregarConsola(j);
	            	  		  log_info(LOG,string_from_format("Ingresa una consola al Nucleo de PID %d",j));
	            	  	  break;

	            	  	  case ARCHIVO_ANSISOP:
	            	  		  //recibo ansisop
	            	  		  protocol=receive_ansisop(j,recvHeader.size);
	            	  		  pcb=crear_pcb_desde_codigo(protocol.payload,j,serverSockUMC, tamanio_pagina,config.stack_size);
	            	  		  free(protocol.payload);

	            	  		  if(pcb->ID!=0){
	            	  			  list_add(newState,pcb);
	            	  			  log_info(LOG,string_from_format("Se agrega PID %d a newState:%d news", pcb->ID ,list_size(newState)));

	            	  			  list_add(readyState,list_remove(newState,0));
	            	  			  log_info(LOG,string_from_format("Se agrega PID %d a readyState:%d news",pcb->ID,list_size(readyState)));

	            	  			  pthread_mutex_unlock(&mutex_ready);
	            	  			  /*if((sockList=search_free_execState(pcb->ID))==0){
	            	  				  log_info(LOG,"No se encuentra una cpu libre");
	            	  		  	  }else{
	            	  		  		send_pcb(sockList,pcb);
	            	  		  	  }*/
	            	  		  }else{
	            	  			  log_error(LOG, "Error al iniciar programa. Ver motivos en SWAP/UMC");
	            	  			  socket_fallo_response(j);
	            	  			  free(pcb);
	            	  			  //hacer algo avisando el error
	            	  		  }
	            	  	  break;

	            	  	  case IO:
	            	  		  log_info(LOG, "Se recibe IO");
	            	  		  int offset = 0;
	            	  		  int rafagas;
	            	  		  void* recvIO = malloc(recvHeader.size);

	            	  		  int stat = recv(j, recvIO, recvHeader.size, 0);
	            	  		  if (stat != ERROR){

	            	  			  char* id = malloc(recvHeader.size - sizeof(int));
	            	  			  deserialize_to_string(recvIO, id, recvHeader.size - sizeof(int), &offset);
	            	  			  log_info(LOG, "IO: %s", id);
	            	  			  deserialize_to_int(recvIO, &(rafagas), &offset);
	            	  			  log_info(LOG, "Ráfagas: %d", rafagas);

	            	  			  s_io* IOinvolucrado = buscarIO(id);
	            	  			  //Busco el IO donde asignar el pcb con las ráfagas
	            	  			  if(IOinvolucrado!=NULL){
	            	  				log_info(LOG, "Respondo que llegó bien la información a la CPU");
							//envio la llegada correcta del mensaje
							socket_ok_response(j);

							//Recibo PCB
							t_pcb* pcb=malloc(sizeof(t_pcb));
							receive_pcb(j,pcb);

					            	recv(j, &recvHeader, sizeof(struct s_header), 0);
					            	 if (recvHeader.id == CPU_FIN){
					            	  sacarCPU(j);
					            	  log_info(LOG,"CPU %d deja de ejecutar", j);
					            	  	}

					            	  else{
							     //Desocupo CPU
							     desocupar_cpu(j);
					            	  	}

							 //Lo paso a la cola de IO correspondiente
							 log_info(LOG, "Agrego PCB+Rafagas a queue de IO");
	            	  				  agregarPCBaIO(IOinvolucrado, pcb, rafagas);

	            	  				  //Después de pasarla a IO, la saco de EXEC y re-planifico
	            	  				  log_info(LOG, "Elimino el PCB de exec y re-planifico");
	            	  				  //delete_from_exec_and_asign(j);
	            	  				  delete_from_exec(pcb->ID);
	            	  			  }else{
	            	  				  log_error(LOG, "La CPU envió un IO no esperado");
	            	  				  socket_fallo_response(j);
	            	  			  }

	            	  			  free(id);
	            	  			  //free(pcb);
	            	  		  }else{
	            	  			  manejar_caida_cpu(j, serverSockUMC);
	            	  		  }

	            	  		  free(recvIO);
	            	  		  /*
	            	  		buffer=receive_io(j,recvHeader.size,config.io_id);
	            	  		//si estan usando el buffer espera que termine de usarlo el hilo
	            	  		//para copiarlo en su stack
	            	  		sem_wait(&bufferReadThread);
	            	  		pthread_create(&pthreadIO,&attr,serverIO,buffer);
	            	  		move_from_ready_to_exec();*/
	            	  	  break;

	            	  	  case FIN_QUANTUM:
	            	  		  //envio la llegada correcta del mensaje
	            	  		  socket_ok_response(j);

	            	  		  //Recibo el PCB
	            	  		  t_pcb* pcbRecv=malloc(sizeof(t_pcb));
	            	  		  receive_pcb(j,pcbRecv);

		            	  	recv(j, &recvHeader, sizeof(struct s_header), 0);
		            	  		if (recvHeader.id == CPU_FIN){
		            	  			sacarCPU(j);
		            	  			log_info(LOG,"CPU %d deja de ejecutar", j);
		            	  		}

		            	  	  if(buscarConsola(pcbRecv->ID)==NULL){
		            	  		move_from_exec_to_finish(j,serverSockUMC,recvHeader, pcbRecv);
		            	  	  	  }

		            	  	  else if (pcbRecv->ID != 0){
	            	  			  log_info(LOG,string_from_format("Se recibe pcb de cpu: %d PID:%d",j,pcbRecv->ID));
	            	  			  move_exec_to_ready(j, pcbRecv);
	            	  		  }else{
	            	  			  log_error(LOG,"Error al recibir el PCB por fin de quantum");
	            	  			  free(pcbRecv);
	            	  		  }


	            	  		  break;

	            	  	  case FINALIZO:
	            	  		  log_info(LOG, "Nucleo recibe un pedido de finalización");
	            	  		  char* stringError = malloc (recvHeader.size);
	            	  		  status = -1;
	            	  		  if (recvHeader.size > 0){
	            	  		  	  status = recv(j, stringError, recvHeader.size,0);
	            	  		  }
	            	  		  if(status != 0){
								  socket_ok_response(j);
								  t_pcb* pcb=malloc(sizeof(t_pcb));
								  receive_pcb(j,pcb);
								  if(pcb->ID != 0){
									  log_info(LOG, "Se recibió el pedido de finalización del PCB: %d", pcb->ID);
									  switch(recvHeader.size){
									  case 0:
										  move_from_exec_to_finish(j,serverSockUMC,recvHeader, pcb);
									  break;

									  default:
										  move_from_exec_to_finish_error(j,serverSockUMC,stringError, pcb);
									  break;
									  }
								  }else{
									log_error(LOG, "Error al recibir PCB para finalización");
									free(pcb);
								  }
								recv(j, &recvHeader, sizeof(struct s_header), 0);
									if (recvHeader.id == CPU_FIN) sacarCPU(j);
	            	  		  }else{
	            	  			  log_error(LOG, "Se cayó la CPU mientras se recibía string de error");
	            	  			  sacarCPU(j);
	            	  		  }
	            	  		  if(recvHeader.size > 0){
	            	  			  free(stringError);
	            	  		  }
	            	  	  break;

	            	  	  case IMPRIMIR:
	            	  		  socket_ok_response(j);
	            	  		  receive_imprimir(j,recvHeader.size);
						  break;

	            	  	  case SHARVAR_GET:


	            	  		  buffer = malloc(recvHeader.size);
	            	  		  status = recv(j, buffer, recvHeader.size, 0);

	            	  		if (status != ERROR){

	            	  		log_info(LOG, "Se procede a obtener variable compartida %s", buffer);
	            	  		  int value = get_shared_var(buffer);
	            	  		  send_value_to_socket(j, OK, value);

	            	  			}

	            	  		else{
	            	  			log_error(LOG, "error recibiendo nombre de shared var");
	            	  			manejar_caida_cpu(j,serverSockUMC);
	            	  		}

	            	  		free(buffer);
	            	  		break;

	            	  	  case SHARVAR_SET:


	            	  		  buffer = malloc(recvHeader.size - sizeof(int));
	            	  		  intBuffer= malloc(sizeof(int));
	            	  		  status = recv(j, buffer, recvHeader.size - sizeof(int), 0);
	            	  		  status = recv(j, intBuffer, sizeof(int), 0);
	            	  		  memcpy(&value, intBuffer, sizeof(int));

	            	  		if (status != ERROR){

	            	  		  log_info(LOG, "Se procede a setear variable compartida %s", buffer);

	            	  		  set_shared_var(buffer, value);

	            	  		  socket_ok_response(j);

	            	  			}

	            	  		else{
	            	  			log_error(LOG, "error recibiendo shared var");
	            	  			manejar_caida_cpu(j,serverSockUMC);
	            	  		}

	            	  		free(buffer);
	            	  		break;

	            	  	  case SEM_GET:
	            	  		buffer = malloc(recvHeader.size);
							status = recv(j, buffer, recvHeader.size, 0);

							if (status != ERROR){

								t_sem* semaforo = get_semaforo(buffer);
								send_value_to_socket(j, OK, semaforo->valor);

								int status = recibir_accion_semaforo(j, buffer, semaforo);

								if(status == OK){
									log_info(LOG, "Se realizó operación sobre semáforo");
								}else{
									if(status==ERROR){
										log_error(LOG, "Caída de CPU %d", j);
										manejar_caida_cpu(j, serverSockUMC);
									}
								}


								}

							else{
								log_error(LOG, "error recibiendo nombre de semaforo");
								manejar_caida_cpu(j, serverSockUMC);
							}
							free(buffer);

	            	  		  break;
	            	  /*	  case SEM_WAIT:

	            	  		buffer = malloc(recvHeader.size);
	            	  		status = recv(j, buffer, recvHeader.size, 0);

	            	  		if (status != ERROR){

	            	  			wait_semaforo(buffer);
	            	  			socket_ok_response(j);

	            	  			}

	            	  		else
	            	  			log_error(LOG, "error recibiendo nombre de semaforo");

	            	  		free(buffer);
	            	  		break;*/


	            	  	  case SEM_SIG:

	            	  		  buffer = malloc(recvHeader.size);
	            	  		  status = recv(j, buffer, recvHeader.size, 0);

	            	  		  if (status != ERROR){

		            	  		signal_semaforo(buffer);
		            	  		socket_ok_response(j);

	            	  		  }

	            	  		  else{
		            	  		log_error(LOG, "error recibiendo nombre de semaforo");
		            	  		manejar_caida_cpu(j, serverSockUMC);
	            	  		  }
	            	  		  free(buffer);
	            	  		  break;

	            	  	  default:
	            	  		  log_error(LOG, "Mensaje inesperado nro: %d de socket: %d", recvHeader.id, j);
	            	  		  socket_fallo_response(j);
	            	  }
	            	  log_info(LOG,"Se espera por algún avento");
	            	  pthread_mutex_unlock(&monitorConfigMutex);
	               }
	            }
	            else if (result == 0) {
	            	close(j);
	            	//SACO EL SOCKET PORQUE DEJO DE TENER ACTIVIDAD
	            	FD_CLR(j, &readset);
	            	t_socketConsola* consola = buscarConsola_porSocket(j);
	            	if(consola!=NULL){

	            		if(consola->pid_activo != -1){
	            		finalizar_por_pid(serverSockUMC, consola->pid_activo);

	            		log_info(LOG, "Se elimino programa %d", consola->pid_activo);
	            		}

	            		sacarConsola(consola->idSocketConsola);

	            		log_info(LOG, "Se cayó la consola %d", j);

	            	}else{
	            		if(buscarCPU(j)!= NULL){
	            			manejar_caida_cpu(j, serverSockUMC);
	            		}
	            	}
	            }
	            	else {
	            		printf("Error in recv()");
	            	}
	         }    
	      }      
	   }     
	} while (1);
	close(listenSock);
	log_destroy(LOG);
	return EXIT_SUCCESS;
}
