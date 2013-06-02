/*
 * mainNivel.c
 *
 *  Created on: 02/06/2013
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/queue.h>

//hilo principal del proceso nivel
//lee el archivo de configuración, crea el logger y lanza los demás hilos
//tambien inicializa los contadores de recursos

t_config * configuraciones;


int main(int argc, char ** argv)
{
	if(argc != 2) //controlar que haya exactamente un parámetro
	{
		puts("Uso: nivel <arch.conf>\n");
		return -1;
	}

	configuraciones = config_create(argv);



}
