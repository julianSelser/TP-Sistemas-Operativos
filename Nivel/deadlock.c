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
int puede_terminar(char pje_actual, char * recs, char * instancias, int cant_recursos);
int liberar_recursos(char pje_actual, char * recs, char * instancias, int cant_recursos);
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
		char * pjes_en_deadlock; //por ejemplo "#$@"
		int cant_pjes_en_deadlock;
		//ACLARACIÓN
		//no asigno memoria para pjes_en_deadlock
		//voy a tomar el puntero que devuelva detectar_deadlock
		//detectar_deadlock ya asignó memoria para este string
		//es responsabilidad de esta funcion liberar esa memoria


		//crear_pjes(); crear personajes de prueba
		//crear_cajas(); crear cajas de prueba

		usleep(tiempo_chequeo_deadlock*1000);

		sem_wait(&sem_general); //semaforo para lockear la lista de personaes y la de recursos. si tomo este semaforo, el hilo principal no puede atender mensajes
		pjes_en_deadlock = detectar_deadlock(); //ver aclaración

	//	dentro de detectar_deadlock() esta el loguer para los involucrados en deadlock

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
			sem_post(&sem_general);


			puts(msj); puts("\n");
			log_info(logger, msj, "INFO");
			free(msj);

			if (recovery)
			{
			enviar(socket_orquestador, SOLICITUD_RECUPERO_DEADLOCK, msg_recupero_deadlock(pjes_en_deadlock), logger);
			sem_wait(&sem_recovery); //para que no vuelva a detectar deadlock hasta que se haya accionado el recovery
			}
		}
		else
		{
			sem_post(&sem_general);
		}

		free(pjes_en_deadlock);
	}
	return 0;
}



char * detectar_deadlock()
{

	char * pjes;
	char * finish;
	char ** pjes_nombre;
	char * involucrados;

	char * recs;
	char * instancias;

	int cant_pjes;
	int cant_recursos;

	int i;
	int cambio = 0;


	cant_pjes = list_size(lista_personajes);

	finish = malloc (cant_pjes); 
	pjes = malloc (cant_pjes);
    pjes_nombre = malloc(cant_pjes);

	for(i=0; i<cant_pjes; i++)
	{
		pjes[i] = ((t_nodo_personaje *)list_get(lista_personajes, i))->ID; //vector con los chars de todos los personajes presentes
		finish[i] = 0; //vector finish, ver silberschatz
		pjes_nombre[i]=((t_nodo_personaje *)list_get(lista_personajes, i))->nombre;//array con todos los nombres de los personajes
	} 

	cant_recursos = list_size(lista_cajas);

	recs = malloc(cant_recursos);
	instancias = malloc(cant_recursos);

	for(i=0; i<cant_recursos; i++)
	{
		recs[i] = ((t_caja *)list_get(lista_cajas, i))->ID; //vector con los chars de cada recurso
		instancias[i] = ((t_caja*)list_get(lista_cajas, i))->disp; //vector "disponible" del algoritmo
	}


	while(cambio)
	{
		char pje_actual;


		cambio = 0;
		for(i=0; i<cant_pjes; i++) //recorro el vector de personajes
		{
			pje_actual = pjes[i];
			if (finish[i]) continue; //salteo este personaje si finish esta en 1
			if (puede_terminar(pje_actual, recs, instancias, cant_recursos))
			{
				liberar_recursos(pje_actual, recs, instancias, cant_recursos); //disp=disp+asig(pje)
				finish[i]=1; //este pje puede terminar
				cambio=1; //marco que hubo un cambio (en el vector disp) para que haya una nueva pasada
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
			log_info(logger,string_from_format("este personaje esta en deadlock:%s",pjes_nombre[i]));

			string_append(&involucrados, pje_deadlock);
			free(pje_deadlock);
		}
		i++;
	} //una vez terminado el algoritmo, junto en un string todos los que quedaron en deadlock (finish=0)

	free(pjes);
	free(finish);
	free(recs);
	free(instancias);
	free(pjes_nombre);
	return involucrados;
}


void *msg_recupero_deadlock(char *pjes){
	t_solicitud_recupero_deadlock *solicitud = malloc(sizeof(t_solicitud_recupero_deadlock));

	solicitud->pjes_deadlock = strdup(pjes);

	return solicitud;
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

int puede_terminar(char pje_actual, char * recs, char * instancias, int cant_recursos)
{
	int cant_necesidades;
	t_list * necesidad;
	t_nodo_personaje * nodo_pje;
	int termina = 1;
	int i = 0;

	nodo_pje = ubicar_pje_por_ID(pje_actual);
	necesidad = nodo_pje->necesidades;
	cant_necesidades = list_size(necesidad);

	while (i<cant_necesidades) //me fijo, por cada necesidad, si la entrada correspondiente del vector disponible es mayor o igual
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
	} //es decir, con esta funcion comparo la fila del personaje con el vector disponible, retorno 1 si el vector disponible es mayor o igual, 0 si es menor
	return termina;
}

int liberar_recursos(char pje_actual, char * recs, char * instancias, int cant_recursos) //todo
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
	} //sumo los recursos asignados que tenia el personaje al vector disponible

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

int index_pje_por_ID(char ID)
{
	int cant_pjes;
	int i = 0;

	cant_pjes = list_size(lista_personajes);

	while(i<cant_pjes)
	{
		t_nodo_personaje * pje_actual;
		pje_actual = (t_nodo_personaje*)list_get(lista_personajes, i);
		if ((pje_actual->ID) == ID) return i;
		i++;
	}

	return -1;
}

