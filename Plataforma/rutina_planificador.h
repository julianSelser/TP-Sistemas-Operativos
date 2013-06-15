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

	//estructura de cada nodo de la lista de personajes
	//contiene el socket de c/proceso personaje y su caracter identificador
	typedef struct{
		int socket;
		char char_personaje;
	}t_nodo_personaje;

	//estructura de cada nodo de la lista de bloqueados por recuros
	//tiene un caracter identificador del recurso y una lista de personajes
	typedef struct{
		char char_recurso;
		t_list * personajes;
	}t_nodo_bloq_por_recurso;

	void rutina_escucha(parametro *info);
	void rutina_planificador(parametro *info);

	t_list *buscar_lista_de_recurso(t_list *bloqueados, char recurso_de_bloqueo);
	void encolar(t_list *cola, void *data);
	void *desencolar(t_list *cola);

	void sigpipe_handler();

	t_nodo_personaje *armar_personaje(t_datos_delPersonaje_alPlanificador *datos, int socket);

	t_mov_permitido *armarMSG_mov_permitido();

#endif /* RUTINA_PLANIFICADOR_H_ */
