/*
 * rutina_orquestador.h
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */

#include "plataforma.h"
#include <commons/config.h>
#include <semaphore.h>
#include <serial.h>

#ifndef RUTINA_ORQUESTADOR_H_
#define RUTINA_ORQUESTADOR_H_

typedef struct
{
  int socket;
	t_list * colas[2];
	char * nombre;
	int puerto_planif;
	char  * IP_planif;
	int puerto;
	char * IP;
	sem_t * sem_listos;
	sem_t * sem_vacia;
	sem_t * sem_bloqueados;
} t_nodo_nivel;


void rutina_orquestador(/*?*/);

void rutina_inotify();
parametro *armar_parametro(t_list ** colas);
void lanzar_planificador(parametro * p);
void manejar_anuncio_nivel(int socket_nivel);
void manejar_sol_info(int socket_nivel);
t_info_nivel_planificador * crear_info_nivel(char * nombre);
void manejar_recs_liberados(int socket);
t_nodo_nivel * ubicar_nivel_por_socket(int socket);
t_nodo_bloq_por_recurso * ubicar_cola_por_rec(t_list * lista_colas, char ID_rec);
void manejar_sol_recovery(int socket);
char decidir(char * involucrados);
t_nodo_personaje * extraer(char ID, t_list * lista_colas);


#endif /* RUTINA_ORQUESTADOR_H_ */
