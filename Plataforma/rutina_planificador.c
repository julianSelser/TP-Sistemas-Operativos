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
#include <commons/string.h>

#include "rutina_orquestador.h"
#include "rutina_planificador.h"
#include "plataforma.h"

#include "serial.h"

//todo: faltan todos los logeos
void rutina_planificador(parametro *info)
{
	int i;
	char buf[100];
	//esto liberarlo, lo que llega por "recibir" es responsabilidad del usuario
	//(estas variables contendran de-serializaciones)
	t_turno_concluido *resultado;
	t_nodo_personaje *personaje;

	//desglose del "parametro info" en su log, colas y semaforos, macro en el .h
	DESGLOSE_INFO(info);

    sem_init(sem_cola_listos, 0, 1);
    sem_init(sem_cola_vacia, 0, 0);
    sem_init(sem_cola_bloqueados, 0, 1);

	//crea el hilo que escuchara personajes
	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL,(void*)rutina_escucha, info);

	while(1)
	{
		bool desconexion = false;

		sem_wait(sem_cola_vacia);
		sem_wait(sem_cola_listos);
		personaje = desencolar(listos, "listos", info->logger_planificador);
		sem_post(sem_cola_listos);

		for(i=0; i<(int)quantum ; i++)
		{
			if(( desconexion = enviar(personaje->socket, NOTIF_MOVIMIENTO_PERMITIDO, armarMSG_mov_permitido(), logger_planificador) < 0 )) break;
			if(( desconexion = recv(personaje->socket, buf, 100, MSG_PEEK)<=0 )) break;//nadie vio esta linea, la escribio roberto

			resultado = recibir(personaje->socket,NOTIF_TURNO_CONCLUIDO);

			usleep((int)(retraso*1000000));

			if((resultado->bloqueado)){
				sem_wait(sem_cola_bloqueados);
				encolar(buscar_lista_de_recurso(bloqueados,resultado->recurso_de_bloqueo) , personaje, string_from_format("%c de bloqueados",resultado->recurso_de_bloqueo), info->logger_planificador);
				sem_post(sem_cola_bloqueados);
				//sem_wait(sem_cola_vacia);
				break;
			}
		}
		log_info(info->logger_planificador, string_from_format("%s recibio Quantum", personaje->nombre), "INFO", info->logger_planificador);

		//si el personaje no quedo bloqueado y no se desconecto: reencolar; sino liberar el nodo
		if(!desconexion && !resultado->bloqueado)
		{
			sem_wait(sem_cola_listos);
			encolar(listos, personaje, "listos", info->logger_planificador);
			sem_post(sem_cola_listos);
			sem_post(sem_cola_vacia);
			free(resultado);
		}
		else if(desconexion){
			free(personaje->nombre);
			free(personaje);
		}
	}
}


void rutina_escucha(parametro *info)
{
	DESGLOSE_INFO(info);

	int socketNuevoPersonaje;
	int socketEscucha = init_socket_escucha(info->puerto, 1, info->logger_planificador);

	printf("\n escuchando por personajes en el planificador...\n\n");
	while (1)
	{
		if ((socketNuevoPersonaje = accept(socketEscucha, NULL, 0)) < 0)/* todo log:Error al aceptar conexion entrante */exit(1);
		else {
			t_nodo_personaje *personaje = armar_nodo_personaje( recibir(socketNuevoPersonaje, ENVIO_DE_DATOS_AL_PLANIFICADOR) ,socketNuevoPersonaje);
			sem_wait(sem_cola_listos);
			encolar(listos, personaje, "listos", info->logger_planificador);
			sem_post(sem_cola_listos);
			sem_post(sem_cola_vacia);
		}
	}
}


// funcion que toma el deserializado del mensaje crudo del personaje al conectarse al planificador
//y lo convierte en un nodo personaje para usar en las listas; tambien libera los datos del mensaje
t_nodo_personaje *armar_nodo_personaje(t_datos_delPersonaje_alPlanificador *datos, int socket)
{
	t_nodo_personaje *personaje = malloc(sizeof(t_nodo_personaje));

	personaje->char_personaje = datos->char_personaje;
	personaje->nombre = datos->nombre_personaje;
	personaje->socket = socket;

	free(datos);
	return personaje;
}


//wrappers por claridad
void encolar(t_list *cola, void *data, char *que_cola, t_log *logger){
	t_link_element *aux;
	char *lista = strdup(" ");

	list_add(cola, data);

	for(aux=cola->head; aux!=NULL ; aux=aux->next){
		string_append(&lista, "->");
		string_append(&lista,((t_nodo_personaje*)aux->data)->nombre);
	}

	if(logger!=NULL && strcmp(lista," ")){//ojo, el strcmp esta bien sin ==0
		log_info(logger, string_from_format("Se ha modificado la cola de %s al ser agregado un personaje", que_cola), "INFO");
		log_info(logger, lista, "INFO");
	}
	free(lista);
}

void *desencolar(t_list *cola, char *que_cola, t_log *logger){
	t_link_element *aux;
	char *lista = strdup(" ");

	for(aux=cola->head; aux!=NULL ; aux=aux->next){
		string_append(&lista, "->");
		string_append(&lista,((t_nodo_personaje*)aux->data)->nombre);
	}

	if(logger!=NULL && strcmp(lista," ")){//ojo, el strcmp esta bien sin ==0
		log_info(logger, string_from_format("Se ha modificado la cola de %s al ser removido un personaje", que_cola), "INFO");
		log_info(logger, lista, "INFO");
	}
	free(lista);

	return list_remove(cola, 0);
}


t_mov_permitido *armarMSG_mov_permitido(){
	t_mov_permitido *msg = malloc(sizeof(t_mov_permitido));
	msg->permitido=true;
	return msg;
}


