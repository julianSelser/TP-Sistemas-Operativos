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
#include <fcntl.h>

#ifndef RUTINA_ORQUESTADOR_H_
#define RUTINA_ORQUESTADOR_H_

typedef struct
{
  int socket;
	t_list * colas[2];
	char * nombre;
	int puerto_planif;
	int puerto;
	char * IP;
	pthread_t hilo_planificador;
	sem_t * sem_listos;
	sem_t * sem_vacia;
	sem_t * sem_bloqueados;
} t_nodo_nivel;


void rutina_orquestador(/*?*/);

void rutina_inotify(int inotify_fd);
parametro *armar_parametro(t_nodo_nivel * nivel, t_log * logger);
t_nodo_nivel * ubicar_nivel_por_socket(int socket, char *index);
t_info_nivel_planificador * crear_info_nivel(char * nombre);
t_nodo_personaje * extraer(char ID, t_list * lista_colas, int intentos);
t_nodo_bloq_por_recurso * ubicar_cola_por_rec(t_list * lista_colas, char ID_rec);
char decidir(char * involucrados);
int agregar_sin_repetidos(char **string, char c);

//manejador de peticiones
void manejar_peticion(int socket);

//manejadores de mensajes
void manejar_sol_recovery(int socket);
void manejar_plan_terminado(int socket);
void manejar_sol_info(int socket_nivel);
void manejar_recs_liberados(int socket);
void manejar_anuncio_nivel(int socket_nivel);

//no miren esto, es feo, hagan como que no esta
#define FD_SETEO 	fcntl(inotify_fd, F_SETFL, FD_CLOEXEC);	\
					FD_ZERO(&maestro);						\
					FD_ZERO(&read_fds);						\
					FD_SET(socketEscucha, &maestro);		\
					FD_SET(inotify_fd, &maestro);			\


#endif /* RUTINA_ORQUESTADOR_H_ */
