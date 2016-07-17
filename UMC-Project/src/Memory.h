/*
 * Memory.h
 *
 *  Created on: 23/5/2016
 *      Author: utnso
 */
/*
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
	void  save_in_memory(int socketSwap, int pid, int pagina, int frame, char* bytes, e_operacion2 oper);
	void  create_mp();
	void  destroy_mp();
	void  create_tlb();
	void  destroy_tlb();
	void  clear_tlb_from_pid(int pid);
	s_tlb* find_process_in_tlb(int pid);
	s_tlb* find_process_page_in_tlb(int pid, int page);
	s_page_table* get_page_from_process(s_process* process, int page);
	void  init_tlb();
	void  add_page_to_tlb(int socketSwap, s_page_table* page, int pid, e_operacion2 oper);
	void  create_page_table();
	void  destroy_page_table();
	void  set_page_as_present(int socketSwap, int pid, int frame, int pagina, s_page_table* page, e_operacion2 oper);
	void  destroy_page_table_from_pid(int pid);
	s_process* find_page_table_from_pid(int pid);
	void  create_frames();
	void  destroy_frames();
	void  init_frames();
	int   get_first_free_frame();
	void  clear_all_memory_from_pid(int pid);
	t_list* get_present_pages();
	t_list* get_present_pages_from_pid(int pid);
	s_page_table*  get_victim_using_clock(t_list* pages);
	s_page_table*  get_victim_using_clock_m(t_list* pages);
	void  set_modif_all_pages();
	int   search_pid_present_in_frame(int frame);
	void clear_pointer_to_pages(t_list* pages);
	void clear_page_in_tlb(int pid, int pagina);

#endif /* SRC_MEMORY_H_ */
*/
