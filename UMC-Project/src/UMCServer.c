/*
 * UMCServer.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#include "UMC.h"

void* main_server(void* arg){

	int socketSwap;
	int listenSock;
	struct sockaddr_in addr;
	struct s_protocol sender;

	int peersock;

	fd_set readset, tempset;
	int maxfd, flags;
	int srvsock, peersoc, j, result, result1, sent, len;
	socklen_t addrlen = sizeof(addr);

	//Creo Socket escucha(nucleo) y socket para reehvio(swap)
	pthread_mutex_lock( &mutex_config );
		listenSock = create_socket_servidor(config.PUERTO, LOG);
		socketSwap = create_socket_cliente(config.IP_SWAP, config.PUERTO_SWAP, LOG);
	pthread_mutex_unlock( &mutex_config );

	log_info(LOG, "Se realiza el handshake con SWAP");
	int handshakeSwap = realize_handshake(socketSwap, LOG);

	log_info(LOG, "Iniciando escucha en UMC");

	init_struct_protocol(&sender);
	listen(listenSock, BACKLOG);

	// Inicialización de hilos
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	//INICIALIZO SET DE SOCKETS Y LE AGREGO EL SOCKET ESCUCHA
	FD_ZERO(&readset);
	FD_SET(listenSock, &readset);
	maxfd = listenSock;

	do {
		 //Acepto conexiones entrantes
		 peersock = accept(listenSock, (struct sockaddr*)&addr, &addrlen);//&len);
		 if (peersock >= 0) {
			//Creo un hilo a cada conexión entrante.
			struct s_arg_thread arg_thread;

			arg_thread.socket = peersock;
			arg_thread.socketSwap = socketSwap;
			arg_thread.handshakeSwap = handshakeSwap;

			pthread_t threads;
			pthread_create(&threads, &attr, &threads_server, (void*)&arg_thread);
		 }
	} while (1);

	pthread_exit(NULL);
    close(listenSock);
	close(socketSwap);
}

void* threads_server(void* arg){

	  struct s_header recvHeader;
	  void* recvData;
	  int   result;
	  struct s_arg_thread data;
	  memcpy(&data, arg, sizeof(struct s_arg_thread));

	  int status = 0;
	  int pid = 0;

	  do{
		  init_struct_header(&recvHeader);
		  result = recv(data.socket, &recvHeader, sizeof(struct s_header), MSG_WAITALL);

		  if (result > 0){
			  switch(recvHeader.id){
			  //Respondo OK a los saludos
				  case SALUDO_NUCLEO:
					  socket_ok_response(data.socket);
					  status = send_page_size_to_socket(data.socket, config.MARCO_SIZE);
					  if (status == ERROR){
						  log_error(LOG, "Error al enviar tamaño de página al núcleo");
					  }else{
						  if (status == OK){
							  log_info(LOG, string_from_format("Se envió el tamaño de página %d al núcleo", config.MARCO_SIZE));
						  }
					  }
					  break;
				  case SALUDO_CPU:
					  socket_ok_response(data.socket);
					  add_cpu_to_list(data.socket);
					  status = send_page_size_to_socket(data.socket, config.MARCO_SIZE);
					  if (status == ERROR){
						  log_error(LOG, string_from_format("Error al enviar tamaño de página al CPU %d", data.socket));
					  }else{
						  if (status == OK){
							  log_info(LOG, string_from_format("Se envió el tamaño de página %d al CPU %d", config.MARCO_SIZE, data.socket));
						  }
					  }
					  break;
				  case CAMBIO_PROCESO_ACTIVO:
					  pid = 0;
					  log_info(LOG, "Se solicitó un cambio de proceso activo para la CPU %d", data.socket);
					  status = recv(data.socket, &pid, recvHeader.size, 0);
					  if(status!=ERROR){
						  status = change_cpu_program(data.socket, pid);

						  if(status == ERROR){
							  log_error(LOG, string_from_format("Error al modificar CPU %d con PID %d", data.socket, pid));
							  socket_fallo_response(data.socket);
						  }else{
							  log_info(LOG, string_from_format("Modificación exitosa de CPU %d con PID %d", data.socket, pid));
							  socket_ok_response(data.socket);

							  if( config.ENTRADAS_TLB > 0) {
								  log_info(LOG, "Se limpia información de TLB sobre PID anterior: %d", status);
								  clear_tlb_from_pid(status);
							  }
						  }
					  }else{
						  s_cpu* cpu = search_cpu(data.socket);
						  if (cpu != NULL){
							  destroy_cpu(data.socket);
							  log_info(LOG, string_from_format("Se cayó la CPU %d", data.socket));
						  }
					  }
					  break;
				  case INICIAR_PROGRAMA:
					  //Inicio de un programa, recibo el resto del mensaje y lo deserializo.
					  recvData = malloc(recvHeader.size);
					  recv(data.socket, recvData, recvHeader.size, 0);

					  //Deserializo respuesta
					  struct s_init_program init_program;
					  deserialize_init_program(&init_program, (recvHeader.size - (2*sizeof(int))),recvData);
					  log_info(LOG,string_from_format("Recibo de Nucleo: PID %d | CantPaginas %d\n ANSISOP:\n %s",init_program.pid,init_program.cant_pag,init_program.codigo));

					  //Pido memoria para reservar a SWAP - Si no falló el handshake
					  if (data.handshakeSwap == OK){
						  log_info(LOG, "Pido reserva de memoria para el PID %d", init_program.pid);
						  status = request_memory_to_SWAP(data.socketSwap, init_program.pid, init_program.cant_pag);
						  if (status == ERROR){
							  log_error(LOG, "Se cayó SWAP mientras se solicitaba memoria");
							  socket_fallo_response(data.socket);
						  }else{
							  if (status == FALLO){
								log_error(LOG, "No hay memoria para el proceso %d", init_program.pid);
								socket_fallo_response(data.socket);
							  }else{
								log_info(LOG, "SWAP informa que reservó la memoria");
								//Armo páginas del proceso.
								build_pages_for_process(init_program.pid, init_program.cant_pag);
								//Envío páginas a SWAP
								status = split_and_send_code(data.socketSwap, init_program.pid, config.MARCO_SIZE, init_program.codigo);
								if (status == ERROR || status == FALLO){
									log_error(LOG, string_from_format("Hubo errores al enviar páginas del pid=%d al proceso SWAP", init_program.pid));
									socket_fallo_response(data.socket);
								}else{
									log_info(LOG, string_from_format("Se enviaron correctamente las páginas del pid=%d al proceso SWAP", init_program.pid));
									socket_ok_response(data.socket);
								}
							  }
						  }
					  }else{
						  //Como falló el handshake, respondo fallo
						  socket_fallo_response(data.socket);
					  }

					  free(init_program.codigo);
					  free(recvData);
					  break;
				  case FINALIZAR_PROGRAMA:
					  log_info(LOG,"Se recibe finalizar programa");
					  pid = 0;
					  //Recibo el PID a eliminar
					  recv(data.socket, &pid, recvHeader.size, 0);
					  log_info(LOG,"Se va a finalizar pid %d", pid);
					  //Limpio memoria sobre ese PID.
					  log_info(LOG, "Se limpia información en memoria sobre PID %d", pid);
					  status = clear_all_memory_from_pid(pid);
					  if(status==OK){
						  //Le aviso a SWAP para que también limpie
						  status = send_fin_to_SWAP(data.socketSwap, pid);
						  if (status == ERROR){
							  log_error(LOG, string_from_format("Error en la comunicación con SWAP para finalizar PID: %d", pid));
							  socket_fallo_response(data.socket);
						  }else{
							  if (status == OK){
								  log_info(LOG, string_from_format("Se finalizó correctamente el PID: %d en SWAP", pid));
								  socket_ok_response(data.socket);
							  }else{
								  log_error(LOG, string_from_format("Error finalizar PID: %d en SWAP", pid));
								  socket_fallo_response(data.socket);
							  }
						  }
					  }else{
						  log_info(LOG, "El PID %d ya fue finalizado anteriormente o nunca se inició", pid);
						  socket_ok_response(data.socket);
					  }
					  //pthread_mutex_unlock( &mutex_swap );
					  //log_info(LOG, "Se pide finalizar programa. Se desbloquea llamadas a SWAP");
					  break;
				  case SOLICITAR_BYTES:
					  //Solicitan bytes. Recibo el resto de la solicitud.
					  recvData = malloc(recvHeader.size);
					  status = recv(data.socket, recvData, recvHeader.size, 0);
					  if(status != ERROR){
						  //Deserializo la solicitud.
						  struct s_req_bytes req_bytes;
						  deserialize_req_bytes(&req_bytes, recvData);

						  //Busco PID asociado al CPU que mandó la solicitud
						  pid = search_pid_from_cpu(data.socket);

						  log_info(LOG, "Se solicita información de la CPU %d sobre: Proceso %d; Página: %d; OFFSET: %d; Tamaño: %d", data.socket, pid, req_bytes.pagina, req_bytes.offset, req_bytes.tamanio);
						  //Valido que la solicitud sea correcta
						  status = check_request_is_valid(&req_bytes, pid);

						  if(status == OK){
							  //Administro la solicitud.
							  char* resp = malloc(req_bytes.tamanio); //Bytes encontrados
							  status = request_bytes(&req_bytes, pid, data.socketSwap, resp);

							  if(status == OK){
								 socket_bytes_response(data.socket, req_bytes.tamanio, resp);
							  }else{
								 if (status == NO_HAY_MEMORIA){
									 //Le aviso a CPU que no hay memoria.
									  socket_error_response(data.socket, status);
									 //Limpio memoria sobre ese PID(Abortar programa!).
									  //clear_all_memory_from_pid(pid);
									  //Le aviso a SWAP para que también limpie
	/*								  status = send_fin_to_SWAP(data.socketSwap, pid);
									  if (status == ERROR){
										  log_error(LOG, string_from_format("Error en la comunicación con SWAP para finalizar PID: %d", pid));
									  }else{
										  if (status == OK){
											  log_info(LOG, string_from_format("Se finalizó correctamente el PID: %d en SWAP", pid));
										  }else{
											  log_error(LOG, string_from_format("Error finalizar PID: %d en SWAP", pid));
										  }
									  }
	*/							 }else{
									 socket_fallo_response(data.socket);
								 }
							  }

							  free(resp);
						  }else{
							  socket_error_response(data.socket, PEDIDO_INVALIDO);
						  }
					  }else{
						  s_cpu* cpu = search_cpu(data.socket);
						  if (cpu != NULL){
							  destroy_cpu(data.socket);
							  log_info(LOG, string_from_format("Se cayó la CPU %d", data.socket));
						  }
					  }
					  free(recvData);
					  break;
				  case ALMACENAR_BYTES:
					  //Solicitan almacenar bytes. Recibo el resto de la solicitud.
					  recvData = malloc(recvHeader.size);
					  status = recv(data.socket, recvData, recvHeader.size, 0);
					  if (status != ERROR){
						  //Deserializo la solicitud.
						  char* bytes = string_new();
						  struct s_req_bytes save_bytes_;
						  int size = recvHeader.size - (3 * sizeof(int));
						  deserialize_save_bytes(&save_bytes_, bytes, size, recvData);

						  //Busco PID asociado al CPU que mandó la solicitud
						  pid = search_pid_from_cpu(data.socket);

						  //Valido que la solicitud sea correcta
						  status = check_request_is_valid(&save_bytes_, pid);

						  log_info(LOG, "Se solicita grabar información desde la CPU %d sobre: Proceso %d; Página: %d; OFFSET: %d; Tamaño: %d", data.socket, pid, save_bytes_.pagina, save_bytes_.offset, save_bytes_.tamanio);

						  if (status == OK){

							  status = save_bytes(&save_bytes_, pid, data.socketSwap, bytes);

							  if(status == OK){
								 socket_ok_response(data.socket);
							  }else{
								 if (status == NO_HAY_MEMORIA){
									 //Le aviso a CPU que no hay memoria, hay que abortar el programa
									  socket_error_response(data.socket, status);
									 //Limpio memoria sobre ese PID(Abortar programa!).
								/*	  clear_all_memory_from_pid(pid);
									  //Le aviso a SWAP para que también limpie
									  status = send_fin_to_SWAP(data.socketSwap, pid);
									  if (status == ERROR){
										  log_error(LOG, string_from_format("Error en la comunicación con SWAP para finalizar PID: %d", pid));
									  }else{
										  if (status == OK){
											  log_info(LOG, string_from_format("Se finalizó correctamente el PID: %d en SWAP", pid));
										  }else{
											  log_error(LOG, string_from_format("Error finalizar PID: %d en SWAP", pid));
										  }
									  }*/
								 }else{
									 socket_fallo_response(data.socket);
								 }
							  }
						  }else{
							  socket_error_response(data.socket, PEDIDO_INVALIDO);
						  }

					  }else{
						  s_cpu* cpu = search_cpu(data.socket);
						  if (cpu != NULL){
							  destroy_cpu(data.socket);
							  log_info(LOG, string_from_format("Se cayó la CPU %d", data.socket));
						  }
					  }
					  free(recvData);
					  break;
				  default:
					  //No sé quién sos ni qué me estás mandando.
					  log_error(LOG, "Mensaje inesperado");
					  socket_fallo_response(data.socket);
			  }
		  }
	  }while(result != 0);

	  s_cpu* cpu = search_cpu(data.socket);
	  if (cpu != NULL){
		  destroy_cpu(data.socket);
		  log_info(LOG, string_from_format("Se cayó la CPU %d", data.socket));
	  }

	  close(data.socket);
}
