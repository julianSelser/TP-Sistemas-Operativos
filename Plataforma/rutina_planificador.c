/*
 * rutina_planificador.c
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */

#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

#include "rutina_orquestador.h"
#include "rutina_planificador.h"
#include "plataforma.h"

#include "serial.h"



void rutina_planificador(parametro *info)
{
	sem_t *cola_listos = &info->semaforos[1];
	sem_t *cola_vacia = &info->semaforos[2];

    sem_init(cola_listos, 0, 1);
    sem_init(cola_vacia, 0, 0);

	//desgloce del "parametro info" para claridad
	t_log *logger_planificador = info->logger_planificador;
	t_list *listos = info->colas[LISTOS];
	t_list *bloqueados = info->colas[BLOQUEADOS];

	//crea el hilo que escuchara personajes
	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL,(void*)rutina_escucha, info);

	//esto liberarlo, lo que llega por "recibir" es responsabilidad del usuario
	//(estas variables contendran de-serializaciones)
	t_turno_concluido *resultado;
	t_nodo_personaje *personaje;
	bool saltear_reencolamiento;
	bool primera=true;
	int i;

	while(1){
		saltear_reencolamiento = false;

		sem_wait(cola_vacia);
		sem_wait(cola_listos);

		if(primera){printf("\n iniciando bucle principal del planificador\n\n");primera=false;}

		personaje = desencolar(listos);
		sem_post(cola_listos);
		sem_post(cola_vacia);


		for(i=0; i<quantum ; i++)
		{
			printf("\n planificador va a enviar mensaje\n\n");
			enviar(personaje->socket, NOTIF_MOVIMIENTO_PERMITIDO, armarMSG_mov_permitido(), logger_planificador);
			printf("\n planificador envio mensaje\n\n");
			resultado = recibir(personaje->socket,NOTIF_TURNO_CONCLUIDO);
			printf("\n planificador recibio notif_turno-concluido\n\n");

			//la "operacion" de dar quantum termino aca, si se hiciera despues no esperaria lo que deberia luego de personajes que terminaron o se bloquean
			sleep(retraso); //por ejemplo, 5 personajes terminan seguidos, si no sleepea aca lo hace sin retraso, cuando deberia haber un retraso por operacion

			if((saltear_reencolamiento =  resultado->bloqueado || resultado->termino_nivel )){
				if(resultado->bloqueado) encolar( buscar_lista_de_recurso(bloqueados,resultado->recurso_de_bloqueo) , personaje);
				sem_wait(cola_vacia);
				break;
			}
		}

		sem_wait(cola_listos);
		if(!saltear_reencolamiento) encolar(listos, personaje); //cuando se des-encola un personaje sale de la cola, ante bloqueo o termino de nivel el reencolamiento debe saltearse
		sem_post(cola_listos);
	}

	free(info);
}


void rutina_escucha(parametro *info)
{
	sem_t *cola_listos = &info->semaforos[1];
	sem_t *cola_vacia = &info->semaforos[2];

	int socketNuevoPersonaje;
	int socketEscucha = init_socket_escucha(info->puerto, 1, info->logger_planificador);

	printf("\n escuchando por personajes en el planificador...\n\n");
	while (1)
	{
		if ((socketNuevoPersonaje = accept(socketEscucha, NULL, 0)) < 0)/* todo log:Error al aceptar conexion entrante */exit(1);
		else {
			t_nodo_personaje *personaje = armar_personaje( recibir(socketNuevoPersonaje, ENVIO_DE_DATOS_AL_PLANIFICADOR) ,socketNuevoPersonaje);
			sem_wait(cola_listos);
			encolar(info->colas[LISTOS], personaje);
			sem_post(cola_listos);
			sem_post(cola_vacia);
		}
	}
}


// funcion que toma el deserializado del mensaje crudo del personaje al conectarse al planificador
//y lo convierte en un nodo personaje para usar en las listas; tambien libera los datos del mensaje
t_nodo_personaje *armar_personaje(t_datos_delPersonaje_alPlanificador *datos, int socket)
{
	t_nodo_personaje *personaje = malloc(sizeof(t_nodo_personaje));

	personaje->char_personaje = datos->char_personaje;
	personaje->socket = socket;

	free(datos);
	return personaje;
}


void encolar(t_list *cola, void *data){
	list_add(cola, data);
}

void *desencolar(t_list *cola){
	return list_remove(cola, 0);
}


t_mov_permitido *armarMSG_mov_permitido(){
	t_mov_permitido *msg = malloc(sizeof(t_mov_permitido));
	msg->permitido=true;
	return msg;
}


t_list *buscar_lista_de_recurso(t_list *bloqueados, char recurso_de_bloqueo)
{
		t_link_element *element = bloqueados->head;
		t_nodo_bloq_por_recurso *nodo = element->data; //nunca va a ser NULL, el orquestador siempre crea esta lista

		while ( element != NULL && !(nodo->char_recurso == recurso_de_bloqueo)) {
			element = element->next;
			nodo = element->data;
		}
		return nodo->personajes;
}


//todo:semaforos en la cola de bloqueados
