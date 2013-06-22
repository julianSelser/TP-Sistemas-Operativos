/*
 * nivel_defs.h
 *
 *  Created on: 08/06/2013
 *      Author: utnso
 */

#ifndef NIVEL_DEFS_H_
#define NIVEL_DEFS_H_

	#include <stdint.h>

	#include <nivel.h> //nivel gui de la catedra
	#include <commons/log.h>
	#include <commons/collections/list.h>
	#include <commons/config.h>

	//-----TYPEDEFS-----//

	typedef struct
	{
		char * nombre;
		char ID;
		uint8_t x;
		uint8_t y;
		uint8_t total;
		uint8_t disp;
	} t_caja;

	typedef struct
	{
		uint8_t asig;
		uint8_t max;
		char ID_recurso;
	} t_necesidad;

	typedef struct
	{
		char ID;
		char *nombre;
		int socket;
		uint8_t x;
		uint8_t y;
		t_list * necesidades;
	} t_nodo_personaje;


	//-----VARIABLES GLOBALES----//

	//configuracion:
	extern char * nombre;
	extern char * ip_orquestador;
	extern int puerto_orquestador;
	extern int tiempo_chequeo_deadlock;
	extern int recovery;
	extern uint8_t cantidad_de_recursos;


	extern t_log * logger;

	extern t_list * lista_cajas;
	extern t_list * lista_personajes;
	extern ITEM_NIVEL * lista_items;


	/*	DECLARACION DE FUNCIONES	*/

	//rutina para el hilo de deteccion
	int rutina_chequeo_deadlock();

	void reubicar_recursos(t_list *necesidades);
	void manejar_peticion(int socket);
	void levantar_config(int argc, char ** argv);
	int conf_es_valida(t_config * configuracion);
	int imprimir_nodo_caja(t_caja * nodo_caja);
	int crear_item_caja_desde_nodo(t_caja * nodo_caja);

	/*	MANEJO DE MENSAJES	*/

	void manejar_ingreso_personaje(int socket);
	void manejar_solicitud_movimiento(int socket);
	void manejar_solicitud_ubicacion_recurso(int socket);
	void manejar_solicitud_instancia_recurso(int socket);
	void manejar_notif_eleccion_victima(int socket);
	void manejar_recursos_reasignados(int socket);


#endif /* NIVEL_DEFS_H_ */
