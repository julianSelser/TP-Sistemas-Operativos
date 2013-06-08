/*
 * nivel_defs.h
 *
 *  Created on: 08/06/2013
 *      Author: utnso
 */

#ifndef NIVEL_DEFS_H_
#define NIVEL_DEFS_H_

	#include <stdint.h>

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


#endif /* NIVEL_DEFS_H_ */
