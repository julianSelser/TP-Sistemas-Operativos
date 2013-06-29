/*
 * rutina_orquestador.h
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */

#include "plataforma.h"
#include <commons/config.h>

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
} t_nodo_nivel;


void rutina_orquestador(/*?*/);

void rutina_inotify();
parametro *armar_parametro();
void lanzar_planificador();

#endif /* RUTINA_ORQUESTADOR_H_ */
