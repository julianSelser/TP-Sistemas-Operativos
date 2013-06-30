/*
 * rutina_planificador.h
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */
#include "serial.h"
#include "plataforma.h" //por que esta el typedef de parametro

#ifndef RUTINA_PLANIFICADOR_H_
#define RUTINA_PLANIFICADOR_H_



	/*	FUNCIONES	*/

	void rutina_escucha(parametro *info);
	void rutina_planificador(parametro *info);

	t_list *buscar_lista_de_recurso(t_list *bloqueados, char recurso_de_bloqueo);
	void encolar(t_list *cola, void *data);
	void *desencolar(t_list *cola);

	void sigpipe_handler();
	t_nodo_personaje *armar_nodo_personaje(t_datos_delPersonaje_alPlanificador *datos, int socket);
	t_mov_permitido *armarMSG_mov_permitido();


	/*	MACROS	*/

	//macro que separa los datos del parametro, creando variables de uso comun para facilitar el uso
	#define DESGLOSE_INFO(x)	t_log *logger_planificador = x->logger_planificador;	\
								t_list *listos = x->colas[LISTOS];						\
								t_list *bloqueados = x->colas[BLOQUEADOS];				\
																						\
								sem_t *sem_cola_listos = &x->semaforos[0];				\
								sem_t *sem_cola_vacia = &x->semaforos[1];				\
								sem_t *sem_cola_bloqueados = &x->semaforos[2]			\


#endif /* RUTINA_PLANIFICADOR_H_ */
