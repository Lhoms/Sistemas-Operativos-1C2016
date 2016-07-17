/*
 * Swap.c
 *
 *  Created on: 24/4/2016
 *      Author: Lajew
 *      Descrp: Proceso SWAP
 */

#include "Swap.h"

#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char* argv[]){

	//t_log*  LOG;
	//s_conf config;

	int listenSock;
	int socketCliente, result,num_pagina,cant_pag_pedidas,indice_espacios_libres;
	//char package[PACKAGESIZE];
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	struct s_header recvHeader;//, sendHeader;
	char* recvMessage;

	char* puntero_binario;

	pid_t pid;




	//Config + Log

	LOG = log_create("Swap.log","Swap.c",0,1);
	log_info(LOG,"\n////////////////////////////////////////////////////////////////////////comienza módulo.");

	if (argc != 2){
		log_error(LOG, "No se indicó la ruta del archivo de configuración");
		log_destroy(LOG);
		return EXIT_FAILURE;
	}

	else config = get_config(argv[1], LOG);

	//Si no recibe el puerto de escucha, no puede escuchar en ningún lado
	if (string_is_empty(config.puerto_escucha)){
		log_destroy(LOG);
		return EXIT_FAILURE;
	}


	int cant_pag = StringToNum(config.cant_pag);

	int tam_pag = StringToNum(config.tam_pag);

	if (cant_pag<=0||tam_pag<=0)
	{
		log_error(LOG, "Error: cantidad de paginas o tamanio de paginas con numero <= 0");
		log_destroy(LOG);
		return EXIT_FAILURE;
	}

	size_t tamanio_binario = (size_t) (cant_pag*tam_pag);




	FILE *archivo_binario = fopen(config.nombre_swap,"wb");

	//funcion que agranda o achica el tamanio del archivo hasta el tamanio solicitado.
	//Al agrandar llena con \0.
	//Como el archivo nuevo pesa 0 bytes, crea un archivo del tamanio solicitado lleno de \0

	ftruncate(fileno(archivo_binario), 0); //tamanio 0 para borrar el contenido si el archivo ya existia

	ftruncate(fileno(archivo_binario), tamanio_binario);

	/*



	int indice_inicializar_binario = 0;

	char caracter = '7'; //inicializo binario con \0

	while(1);

    if (archivo_binario!= NULL) {

    	for (;indice_inicializar_binario<tamanio_binario;indice_inicializar_binario++)
    	{
        if(fwrite (&caracter, sizeof (caracter), 1, archivo_binario+indice_inicializar_binario) < 0)
        {
        	log_error(LOG, "Error: no se pudo escribir el archivo binario");
        	fclose (archivo_binario);
    		log_destroy(LOG);
    		return EXIT_FAILURE;
        }
    	}

    }
    else
    {
    	log_error(LOG, "Error: no se pudo crear el archivo binario");
		log_destroy(LOG);
		return EXIT_FAILURE;
    }*/

    log_info(LOG, "Archivo binario creado");


    fclose (archivo_binario);

    int fd = open (config.nombre_swap, O_RDWR); //mmap necesita el file descriptor de open

    //se crea mapeo del archivo binario

    puntero_binario = (char *) mmap((caddr_t) 0, tamanio_binario, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (puntero_binario == MAP_FAILED)
    {
    	log_error(LOG, "Error en mmap");
    	close (fd);
		log_destroy(LOG);
		return EXIT_FAILURE;
    }
/*
    char caracter = '7';

    int indice_inicializar_binario = 0;

    for (;indice_inicializar_binario<tamanio_binario;indice_inicializar_binario++)
    {
    	memcpy(puntero_binario+indice_inicializar_binario,&caracter,1);
    }
*/





    //bitmap de paginas disponibles.



    //t_bitarray 	*bitmap = bitarray_create(char *bitarray, (size_t) cant_pag);

    /*
     * 	typedef struct {
		char *bitarray;
		size_t size;
	} t_bitarray;
     * bool 		 bitarray_test_bit(t_bitarray*, off_t bit_index);
     * void		 bitarray_set_bit(t_bitarray*, off_t bit_index);
     * void		 bitarray_clean_bit(t_bitarray*, off_t bit_index);
     * size_t		 bitarray_get_max_bit(t_bitarray*);
     * void 		 bitarray_destroy(t_bitarray*);
     */
     
     char *array_bitmap = malloc((cant_pag/8) + 1);
     t_bitarray *bitmap;
     
     bitmap = bitarray_create(array_bitmap, (size_t) (cant_pag/8) + 1);

    off_t bit_index;

    for(bit_index=0;bit_index<cant_pag;bit_index++){bitarray_clean_bit(bitmap, bit_index);}



    nodo_lista *estructura_control = NULL; //creo la lista de la estructura de control

    /*char palabra[4]= "hola";

    memcpy(puntero_binario,&palabra,4);

    char palabra_leida[256];

    memcpy(palabra_leida,puntero_binario,256);

    printf("%s\n",palabra_leida);

        while(1);*/

/*

 	//Prueba de funciones

    char palabra[20]= "pid 7 pag 0";

    indice_espacios_libres = tiene_suficientes_paginas(bitmap,1);

    reservar_memoria(bitmap,&estructura_control,7,indice_espacios_libres,1);

    almacenar_bytes(puntero_binario,&estructura_control,7,0,palabra);

    //memcpy(puntero_binario+sizeof(char)*tam_pag*(indice_espacios_libres+0), &palabra, 11);

    indice_espacios_libres = tiene_suficientes_paginas(bitmap,3);

    reservar_memoria(bitmap,&estructura_control,5,indice_espacios_libres,3);

    char palabra1[20]= "pid 5 pag 0";

    almacenar_bytes(puntero_binario,&estructura_control,5,0,palabra1);

    //memcpy(puntero_binario+sizeof(char)*tam_pag*(indice_espacios_libres+0), &palabra1, 11);

    char palabra2[20]= "pid 5 pag 1";

    almacenar_bytes(puntero_binario,&estructura_control,5,1,palabra2);

    //memcpy(puntero_binario+sizeof(char)*tam_pag*(indice_espacios_libres+1), &palabra2, 11);

    char palabra3[20]= "pid 5 pag 2";

    almacenar_bytes(puntero_binario,&estructura_control,5,2,palabra3);

    //memcpy(puntero_binario+sizeof(char)*tam_pag*(indice_espacios_libres+2), &palabra3, 11);

    indice_espacios_libres = tiene_suficientes_paginas(bitmap,1);

    reservar_memoria(bitmap,&estructura_control,6,indice_espacios_libres,1);

    char palabra4[20]= "pid 6 pag 0";

    almacenar_bytes(puntero_binario,&estructura_control,6,0,palabra4);

    //memcpy(puntero_binario+sizeof(char)*tam_pag*(indice_espacios_libres+0), &palabra4, 11);

    mostrar_lista(&estructura_control);

    mostrar_bitmap(bitmap);

    finalizar_programa(bitmap,&estructura_control,7);

    finalizar_programa(bitmap,&estructura_control,6);

    mostrar_lista(&estructura_control);

    mostrar_bitmap(bitmap);

    compactacion(bitmap,&estructura_control,puntero_binario);

    mostrar_lista(&estructura_control);

    mostrar_bitmap(bitmap);

    indice_espacios_libres = tiene_suficientes_paginas(bitmap,2);

    reservar_memoria(bitmap,&estructura_control,42,indice_espacios_libres,2);

    char palabra5[20]= "pid 42 pag 0";

    almacenar_bytes(puntero_binario,&estructura_control,42,0,palabra5);

    //memcpy(puntero_binario+sizeof(char)*tam_pag*(indice_espacios_libres+0), &palabra5, 12);

    char palabra6[20]= "pid 42 pag 1";

    almacenar_bytes(puntero_binario,&estructura_control,42,1,palabra6);

    almacenar_bytes(puntero_binario,&estructura_control,42,2,palabra6);

    //memcpy(puntero_binario+sizeof(char)*tam_pag*(indice_espacios_libres+1), &palabra6, 12);

    mostrar_lista(&estructura_control);

    mostrar_bitmap(bitmap);

    int i;

    char palabra_leida[256];

    for(i=0;i<5;i++)
    {
        memcpy(palabra_leida,puntero_binario+sizeof(char)*tam_pag*i,256);

        printf("%s\n",palabra_leida);
    }





    while(1);*/





/*

    char palabra[256]= "hola";

    char palabra_leida[256];

    int indice;

    for(indice=5;indice<256;indice++)
    {
    	palabra[indice] = '\0';
    }

    printf("%d\n",strlen(palabra));



    char *string = malloc(sizeof(char)*256);

    memcpy(string,palabra,256);

    //string[strlen(palabra)] = '\0';

    printf("%s\n",string);

	  if(almacenar_bytes(puntero_binario,&estructura_control,5,0,tam_pag,string)==ERROR)
	  {
		  printf("error\n");
	  }
	  else
	  {
		  printf("todo bien\n");
	  }



    int primer_espacio = espacio_pagina0(&estructura_control,5);

    printf("primer espacio: %d\n",primer_espacio);

    int num_pag = 0;

    int offset = tam_pag*(primer_espacio+num_pag);

    memcpy(palabra_leida,puntero_binario+offset,256);

    printf("%s\n",palabra_leida);


	  if(almacenar_bytes(puntero_binario,&estructura_control,5,1,tam_pag,"pagina1")==ERROR)
	  {
		  printf("error\n");
	  }
	  else
	  {
		  printf("todo bien\n");
	  }



  primer_espacio = espacio_pagina0(&estructura_control,5);

  printf("primer espacio: %d\n",primer_espacio);

  num_pag = 1;

  offset = tam_pag*(primer_espacio+num_pag);

  memcpy(palabra_leida,puntero_binario+offset,256);

  printf("%s\n",palabra_leida);


  if(almacenar_bytes(puntero_binario,&estructura_control,7,0,tam_pag,"pid 7 pag 0")==ERROR)
  {
	  printf("error\n");
  }
  else
  {
	  printf("todo bien\n");
  }



primer_espacio = espacio_pagina0(&estructura_control,7);

printf("primer espacio: %d\n",primer_espacio);

num_pag = 0;

offset = tam_pag*(primer_espacio+num_pag);

memcpy(palabra_leida,puntero_binario+offset,256);

printf("%s\n",palabra_leida);




    while(1);*/




    	//Creo el socket.
    	listenSock = create_socket_servidor(config.puerto_escucha, LOG);
    	//len = sizeof(addr);
    	listen(listenSock,BACKLOG);
    	socketCliente=accept(listenSock,(struct sockaddr*)&addr,&addrlen);
        if (socketCliente < 0) {
           printf("Error en accept()");
           log_error(LOG, "Error: Error en Accept");
           close (fd);
           log_destroy(LOG);
           close(listenSock);
           bitarray_destroy(bitmap);
           return EXIT_FAILURE;
        }

        log_info(LOG, "Socket conectado");

    	//recv(socketCliente, &package, PACKAGESIZE, 0);

        while(1) //loop para recibir y enviar mensajes a UMC
        {
        do{
        	init_struct_header(&recvHeader);
        	result = recv(socketCliente, &recvHeader, sizeof(struct s_header), MSG_WAITALL);
        }while(result == ERROR);

        if(result == 0)
        {
            printf("UMC desconectado\n");
            log_error(LOG, "Cliente desconectado");
            close (fd);
            log_destroy(LOG);
            close(listenSock);
            close(socketCliente);
            bitarray_destroy(bitmap);
            return EXIT_FAILURE;
        }

        switch(recvHeader.id)
        {
        	case SALUDO_UMC:
        	
        		//Respondo saludo con OK

        		socket_response(socketCliente,OK);
        		
        		log_info(LOG, "Recibido saludo de UMC, se respondio con OK");

        		break;

        	case RESERVAR_MEMORIA: //preguntaron si tiene x cantidad de paginas libres

        		//hay que ver si se dispone del espacio disponible
        		//UMC envia en el payload pid (pid_t) y cantidad de paginas (int)

        		log_info(LOG, "se pide reservar memoria");

        		cant_pag_pedidas = 0;

        		do{
        		    result = recv(socketCliente, &pid, sizeof(pid_t), MSG_WAITALL);
        		}while(result == ERROR);

        		log_info(LOG, "para el pid %d", pid);

        		do{
        		    result = recv(socketCliente, &cant_pag_pedidas, sizeof(int), MSG_WAITALL);
        		}while(result == ERROR);

        		log_info(LOG, "cantidad de paginas %d", cant_pag_pedidas);


        		//comprobacion si ya se habia reservado memoria para dicho pid
        		//Esta todo bien si al buscar el pid en la estructura da error por no encontrarlo
        		//En caso contrario se rechaza el pedido porque no esta permitida la solicitud de mas paginas
/*        		if(espacio_pagina0(&estructura_control,pid)!=ERROR)
        		{
        			log_error(LOG, "Se pidio reservar memoria para un pid ya existente");
        			printf("Error: Se pidio reservar memoria para un pid ya existente\n");
        			socket_response(socketCliente,FALLO);
        			break;
        		}

*/
        		log_info(LOG, string_from_format("Se busca espacio contiguo libre de %d paginas para el pid %d", cant_pag_pedidas, pid));

        		indice_espacios_libres = tiene_suficientes_paginas(bitmap,cant_pag_pedidas);

        		if(indice_espacios_libres == NECESITA_COMPACTACION)
        		{
        			log_info(LOG,"Se posee suficiente espacio libre pero esta fragmentado, entonces se procede a compactar");
        			indice_espacios_libres = compactacion(bitmap,&estructura_control,puntero_binario);

        			//indice_espacios_libres = NO_TIENE_SUFICIENTES_PAGINAS;//usar esta linea si compactacion no funciona

        		}

        		if(indice_espacios_libres == NO_TIENE_SUFICIENTES_PAGINAS)	//-1 es no tiene suficientes paginas
        		{
           			printf("No hay memoria suficiente para almacenar al programa %d\n", pid);
            		log_error(LOG, string_from_format("Paginas disponibles son insuficientes para pedido de UMC, se rechaza el pedido de pid %d", pid));
            		log_info(LOG, "Se responde FALLO por socket");
        			socket_response(socketCliente,FALLO);

        		}



        		if(indice_espacios_libres >= 0)
        		{
        			//socket_response(socketCliente,SUFICIENTES_PAGINAS);
              		log_info(LOG, "Paginas disponibles son suficientes para pedido de UMC");

              		//hay que reservar la memoria
            		  if(reservar_memoria(bitmap,&estructura_control,pid,indice_espacios_libres,cant_pag_pedidas)==ERROR)
            		  {
            			  log_error(LOG, string_from_format("Error al reservar %d paginas para proceso con pid %d",cant_pag_pedidas,pid));
            			  log_info(LOG, "Se responde FALLO por socket");
            			  socket_response(socketCliente,FALLO);

            		  }
            		  else
            		  {
            			  socket_response(socketCliente,OK);
            			  log_info(LOG, "Se envio OK a UMC");
            		  }


        		}




        		break;

        	case ALMACENAR_BYTES:


        		//El payload va a tener pid, numero de la pagina y el contenido a guardar correspondiente a dicha pagina
        		//Se utiliza para cargar el programa en swap por primera vez
        		//Tambien se utiliza para sobreescribir contenido ya existente

        	log_info(LOG, "Recibido solicitud de guardar programa");

        	//log_info(LOG,"Retardo de acceso");
        	usleep(StringToNum(config.retardo_acceso)*1000);

        	if((recvHeader.size)-8>StringToNum(config.tam_pag))
        	{
        		//me manda el contenido a almacenar en una pagina
        		//Asi que dicho contenido no puede ser mayor al tamanio de una pagina
        		socket_response(socketCliente,FALLO);
        		log_error(LOG, "Error se envio tamanio mayor a una pagina");
        		log_info(LOG, "Se rechaza ALMACENAR_BYTES");
        		break;
        		
        	}

    		do{
    		    result = recv(socketCliente, &pid, sizeof(pid_t), MSG_WAITALL);
    		}while(result == ERROR);

    		do{
    		    result = recv(socketCliente, &num_pagina, sizeof(int), MSG_WAITALL);
    		}while(result == ERROR);

        		//Se recibe archivo ansisop
  	  		  recvMessage = malloc(recvHeader.size);

      		do{
      			recv(socketCliente, recvMessage, (recvHeader.size)-(sizeof(pid_t)+sizeof(int)), MSG_WAITALL);
      		}while(result == ERROR);


  	  		log_info(LOG, "Programa recibido");

  	  		//Ahora debe asignar el programa en una pagina/s

  	  		//debe guardar el programa en el archivo binario

  	  		//usar mmap

  		  if(almacenar_bytes(puntero_binario,&estructura_control,pid,num_pagina,recvMessage)==ERROR)
  		  {
  			 log_error(LOG, "Fallo al almacenar");
  			 log_info(LOG, "Se responde FALLO por socket");
  			 socket_response(socketCliente,FALLO);
  		  }
  		  else
  		  {
  			  log_info(LOG, "Se almaceno perfectamente");
  			  log_info(LOG, "Se responde OK por socket");
  			  socket_response(socketCliente,OK);
  		  }

  		free(recvMessage);





	  		  break;

        	case FINALIZAR_PROGRAMA:

        		//debe eliminar el proceso de la lista y marcar la pagina como libre
        		//el payload solo tiene el pid.
        		//hay que buscar en la lista por pid

        		//usar int quitar_elemento_lista (nodo_lista **lista, pid_t pid_quitar)
        		//usar void		 bitarray_clean_bit(t_bitarray*, off_t bit_index);

        		log_info(LOG, "Recibido solicitud de finalizar programa");

        		do{
        		    result = recv(socketCliente, &pid, sizeof(pid_t), MSG_WAITALL);
        		}while(result == ERROR);

        		log_info(LOG, "se finaliza pid %d", pid);


        		  if(finalizar_programa(bitmap,&estructura_control,pid)==ERROR)
        		  {
        			  log_error(LOG, "Fallo al finalizar");
        			  log_info(LOG, "Se responde FALLO por socket");
        			  socket_response(socketCliente,FALLO);

        		  }
        		  else
        		  {
           			  log_info(LOG, "se finalizo pid %d", pid);
        			  log_info(LOG, "Se responde OK por socket");
        			  socket_response(socketCliente,OK);
         		  }




        		break;

        	case SOLICITAR_BYTES: //pedido lectura

        		//El payload tiene pid y numero de pagina deseada.
        		//La numeracion de pagina es local a cada pid.
        		//Es decir que para todos los pid la primer pagina es la 0.

        		//debe leer el archivo binario y devolverlo

        		log_info(LOG, "Recibido solicitud de envio de contenido de pagina");

        		//log_info(LOG,"Retardo de acceso");
            	usleep(StringToNum(config.retardo_acceso)*1000);

        		do{
        		    result = recv(socketCliente, &pid, sizeof(pid_t), MSG_WAITALL);
        		}while(result == ERROR);

        		do{
        		    result = recv(socketCliente, &num_pagina, sizeof(int), MSG_WAITALL);
        		}while(result == ERROR);


     		  if(enviar_pagina(puntero_binario,socketCliente,&estructura_control,pid,num_pagina)==ERROR)
     		  {
      			  log_error(LOG, "Fallo al enviar pagina");
      			  log_info(LOG, "Se responde FALLO por socket");
      			  socket_response(socketCliente,FALLO);
      		  }
      		  else
      		  {
      			  log_info(LOG, "Se envio pagina correctamente");
      		  }



        		break;
        	default:
        	  printf("El ID recibido es incorrecto\n");
        	  log_error(LOG, "El ID recibido es incorrecto");
        }
        }//fin while(1)

}


