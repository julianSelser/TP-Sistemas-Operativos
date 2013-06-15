/*
 * stub_personaje.c
 *
 *  Created on: Jun 14, 2013
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

#include "serial.h"



#define PUERTO 7000

typedef struct{
	pthread_t hilo;
	char id_char;
} t_per;



void bloquear_algun_personaje();
void crear_personaje();
void rutina_personaje();
t_turno_concluido *armar_turn_c();
t_datos_delPersonaje_alPlanificador *armar_datos();



sem_t b;
bool bloq = false;
char c, car_personaje = 100;

int main(void){

	iniciar_serializadora();
	sem_init(&b,0,1);

	printf("\n\n conectando como un nivel para crear el planificador...\n\n");
	int socketnivel = init_socket_externo(10000, "127.0.0.1", NULL);

	printf("\n\n stub del personaje, recibiendo input...\n\n");
	printf("comando: ");

	while(1){
		c = getchar();

		if(c=='f') break;

		if(c=='b') bloquear_algun_personaje();

		if(c=='n') rutina_personaje(); //crear_personaje();

	}

	printf("\n terminando...\n\n");
	sem_destroy(&b);

	return EXIT_SUCCESS;
}


void bloquear_algun_personaje(){

	printf("\n se procede a bloquear algun personaje\n\n");

	sem_wait(&b);
	bloq = true;
	sem_post(&b);
}

void crear_personaje(){
	pthread_t nuevo;
	car_personaje++;
	pthread_create(&nuevo, NULL, (void*)rutina_personaje, NULL);

}

void rutina_personaje(){

	printf("\n se ha creado el personaje %c\n\n", car_personaje);

	int socket = init_socket_externo(PUERTO,"127.0.0.1",NULL);


	enviar(socket, ENVIO_DE_DATOS_AL_PLANIFICADOR, (void*)armar_datos(), NULL);



	while(1){
		recibir(socket, NOTIF_MOVIMIENTO_PERMITIDO);
		printf("\n personaje recibio notif_turno_permitido\n\n");
		enviar(socket, NOTIF_TURNO_CONCLUIDO, armar_turn_c(), NULL);
		printf("\n comunicacion concretada correctamente\n\n");
	}

}

/* ARMADO DE MENSAJES */


t_turno_concluido *armar_turn_c(){
	t_turno_concluido *t_c = malloc(sizeof(t_turno_concluido));


	t_c->recurso_de_bloqueo = '0';
	t_c->termino_nivel = false;

	sem_wait(&b);
	t_c->bloqueado = bloq;
	bloq  = false;
	sem_post(&b);

	return t_c;
}


t_datos_delPersonaje_alPlanificador *armar_datos(){
	t_datos_delPersonaje_alPlanificador *datos = malloc(sizeof(t_datos_delPersonaje_alPlanificador));

	datos->char_personaje = car_personaje;

	return datos;
}

