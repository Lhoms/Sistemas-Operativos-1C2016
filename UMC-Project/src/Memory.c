/*
 * Memory.c
 *
 *  Created on: 23/5/2016
 *      Author: utnso
 *      Descr:  Manejo de meemoria
 */

//#include "Memory.h"
#include "UMC.h"

int create_struct_memory(){
	//Creación de memoria principal.
	create_mp();
	if(memory==NULL){
		return EXIT_FAILURE;
	}

	//Creación de TLB-> Sólo si está habilitada.
	if(config.ENTRADAS_TLB!=0){
		create_tlb();
	}

	//Creación tabla de páginas.
	create_page_table();

	//Creación de marcos;
	create_frames();

	return EXIT_SUCCESS;
}

void destroy_struct_memory(){

	//Destruyo memoria principal.
	destroy_mp();

	//Destruyo TLB si está habilitada
	if(config.ENTRADAS_TLB!=0){
		destroy_tlb();
	}

	//Destruyo tabla de páginas
	destroy_page_table();

	//Destruyo marcos;
	destroy_frames();

}
void create_mp(){

	int tam = config.MARCOS * config.MARCO_SIZE;
	log_info(LOG, "Se crea memoria principal de tamaño %d", tam);

	memory = malloc(tam);

	char* init = string_repeat('\0',tam);

	memcpy(memory, init, tam);
}

void destroy_mp(){
	log_info(LOG, "Se destruye memoria principal");
	free(memory);
}

void create_tlb(){
	log_info(LOG, "Se crea TLB");
	tlb = list_create();
	init_tlb();
}

void destroy_tlb(){
	log_info(LOG, "Se elimina TLB y sus elementos.");
	pthread_mutex_lock( &mutex_tlb );
		list_destroy_and_destroy_elements(tlb, free);
	pthread_mutex_unlock( &mutex_tlb );
}

void clear_tlb_from_pid(int pid){

	void clear(void*parametro){
		s_tlb* entry = (s_tlb*) parametro;

		if (entry->pid == pid){
			log_info(LOG, "Se limpia información en la TLB sobre la página %d del proceso %d con referencia a marco %d", entry->pagina, pid, entry->marco);
			entry->marco = 0;
			entry->pagina = 0;
			entry->pid = 0;
			entry->memory_flag = 0;
			entry->modif_flag = 0;
			entry->ref_time = 0;
		}
	}

	log_info(LOG, "Bloqueo de TLB para limpieza sobre PID %d", pid);
	pthread_mutex_lock( &mutex_tlb );
		list_iterate(tlb, clear);
	pthread_mutex_unlock( &mutex_tlb );
	log_info(LOG, "Desbloqueo de TLB para limpieza sobre PID %d", pid);
}

//Único lugar donde se debería hacer add a esta lista(ya que es fijo el tamaño)
void init_tlb(){
	int i;

	log_info(LOG, "Se inicializa la TLB con %d entradas", config.ENTRADAS_TLB);
	for(i=0; i < config.ENTRADAS_TLB; i++){
		s_tlb* data = malloc(sizeof(s_tlb));

		data->marco = 0;
		data->pagina = 0;
		data->pid = 0;
		data->modif_flag = 0;
		data->memory_flag = 0;
		data->ref_time = 0;

		pthread_mutex_lock( &mutex_tlb );
			list_add(tlb,data);
		pthread_mutex_unlock( &mutex_tlb );
	}
}

void clean_tlb(){

	void clear_entry(void* entry){
		s_tlb* act = (s_tlb*) entry;

		log_info(LOG, "Se limpia registro de la TLB. Marco: %d; Pagina: %d; PID: %d; BIT Modificado: %d; BIT Presencia: %d; Tiempo Referencia: %d",
				act->marco, act->pagina, act->pid, act->modif_flag, act->memory_flag, act->ref_time);

		act->marco = 0;
		act->pagina = 0;
		act->pid = 0;
		act->modif_flag = 0;
		act->memory_flag = 0;
		act->ref_time = 0;

	}

	//La función que itera dentro de la lista es la local(clear_entry)
	pthread_mutex_lock( &mutex_tlb );
		list_iterate(tlb, clear_entry);
	pthread_mutex_unlock( &mutex_tlb );

}

