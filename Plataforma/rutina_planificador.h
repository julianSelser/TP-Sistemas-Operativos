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




	void rutina_escucha(parametro *info);
	void rutina_planificador(parametro *info);

	bool personaje_esta_en_colas();
	void encolar(t_list *cola, void *data, char *que_cola, t_log *logger);
	void *desencolar(t_list *cola, char *que_cola, t_log *logger);

	t_nodo_personaje *armar_nodo_personaje(t_datos_delPersonaje_alPlanificador *datos, int socket);
	t_mov_permitido *armarMSG_mov_permitido();

	void cerrar_planificador();

	void personaje_destroyer(void *data);
	void bloqueados_destroyer(void *data);



#endif /* RUTINA_PLANIFICADOR_H_ */
