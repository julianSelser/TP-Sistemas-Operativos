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

__thread t_nodo_personaje *personaje=NULL;
__thread parametro *est;
__thread pthread_t hilo_escucha;

void rutina_planificador(parametro *info)
{
	int i;
	t_turno_concluido *resultado;

	//desgloce del parametro
	t_log *logger_planificador = info->logger_planificador;
	t_list *bloqueados = info->colas[BLOQUEADOS];
	t_list *listos = info->colas[LISTOS];
	sem_t *sem_cola_listos = &info->semaforos[0];
	sem_t *sem_cola_vacia = &info->semaforos[1];
	sem_t *sem_cola_bloqueados = &info->semaforos[2];

    sem_init(sem_cola_listos, 0, 1);
    sem_init(sem_cola_vacia, 0, 0);
    sem_init(sem_cola_bloqueados, 0, 1);

	est = info;
	signal(SIGTERM, cerrar_planificador);//recien a partir de este punto me interesa bloquearla

	pthread_create(&hilo_escucha, NULL,(void*)rutina_escucha, info);


	while(1)
	{
		bool desconexion = false;

		sem_wait(sem_cola_vacia);
		sem_wait(sem_cola_listos);
		personaje = desencolar(listos, "Listos", logger_planificador);
		sem_post(sem_cola_listos);

		for(i=0; i<(int)quantum ; i++)
		{
			if(( desconexion = enviar(personaje->socket, NOTIF_MOVIMIENTO_PERMITIDO, armarMSG_mov_permitido(), logger_planificador) < 0 )) break;
			if(( desconexion = !is_connected(personaje->socket) )) break;

			resultado = recibir(personaje->socket,NOTIF_TURNO_CONCLUIDO);

			usleep((int)(retraso*1000000));

			if((resultado->bloqueado)){
				sem_wait(sem_cola_bloqueados);
				encolar(buscar_lista_de_recurso(bloqueados,resultado->recurso_de_bloqueo), personaje, string_from_format("%s quedó esperando %c, bloqueados por %c", personaje->nombre, resultado->recurso_de_bloqueo, resultado->recurso_de_bloqueo), logger_planificador);
				sem_post(sem_cola_bloqueados);
				break;
			}
		}

		//si el personaje no quedo bloqueado y no se desconecto: reencolar; sino liberar el nodo
		if(!desconexion && !resultado->bloqueado)
		{
			log_info(logger_planificador, string_from_format("%s recibio Quantum!", personaje->nombre), "INFO", logger_planificador);

			sem_wait(sem_cola_listos);
			encolar(listos, personaje, "Listos", logger_planificador);
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
	//desgloce del parametro
	t_log *logger_planificador = info->logger_planificador;
	t_list *listos = info->colas[LISTOS];
	sem_t *sem_cola_listos = &info->semaforos[0];
	sem_t *sem_cola_vacia = &info->semaforos[1];

	int socketNuevoPersonaje;
	int socketEscucha = init_socket_escucha(info->puerto, 1, logger_planificador);

	while (1)
	{
		if ((socketNuevoPersonaje = accept(socketEscucha, NULL, 0)) < 0) {
			log_error(logger_planificador, "Error aceptando un personaje, saliendo...", "ERROR");
			exit(1);
		}
		else {
			t_nodo_personaje *pje = armar_nodo_personaje( recibir(socketNuevoPersonaje, ENVIO_DE_DATOS_AL_PLANIFICADOR) ,socketNuevoPersonaje);
			sem_wait(sem_cola_listos);
			encolar(listos, pje, "Listos", logger_planificador);
			sem_post(sem_cola_listos);
			sem_post(sem_cola_vacia);
		}
	}
}


//funcion que toma el deserializado del mensaje crudo del personaje al conectarse al planificador
//y lo convierte en un nodo personaje para usar en las listas; tambien libera los datos del mensaje
t_nodo_personaje *armar_nodo_personaje(t_datos_delPersonaje_alPlanificador *datos, int socket)
{
	t_nodo_personaje *pje = malloc(sizeof(t_nodo_personaje));

	pje->char_personaje = datos->char_personaje;
	pje->nombre = datos->nombre_personaje;
	pje->socket = socket;

	free(datos);
	return pje;
}


//encolar agrega "data" al final de la "cola" y logea la cola con el nombre especificado (que_cola) con los datos de la "cola"
void encolar(t_list *cola, void *data, char *que_cola, t_log *logger){

	list_add(cola, data);

	if(logger!=NULL)
	{
		t_link_element *aux;
		char *lista = strdup("");

		for(aux=cola->head; aux!=NULL ; aux=aux->next){
			string_append(&lista,((t_nodo_personaje*)aux->data)->nombre);
			if(aux->next!=NULL)	string_append(&lista, "->");
		}
		if(strcmp(lista,""))//ojo, el strcmp esta bien sin ==0
		{
			log_info(logger, string_from_format("%s: %s", que_cola, lista), "INFO");
		}
		free(lista);
	}
}

//encolar remueve y devuelve los datos al principio de la "cola" y logea la cola con el nombre especificado (que_cola) con los datos de la "cola"
//notar que no se van a logear colas vacias, ya hay suficiente output en pantalla...
void *desencolar(t_list *cola, char *que_cola, t_log *logger){

	void *element = list_remove(cola, 0);

	if(logger!=NULL){
		t_link_element *aux;
		char *lista = strdup("");

		for(aux=cola->head; aux!=NULL ; aux=aux->next){
			string_append(&lista,((t_nodo_personaje*)aux->data)->nombre);
			if(aux->next!=NULL)	string_append(&lista, "->");
		}
		if(strcmp(lista,"") && element!=NULL){//ojo, el strcmp esta bien sin ==0
			log_info(logger, string_from_format("Desencolando a %s! %s: %s",  ((t_nodo_personaje*)element)->nombre, que_cola, lista), "INFO");
		}
		free(lista);
	}
	return element;
}


void cerrar_planificador(){//la unica manera sana que encontre de que el personaje que esta desencolado se libere bien cuando cae un nivel

	if(personaje!=NULL && !personaje_esta_en_colas()) encolar(est->colas[LISTOS], personaje, NULL, NULL);

	list_destroy_and_destroy_elements(est->colas[LISTOS], personaje_destroyer);
	list_destroy_and_destroy_elements(est->colas[BLOQUEADOS], bloqueados_destroyer);

	log_destroy(est->logger_planificador);

	sem_destroy(&est->semaforos[0]);
	sem_destroy(&est->semaforos[1]);
	sem_destroy(&est->semaforos[2]);

	free(est);

	pthread_cancel(hilo_escucha);
	pthread_exit(NULL);
}


t_mov_permitido *armarMSG_mov_permitido(){
	t_mov_permitido *msg = malloc(sizeof(t_mov_permitido));
	msg->permitido=true;
	return msg;
}


void personaje_destroyer(void *data){
	t_nodo_personaje *p = data;
	close(p->socket);
	free(p->nombre);
	free(p);
}


void bloqueados_destroyer(void *data){
	t_nodo_bloq_por_recurso *b = data;
	list_destroy_and_destroy_elements(b->personajes, personaje_destroyer);
	free(b);
}


bool personaje_esta_en_colas()//protip: no leer esta funcion
{
	t_link_element *aux;

	for(aux=((t_list*)est->colas[LISTOS])->head; aux!=NULL && ((t_nodo_personaje*)aux->data)->socket!=personaje->socket; aux=aux->next);

	if(aux!=NULL)
		return true;
	else
		aux=((t_list*)est->colas[BLOQUEADOS])->head;

	while(aux!=NULL)
	{
		t_link_element *a;

		for(a=((t_nodo_bloq_por_recurso*)aux->data)->personajes->head; a!=NULL && ((t_nodo_personaje*)a->data)->socket!=personaje->socket; a=a->next);
		if(a!=NULL) return true;

		aux=aux->next;
	}
	return false;
}
