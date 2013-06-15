/* serializadora_test.c
 *
 *  Created on: Jun 12, 2013
 *      Author: julian
 */
/*
#include <stdint.h>
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
#include <semaphore.h>

#include "serial.h"

void envio();
void recibo();

#define PUERTO 5000

sem_t x;
sem_t y;

int main(void){
	iniciar_serializadora();

	recibo();

	sem_init(&x,0,0);
	sem_init(&y,0,0);

	pthread_t a,b;
	pthread_create(&b,NULL,recibo,NULL);
	pthread_create(&a,NULL,envio,NULL);

	pthread_join(a,NULL);
	pthread_join(b,NULL);

	return 0;
}

void envio(){

	sem_wait(&x);
	int socket = init_socket_externo(PUERTO, "127.0.0.1", NULL);

	t_turno_concluido *t = malloc(sizeof(t_turno_concluido));
	t->bloqueado = true; t->termino_nivel = true; t->recurso_de_bloqueo = 'G';

	enviar(socket, NOTIF_TURNO_CONCLUIDO, t, NULL);

	sem_post(&y);

	close(socket);
}


void recibo()
{
	int socketNuevaConexion;
	int socket = init_socket_escucha(PUERTO,1,NULL);
	printf("\n\n entra al server\n\n");

	listen(socket, 10);
	//sem_post(&x);

	socketNuevaConexion = accept(socket, NULL, 0);

	if(NOTIF_TURNO_CONCLUIDO==getnextmsg(socketNuevaConexion)) printf("\n recibe bien el msg\n\n");

	//sem_wait(&y);
	t_turno_concluido *turn = recibir(socketNuevaConexion,NOTIF_TURNO_CONCLUIDO);

	if(turn!=NULL) {
		if(turn->bloqueado==true && turn->termino_nivel==true && turn->recurso_de_bloqueo=='G'){
			printf("\n Recibido lo que se esperaba:\nbloq: %d , termino: %d, caracter: %c\n\n",turn->bloqueado,turn->termino_nivel,turn->recurso_de_bloqueo);
		}
		else printf("\n Viene algo que no estaba dentro de lo esperado\n bloq: %d , termino: %d, caracter: %c\n\n",turn->bloqueado,turn->termino_nivel,turn->recurso_de_bloqueo);
	}
	else printf("\n no trae bien el deserializado \n\n");

	free(turn);
	close(socket);
	close(socketNuevaConexion);
}
*/
