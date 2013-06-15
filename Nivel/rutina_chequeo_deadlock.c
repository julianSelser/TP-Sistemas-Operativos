/*
 * rutina_chequeo_deadlock.c
 *
 *  Created on: Jun 15, 2013
 *      Author: julian
 */

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <tad_items.h>
#include <nivel.h>
#include "Nivel.h"


void rutina_chequeo_deadlock(int *tiempo_deadlock);


void rutina_chequeo_deadlock(int *tiempo_deadlock){
	t_list *personajes = list_create();


	while(1){
		usleep(tiempo_deadlock*1000);

	}

}
