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

	//-----TYPEDEFS-----//

	typedef struct
	{
		uint8_t asig;
		uint8_t max;
		char ID_recurso;
	} t_necesidad;

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
		char ID;
		int socket;
		uint8_t x;
		uint8_t y;
		t_necesidad * necesidades;
	} t_personaje;


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


#endif /* NIVEL_DEFS_H_ */
