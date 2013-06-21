/*
 * deadlock.c
 *
 *  Created on: 16/06/2013
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>

#include <serial.h>
#include "Nivel.h"

int indexof(char * array, char c, int size);
int puede_terminar(char pje_actual, char * recs, uint8_t * instancias, int cant_recursos);
int liberar_recursos(char pje_actual, char * recs, uint8_t * instancias, int cant_recursos);
t_nodo_personaje * ubicar_pje_por_ID(char ID);
char * detectar_deadlock();


//t_list * lista_personajes;
//t_list * lista_cajas;
//estas dos listas se declaran en el .h y se definen en el otro hilo, estaban aca para hacer pruebas

//INICIO FUNCIONES DE PRUEBA

/*int crear_cajas()
{
	int i = 0x41;

	lista_cajas = list_create();

	while(i<0x48)
	{
		t_caja * caja;

		caja = malloc(sizeof(t_caja));

		caja->nombre=NULL;
		caja->disp = i-0x40;
		caja->total = i-0x3A;
		caja->x=i-60;
		caja->y=i-60;
		caja->ID=i;

		list_add(lista_cajas, caja);

		i++;

	}


	return 0;
}

int crear_pjes()
{
	int i = 0x3A;

	lista_personajes=list_create();

	while(i<0x41)
	{
		t_nodo_personaje * nuevo_pje;
		t_list * lista_necesidades;
		int j = 0;

		nuevo_pje = malloc(sizeof(t_nodo_personaje));

		lista_necesidades = list_create();

		while(j<3)
		{
			t_necesidad * nodo_necesidad;

			nodo_necesidad = malloc(sizeof(nodo_necesidad));

			nodo_necesidad->ID_recurso=0x41+((j*i)%7);
			nodo_necesidad->asig=j;
			nodo_necesidad->max=(i%6)+j;

			list_add(lista_necesidades, (void *)nodo_necesidad);

			j++;
		}

		nuevo_pje->ID = i;
		nuevo_pje->x = i-10;
		nuevo_pje->y = i+10;
		nuevo_pje->socket = 0;
		nuevo_pje->necesidades = lista_necesidades;
		list_add(lista_personajes,(void *) nuevo_pje);
		i++;
	}

	return 0;
}*/

//FIN FUNCIONES DE PRUEBA

int rutina_chequeo_deadlock()
{
	while(1)
	{
		char * pjes_en_deadlock;
		int cant_pjes_en_deadlock;
		//ACLARACIÓN
		//no asigno memoria para pjes_en_deadlock
		//voy a tomar el puntero que devuelva detectar_deadlock
		//detectar_deadlock ya asignó memoria para este string
		//es responsabilidad de esta funcion liberar esa memoria


		//crear_pjes(); crear personajes de prueba
		//crear_cajas(); crear cajas de prueba

		usleep(tiempo_chequeo_deadlock*1000);

		//TOMAR MUTEX GENERAL

		pjes_en_deadlock = detectar_deadlock(); //ver aclaración

		//SOLTAR MUTEX GENERAL
		cant_pjes_en_deadlock = strlen(pjes_en_deadlock);

		if (cant_pjes_en_deadlock) //es decir, si hay al menos dos (imposible que haya uno)
		{
			char * msj;
			t_nodo_personaje * personaje;
			int i;

			msj = malloc(1);
			msj[0] = '\0';

			string_append(&msj, "Deadlock! Se encuentran involucrados ");

			i = 0;
			while(i < cant_pjes_en_deadlock)
			{
				personaje = ubicar_pje_por_ID(pjes_en_deadlock[i]);

				string_append(&msj, personaje->nombre);

				if((i+2)==cant_pjes_en_deadlock)
				{
					string_append(&msj, " y ");
				}
				else if((i+1)== cant_pjes_en_deadlock)
				{
					string_append(&msj, ".");
				}
				else
				{
					string_append(&msj, ", ");
				}

				i++;
			}

			puts(msj); puts("\n");
			log_info(logger, msj, "INFO");
			free(msj);

			if (recovery)
			{
			//enviar(socket_orquestador, SOLICITUD_RECUPERO_DEADLOCK, pjes_en_deadlock, logger);
			//WAIT(MUTEX RECOVERY)
			}
		}

		free(pjes_en_deadlock);
	}
	return 0;
}



