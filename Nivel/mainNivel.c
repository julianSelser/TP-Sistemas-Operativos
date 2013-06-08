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
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <tad_items.h>
#include <nivel.h> //nivel gui de la catedra

#include "nivel_defs.h"

//hilo principal del proceso nivel
//lee el archivo de configuración, crea el logger y lanza los demás hilos
//tambien inicializa los contadores de recursos

int conf_es_valida(t_config * configuracion);
int imprimir_nodo_caja(t_caja * nodo_caja);

int main(int argc, char ** argv)
{
	t_config * configuracion;
	char * log_name;

	int i;

	if(argc != 2) //controlar que haya exactamente un parámetro
	{
		puts("Uso: nivel <arch.conf>\n");
		return -1;
	}

	configuracion = config_create(argv[1]);

	if (!conf_es_valida(configuracion)) //ver que el archivo de config tenga todito
	{
		puts("Archivo de configuración incompleto o inválido.\n");
		return -2;
	}

	nombre = config_get_string_value(configuracion, "Nombre");

	//ahora que ya se como se llama el nivel, puedo crear el log
	log_name = malloc(strlen(nombre)+1);
	strcpy(log_name, nombre);

	string_append(&log_name, ".log");
	logger = log_create(log_name, "NIVEL", 0, LOG_LEVEL_TRACE);


	ip_puerto_orquestador = config_get_string_value(configuracion, "orquestador");
	tiempo_chequeo_deadlock = config_get_int_value(configuracion, "TiempoChequeoDeadlock");
	recovery = strcmp("On", config_get_string_value(configuracion, "Recovery"));

	cantidad_de_recursos = config_keys_amount(configuracion) - 4; //este 4 esta hardcodeado, pero la realidad es que siempre el archivo de config tiene la cantidad de cajas y cuatro entradas más

	lista_cajas = list_create();

	for(i=1; i<=cantidad_de_recursos; i++)
	{
		char ** datos_caja;
		char * nombre_caja;
		char numero_caja[2];
		t_caja * nodo_caja;

		//todo revisar condiciones de error de malloc?

		numero_caja[0] = i+0x30; //quick fix a falta de itoa
		numero_caja[1] = '\0';

		nombre_caja = malloc(5); //porque la palabra Caja ocupa 4 bytes
		strcpy(nombre_caja, "Caja");
		string_append(&nombre_caja, numero_caja); //o sea, Caja1, Caja2, etc

		datos_caja = config_get_array_value(configuracion, nombre_caja);

		nodo_caja = malloc(sizeof(t_caja));

		nodo_caja->nombre = malloc (strlen(datos_caja[0]));
		strcpy(nodo_caja->nombre, datos_caja[0]); //nombre es un puntero

		nodo_caja->ID = datos_caja[1][0];
		nodo_caja->disp = nodo_caja->total = atoi(datos_caja[2]); //todo verificar condicion de error de atoi?
		nodo_caja->x = atoi(datos_caja[3]); //todo idem
		nodo_caja->y = atoi(datos_caja[4]); //todo idem

		list_add(lista_cajas, (void *) nodo_caja);
		free(datos_caja);
	}

	//en este punto, se termino de leer el archivo de config y se enlistaron todos los recursos
	config_destroy(configuracion);

	//for (i=0; i< cantidad_de_recursos; i++)	imprimir_nodo((t_caja *)list_get(lista_cajas, i));
	//linea para controlar que se haya enlistado todito bien


	return 0; //para evitar el warning
}

int conf_es_valida(t_config * configuracion)
{
	return(config_has_property(configuracion, "Nombre") &&
			config_has_property(configuracion, "TiempoChequeoDeadlock") &&
			config_has_property(configuracion, "Recovery") &&
			config_has_property(configuracion, "Caja1") && //al menos una caja de recursos
			config_has_property(configuracion, "orquestador")
			);
}

int imprimir_nodo_caja(t_caja * nodo_caja)
{
	printf("Esta caja contiene %d %s de %d, su símbolo es %c y se encuentra en la posición (%d,%d)\n", nodo_caja->disp, nodo_caja->nombre, nodo_caja->disp, nodo_caja->ID, nodo_caja->x, nodo_caja->y);
	return 0;
}




//--------------FIN MAIN--------------//

}
