/*
 * nivel_defs.h
 *
 *  Created on: 08/06/2013
 *      Author: utnso
 */

#ifndef NIVEL_DEFS_H_
#define NIVEL_DEFS_H_

	#include <stdint.h>

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

	char * nombre;
	char * ip_puerto_orquestador;
	int tiempo_chequeo_deadlock;
	int recovery;
	uint8_t cantidad_de_recursos;

	t_log * logger;

	t_list * lista_cajas;

#endif /* NIVEL_DEFS_H_ */
