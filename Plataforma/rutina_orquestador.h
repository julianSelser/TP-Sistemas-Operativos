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

void rutina_orquestador(/*?*/);

void rutina_inotify();
parametro *armar_parametro();
void lanzar_planificador();

#endif /* RUTINA_ORQUESTADOR_H_ */