void create_page_table(){
	log_info(LOG, "Se crea tabla de páginas");
	process_tpage = list_create();
}

void destroy_page_table(){

	void delete_page_table(void* entry){
		s_process* process = (s_process*) entry;

		log_info(LOG, "Se destruye tabla de páginas del proceso %d", process->pid);
		list_destroy_and_destroy_elements(process->pages, free);
	}

	pthread_mutex_lock( &mutex_page_table );
		list_iterate(process_tpage, delete_page_table);
		list_destroy_and_destroy_elements(process_tpage, free);
	pthread_mutex_unlock( &mutex_page_table );
}

int destroy_page_table_from_pid(int pid){

	bool search(void*parametro){
		s_process* entry = (s_process*) parametro;
		return entry->pid == pid;
	}

	void clear_pages_on_frame(void* parametro){
		s_page_table* page = (s_page_table*) parametro;

		if(page->memory_flag == 1){
			frames[page->marco] = 0;

			log_info(LOG, "La página %d del proceso %d está en memoria actualmente en el marco. Se limpia entrada", page->pagina, pid, page->marco);
			//log_info(LOG, "Se bloquea memoria para limpieza de marco %d", page->marco);
			//pthread_mutex_lock(&mutex_memory);
				char* init = string_repeat('\0',config.MARCO_SIZE);
				int offset = page->marco * config.MARCO_SIZE;

				memcpy(memory+offset, init, config.MARCO_SIZE);
			//pthread_mutex_unlock(&mutex_memory);
			//log_info(LOG, "Se desbloquea memoria para limpieza de marco %d", page->marco);
		}
	}

	int ret = -1;

	log_info(LOG, "Se bloquea memoria para limpieza de marcos asignados por proceso");
	pthread_mutex_lock(&mutex_memory);

	log_info(LOG, "Bloqueo de tabla de páginas para limpieza de PID: %d", pid);
	pthread_mutex_lock( &mutex_page_table );
		s_process* process = find_page_table_from_pid(pid);

		if(process != NULL){
			list_iterate(process->pages, clear_pages_on_frame);
			list_destroy_and_destroy_elements(process->pages, free);
			list_remove_and_destroy_by_condition(process_tpage, search , free);

			ret = OK;
		}else{
			ret = ERROR;
		}
	pthread_mutex_unlock( &mutex_page_table );
	log_info(LOG, "Desbloqueo de tabla de páginas para limpieza de PID: %d", pid);

	pthread_mutex_unlock(&mutex_memory);
	log_info(LOG, "Se desbloquea memoria para limpieza de marcos asignados por proceso");

	return ret;
}

void create_frames() {
	log_info(LOG, "Se crea vector de %d marcos en memoria", config.MARCOS);
	frames = malloc(sizeof(int)*config.MARCOS);
	init_frames();
}

void init_frames() {

	int i;
	log_info(LOG, "Se inicializan los %d marcos como libres", config.MARCOS);
	for(i=0; i < config.MARCOS ;i++) {
		frames[i] = 0;
	}
}

int get_first_free_frame(){
	int i;

	for(i=0; i < config.MARCOS; i++){
		if (frames[i]==0){
			log_info(LOG, "Marco %d está libre", i);
			return i;
		}
	}

	return -1;
}

void destroy_frames() {
	log_info(LOG, "Se elimina información sobre marcos de memoria");
	pthread_mutex_lock( &mutex_frames );
		free(frames);
	pthread_mutex_unlock( &mutex_frames );
}

void create_process_page_table(int pid){
	int i;

	s_process* new = malloc(sizeof(s_process));

	new->pid = pid;
	new->pages = list_create();

	for(i=0; i < config.MARCO_X_PROC; i++){
		s_page_table* page = malloc(sizeof(s_page_table));

		page->marco = 0;
		page->pagina = i;
		page->bituso = 0;
		page->memory_flag = 0;
		page->modif_flag = 0;
		page->puntero = 0;
		page->ref_time = 0;

		list_add(new->pages, page);
	}

	pthread_mutex_lock( &mutex_page_table );
		list_add(process_tpage, new);
	pthread_mutex_unlock( &mutex_page_table );

}

