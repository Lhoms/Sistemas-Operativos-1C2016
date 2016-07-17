/*
 * Primitivas.h
 *
 *  Created on: 27/4/2016
 *      Author: utnso
 */

#ifndef SRC_PRIMITIVAS_H_
#define SRC_PRIMITIVAS_H_
	#include <parser/parser.h>

t_puntero AnSISOP_definirVariable(t_nombre_variable identificador_variable);
t_puntero AnSISOP_obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable AnSISOP_dereferenciar(t_puntero direccion_variable);
void AnSISOP_asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable AnSISOP_obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable AnSISOP_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void AnSISOP_irAlLabel(t_nombre_etiqueta t_nombre_etiqueta);
void AnSISOP_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void AnSISOP_finalizar(void);
void AnSISOP_retornar(t_valor_variable retorno);
void AnSISOP_imprimir(t_valor_variable valor_mostrar);
void AnSISOP_imprimirTexto(char* texto);
void AnSISOP_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void AnSISOP_wait(t_nombre_semaforo identificador_semaforo);
void AnSISOP_signal(t_nombre_semaforo identificador_semaforo);

#endif /* SRC_PRIMITIVAS_H_ */