char * detectar_deadlock()
{

	char * pjes;
	uint8_t * finish;
	char * involucrados;

	char * recs;
	uint8_t * instancias;

	int cant_pjes;
	int cant_recursos;

	int i;
	int cambio;


	cant_pjes = list_size(lista_personajes);

	finish = malloc (cant_pjes);
	pjes = malloc (cant_pjes);

	for(i=0; i<cant_pjes; i++)
	{
		pjes[i] = ((t_nodo_personaje *)list_get(lista_personajes, i))->ID;
		finish[i] = 0;
	}

	cant_recursos = list_size(lista_cajas);

	recs = malloc(cant_recursos);
	instancias = malloc(cant_recursos);

	for(i=0; i<cant_recursos; i++)
	{
		recs[i] = ((t_caja *)list_get(lista_cajas, i))->ID;
		instancias[i] = ((t_caja*)list_get(lista_cajas, i))->disp;
	}



	while(cambio)
	{
		char pje_actual;

		cambio = 0;
		for(i=0; i<cant_pjes; i++)
		{
			pje_actual = pjes[i];
			if (finish[i]) continue;
			if (puede_terminar(pje_actual, recs, instancias, cant_recursos))
			{
				liberar_recursos(pje_actual, recs, instancias, cant_recursos);
				finish[i]=1;
				cambio=1;
			}
		}
	}

	i= 0;

	involucrados = malloc(1);
	involucrados[0] = '\0';

	while(i<cant_pjes)
	{
		if(finish[i]==0)
		{
			char * pje_deadlock;
			pje_deadlock = malloc(2);
			pje_deadlock[0] = pjes[i];
			pje_deadlock[1] = '\0';

			string_append(&involucrados, pje_deadlock);
			free(pje_deadlock);
		}
		i++;
	}

	free(pjes);
	free(finish);
	free(recs);
	free(instancias);

	return involucrados;
}


int indexof(char * array, char c, int size)
{
	int i = 0;

	while (i<size)
	{
		if (array[i] == c) return i;
		i++;
	}

	return -1;
}

//ojo, las dos funciones que siguen son muy parecidas
//todo compartir codigo?

int puede_terminar(char pje_actual, char * recs, uint8_t * instancias, int cant_recursos) //todo
{
	int cant_necesidades;
	t_list * necesidad;
	t_nodo_personaje * nodo_pje;
	int termina = 1;
	int i = 0;

	nodo_pje = ubicar_pje_por_ID(pje_actual);
	necesidad = nodo_pje->necesidades;
	cant_necesidades = list_size(necesidad);

	while (i<cant_necesidades)
	{
		int posrec;
		t_necesidad * nec_actual;

		nec_actual = (t_necesidad *)list_get(necesidad, i);
		posrec = indexof(recs, nec_actual->ID_recurso, cant_recursos);

		if((instancias[posrec])<((nec_actual->max)-(nec_actual->asig)))
		{
			termina = 0;
			break;
		}

		i++;
	}
	return termina;
}

int liberar_recursos(char pje_actual, char * recs, uint8_t * instancias, int cant_recursos) //todo
{
	int cant_necesidades;
	t_list * necesidad;
	t_nodo_personaje * nodo_pje;
	int i = 0;

	nodo_pje = ubicar_pje_por_ID(pje_actual);
	necesidad = nodo_pje->necesidades;
	cant_necesidades = list_size(necesidad);

	while (i<cant_necesidades)
	{
		int posrec;
		t_necesidad * nec_actual;

		nec_actual = (t_necesidad *)list_get(necesidad, i);
		posrec = indexof(recs, nec_actual->ID_recurso, cant_recursos);

		instancias[posrec] = instancias[posrec] + nec_actual->asig;
		i++;
	}

	return 0;
}

t_nodo_personaje * ubicar_pje_por_ID(char ID)
{
	int cant_pjes;
	int i = 0;

	cant_pjes = list_size(lista_personajes);

	while(i<cant_pjes)
	{
		t_nodo_personaje * pje_actual;
		pje_actual = (t_nodo_personaje*)list_get(lista_personajes, i);
		if ((pje_actual->ID) == ID) return pje_actual;
		i++;
	}

	return NULL;
}