int  clear_all_memory_from_pid(int pid){

	if(config.ENTRADAS_TLB > 0){
		log_info(LOG, "TLB activa. Se limpia información sobre PID: %d", pid);
		clear_tlb_from_pid(pid);
	}

	log_info(LOG, "Se elimina información sobre el PID %d en tabla de páginas y memoria", pid);
	return destroy_page_table_from_pid(pid);

}

s_process* find_page_table_from_pid(int pid){

	bool search(void*parametro){
		s_process* entry = (s_process*) parametro;
		return entry->pid == pid;
	}

	return list_find(process_tpage, search);

}

void add_page_to_tlb(int socketSwap, s_page_table* page, int pid, e_operacion oper){

	bool search(void* parametro){
		s_tlb* entry = (s_tlb*) parametro;
		return entry->pid == 0;
	}

	bool sort(void* parametro1, void* parametro2){
		s_tlb* entry1 = (s_tlb*) parametro1;
		s_tlb* entry2 = (s_tlb*) parametro2;

		return entry1->ref_time < entry2->ref_time;
	}

	log_info(LOG, "Se carga página en TLB");
	log_info(LOG, "Se bloquea TLB para carga de página");
	//Busco si hay entradas libres, si no hay elijo cuál actualizar por LRU y sino actualizo la primera que encuentro
	pthread_mutex_lock( &mutex_tlb );
		s_tlb* tlb_entry = list_find(tlb, search);

		if(tlb_entry != NULL){
			log_info(LOG, "Hay entrada libre en TLB, se carga página");
			//Hay entrada libre
			tlb_entry->marco = page->marco;
			tlb_entry->pagina = page->pagina;
			tlb_entry->pid = pid;
			tlb_entry->memory_flag = 1;
			if(oper == LECTURA) tlb_entry->modif_flag = 0;
			if(oper == ESCRITURA) tlb_entry->modif_flag = 1;
			tlb_entry->ref_time = get_actual_time_integer();

		}else{
			//No hay entrada libre - Ordeno y reemplazo la última
			log_info(LOG, "No hay entrada libre en TLB, se elige página a reemplazar");
			list_sort(tlb, sort);

			// LOGGEAR y ver si hay que actualizar SWAP
				tlb_entry = list_get(tlb, config.ENTRADAS_TLB - 1);
				if(tlb_entry != NULL){
					log_info(LOG, "Se reemplaza entrada: PID:%d; MARCO:%d; PAGINA:%d", tlb_entry->pid, tlb_entry->marco, tlb_entry->pagina);
					if(tlb_entry->modif_flag == 1){
						log_info(LOG, "Envío actualización a SWAP sobre la página modificada");
						char* content = malloc(config.MARCO_SIZE);//string_new();

						int offset = ( config.MARCO_SIZE * tlb_entry->marco );
						memcpy(content, memory+offset, config.MARCO_SIZE);
						int status = update_page_in_SWAP(socketSwap, pid, tlb_entry->pagina, content);

						free(content);
					}
				}
			// LOGGEAR

			s_tlb* new_entry = malloc(sizeof(s_tlb));

			new_entry->marco = page->marco;
			new_entry->pagina = page->pagina;
			new_entry->pid = pid;
			new_entry->memory_flag = 1;
			if(oper == LECTURA) new_entry->modif_flag = 0;
			if(oper == ESCRITURA) new_entry->modif_flag = 1;
			new_entry->ref_time = get_actual_time_integer();

			list_replace_and_destroy_element(tlb, config.ENTRADAS_TLB - 1, new_entry, free);
		}
	pthread_mutex_unlock( &mutex_tlb );
	log_info(LOG, "Se desbloquea TLB para carga de página");
}

s_tlb* find_process_in_tlb(int pid){

	bool search(void*parametro){
		s_tlb* entry = (s_tlb*) parametro;
		return entry->pid == pid;
	}

	pthread_mutex_lock( &mutex_tlb );
		s_tlb* entry2 = list_find(tlb, search);
	pthread_mutex_unlock( &mutex_tlb );

	return entry2;
}

