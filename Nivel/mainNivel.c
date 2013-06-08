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

#include "nivel_defs.h"

//hilo principal del proceso nivel
//lee el archivo de configuración, crea el logger y lanza los demás hilos
//tambien inicializa los contadores de recursos


char * nombre;
char * ip_puerto_orquestador;
int tiempo_chequeo_deadlock;
int recovery;
uint8_t cantidad_de_recursos;
t_log * logger;

t_list * lista_cajas; //mehea

int conf_es_valida(t_config * configuracion);

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
	log_name = nombre;
	string_append(&log_name, ".log");
	logger = log_create(log_name, "NIVEL", 0, LOG_LEVEL_TRACE);


	ip_puerto_orquestador = config_get_string_value(configuracion, "orquestador");
	tiempo_chequeo_deadlock = config_get_int_value(configuracion, "TiempoChequeoDeadlock");
	recovery = strcmp("On", config_get_string_value(configuracion, "Recovery"));

	cantidad_de_recursos = config_keys_amount(configuracion) - 4; //este 4 esta hardcodeado, pero la realidad es que siempre el archivo de config tiene la cantidad de cajas y cuatro entradas más

	lista_cajas = list_create();

	for(i=1; i==cantidad_de_recursos; i++)
	{
		char ** datos_caja;
		char * nombre_caja;
		char numero_caja[1];
		t_caja nodo_caja;

		//todo revisar condiciones de error de malloc?

		numero_caja[0] = i+0x30; //quick fix a falta de itoa

		nombre_caja = malloc(5); //porque la palabra Caja ocupa 4 bytes
		strcpy(nombre_caja, "Caja");
		string_append(&nombre_caja, numero_caja); //o sea, Caja1, Caja2, etc

		datos_caja = config_get_array_value(configuracion, nombre_caja);

		nodo_caja.nombre = malloc (strlen(datos_caja[0]));
		strcpy(nodo_caja.nombre, datos_caja[0]); //nombre es un puntero

		nodo_caja.ID = datos_caja[1][0];
		nodo_caja.disp = nodo_caja.total = atoi(datos_caja[2]); //todo verificar condicion de error de atoi?
		nodo_caja.x = atoi(datos_caja[3]); //todo idem
		nodo_caja.y = atoi(datos_caja[4]); //todo idem

		list_add(lista_cajas, &nodo_caja);
		free(datos_caja);
	}

	//en este punto, se termino de leer el archivo de config y se enlistaron todos los recursos
	config_destroy(configuracion);

	return 0; //para evitar el warning
}

int conf_es_valida(t_config * configuracion) //todo
{
	; //placeholder
	return 1;
}