s_tlb* find_process_page_in_tlb(int pid, int page){

	bool search(void*parametro){
		s_tlb* entry = (s_tlb*) parametro;
		return entry->pid == pid && entry->pagina == page;
	}

	log_info(LOG, "Se bloquea TLB para búsqueda de PID: %d Página: %d", pid, page);
	pthread_mutex_lock( &mutex_tlb );
		s_tlb* entry2 =  list_find(tlb, search);
	pthread_mutex_unlock( &mutex_tlb );
	log_info(LOG, "Se desbloquea TLB para búsqueda de PID: %d Página: %d", pid, page);

	return entry2;
}

s_page_table* get_page_from_process(s_process* process, int page){

	bool search(void* parametro){
		s_page_table* pageAct = (s_page_table*) parametro;
		return (pageAct->pagina == page);
	}

	return list_find(process->pages, search);
}

void clear_pointer_to_pages(t_list* pages){

	void clear(void*parametro){
		s_page_table* pageAct = (s_page_table*) parametro;
		pageAct->puntero = 0;
	}

	list_iterate(pages, clear);
}

void set_page_as_present(int socketSwap, int pid, int frame, int pagina, s_page_table* page, e_operacion oper){

	page->memory_flag = 1;
	page->marco = frame;
	page->puntero = 1;
	page->bituso = 0;
	page->ref_time = get_actual_time_integer();

	//Además de ponerla presente, actualizo TLB(si está habilitada).
	if (config.ENTRADAS_TLB > 0){
		log_info(LOG, "Se actualiza información a TLB");
		add_page_to_tlb(socketSwap, page, pid, oper);
	}
}

void save_in_memory(int socketSwap, int pid, int pagina, int frame, char* bytes, e_operacion oper){

	log_info(LOG, "Se guarda página %d del proceso %d en el marco %d. Contenido: %s", pagina, pid, frame, bytes);
	int offset = ( config.MARCO_SIZE * frame );
	memcpy(memory+offset, bytes, config.MARCO_SIZE);
	//.2.2) Se marca el frame como ocupado
	log_info(LOG, "Se marca el frame como ocupado");
	frames[frame] = 1;
	//.2.3) Se marca la página como presente en tabla de páginas

	s_process* process = find_page_table_from_pid(pid);
	s_page_table* page = get_page_from_process(process, pagina);
	log_info(LOG, "Se marca la página como presente en memoria");
	set_page_as_present(socketSwap, pid, frame, pagina, page, oper);

}

void clear_page_in_tlb(int pid, int pagina){

	bool search(void* parametro){
		s_tlb* act = (s_tlb*) parametro;
		return act->pid == pid && act->pagina == pagina;
	}

	s_tlb* entry = list_find(tlb, search);

	if (entry != NULL){
		entry->marco = 0;
		entry->pagina = 0;
		entry->pid = 0;
		entry->memory_flag = 0;
		entry->modif_flag = 0;
		entry->ref_time = 0;

		log_info(LOG, "Se limpió información de página %d del proceso %d en la TLB", pagina, pid);
	}
}

void set_modif_all_pages(){

	void set_modif(void* parametro){
		s_page_table* page = (s_page_table*) parametro;
		log_info(LOG, "Se setea la página %d, presente en el marco %d, como modificada", page->pagina, page->marco);
		page->modif_flag = 1;
	}

	log_info(LOG, "Se bloquea tabla de páginas para flush de memoria");
	pthread_mutex_lock(&mutex_page_table);

	t_list* presentPages = get_present_pages();

	if(presentPages != NULL){
		list_iterate(presentPages, set_modif);
	}

	pthread_mutex_unlock(&mutex_page_table);
	log_info(LOG, "Se desbloquea tabla de páginas para flush de memoria");
}

t_list* get_present_pages(){

	t_list* ret_list = list_create();

	log_info(LOG, "Se obtienen las páginas de los procesos presentes en memoria");

	void search_pages(void* parametro){

		bool isPresent(void* parametro){
			s_page_table* page = (s_page_table*) parametro;
			return page->memory_flag == 1;
		}

		s_process* proceso = (s_process*) parametro;

		t_list* filter = list_filter(proceso->pages, isPresent);

		if (filter != NULL){
			list_add_all(ret_list, filter);
			log_info(LOG, "El proceso %d tiene %d páginas presentes en memoria", proceso->pid, list_size(filter));
		}else{
			log_info(LOG, "El proceso %d no tiene páginas presentes en memoria", proceso->pid);
		}
	}

	list_iterate(process_tpage, search_pages);

	return ret_list;
}

t_list* get_present_pages_from_pid(int pid){

	t_list* ret_list;

	bool isPresent(void* parametro){
		s_page_table* page = (s_page_table*) parametro;
		return page->memory_flag == 1;
	}

	bool search(void* parametro){
		s_process* proc = (s_process*) parametro;

		return proc->pid == pid;
	}

	s_process* proceso = list_find(process_tpage, search);

	if(proceso != NULL){
		ret_list = list_filter(proceso->pages, isPresent);
		return ret_list;
	}

	return NULL;
}

s_page_table* get_victim_using_clock(t_list* pages){
	int index = 0;
	int flag = 0;

	bool byTimeRef(void* parametro1, void* parametro2){
		s_page_table* page1 = (s_page_table*) parametro1;
		s_page_table* page2 = (s_page_table*) parametro2;

		return page1->ref_time <= page2->ref_time;
	}

	void getIndex(void*parametro){
		s_page_table* page = (s_page_table*) parametro;
		if (page->puntero != 1){
			index++;
		}
	}

	list_sort(pages, byTimeRef);
	list_iterate(pages, getIndex);

	while(flag == 0){
		s_page_table* page = list_get(pages, index);
		page->puntero = 1;

		if (page->bituso == 0){
			flag = 1;
			log_info(LOG, "Víctima-> Marco: %d; Página: %d", page->marco, page->pagina);
			return page;
		}else{
			page->bituso = 0;
			page->puntero = 0;
			index++;
		}

		if(index == list_size(pages)){
			//Desde el principio
			index = 0;
		}
	}

}

s_page_table* get_victim_using_clock_m(t_list* pages){
	int index = 0;
	int flag = 0;
	int found = 0;

	bool byTimeRef(void* parametro1, void* parametro2){
		s_page_table* page1 = (s_page_table*) parametro1;
		s_page_table* page2 = (s_page_table*) parametro2;

		return page1->ref_time <= page2->ref_time;
	}

	void getIndex(void*parametro){
		s_page_table* page = (s_page_table*) parametro;
		if (page->puntero != 1){
			index++;
		}
	}

	list_sort(pages, byTimeRef);
	list_iterate(pages, getIndex);

	int size = list_size(pages);
	int count = 0;

	while(found == 0){
		s_page_table* page = list_get(pages, index);
		page->puntero = 1;

		if (flag==0){ //Primera vuelta
			if(page->bituso == 0 && page->modif_flag == 0){
				log_info(LOG, "Víctima-> Marco: %d; Página: %d", page->marco, page->pagina);
				return page;
			}else{
				page->puntero = 0;
				index++;
			}
		}else{
			if(page->bituso == 0 && page->modif_flag == 1){
				log_info(LOG, "Víctima-> Marco: %d; Página: %d", page->marco, page->pagina);
				return page;
			}else{
				page->puntero = 0;
				page->bituso = 0;
				index++;
			}
		}
		//Para saber si terminé la vuelta.
		count++;
		if(count == size){
			if(flag==1) flag = 0; //Terminé la segunda vuelta, voy a la primera devuelta.
			else if(flag==0) flag = 1; //Terminé la primer vuelta, voy a la segunda.
			count = 0;
		}

		if(index == list_size(pages)){
			//Desde el principio
			index = 0;
		}
	}

}
int search_pid_present_in_frame(int frame){

	int pid = -1;

	bool search_pid(void* parametro){

		bool search_present_page(void* parametro){
			s_page_table* page = (s_page_table*) parametro;

			return page->marco == frame && page->memory_flag == 1;
		}

		s_process* proceso = (s_process*) parametro;

		if (list_find(proceso->pages, search_present_page) != NULL){
			return true;
		}else{
			return false;
		}
	}

	s_process* proceso = list_find(process_tpage, search_pid);

	if (proceso != NULL){
		pid = proceso->pid;
	}

	return pid;
}
