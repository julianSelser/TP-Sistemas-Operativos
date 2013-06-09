/*
 * hiloEscuchaConexiones.c
 *
 *  Created on: 08/06/2013
 *      Author: utnso
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <commons/collections/list.h>
#include <commons/string.h>

#include "nivel_defs.h"

#define BUFF_SIZE 1024

t_list * lista_personajes;

void * iniciarEscucha(int puerto)

{
  int socket_requestor;
	int socket_escucha;
	struct sockaddr_in infoRequestor;
	struct sockaddr_in infoEscucha;
	int puerto_escucha;
	char buffer[BUFF_SIZE];

	lista_personajes = list_create();
	//primero se comporta como un cliente
	//despues el orquestador le dice por donde escuchar
	//ahi empieza a comportarse como un servidor

	//primero tengo que obtener la ip y el puerto


	if ((socket_requestor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("No se pudo crear el socket!\n");
		exit(-3);
	}

	infoRequestor.sin_family = AF_INET;
	infoRequestor.sin_addr.s_addr = inet_addr(ip_orquestador);
	infoRequestor.sin_port = htons(puerto_orquestador);

	if ((connect(socket_requestor, (struct sockaddr *) &infoRequestor, sizeof(infoRequestor))) != 0)
	{
		printf("Error al conectarse al orquestador. Está levantado?\n");
		exit(-4);
	}

	//puerto_escucha = recibir(socket_requestor, PUERTO_PARA_ESCUCHAR);


	if ((socket_escucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) //creo socket escucha
	{
		printf("No se pudo crear el socket!\n");
		exit(-3);
	}

	infoEscucha.sin_family = AF_INET;
	infoEscucha.sin_addr.s_addr = INADDR_ANY;
	infoEscucha.sin_port = htons(puerto_escucha); //escucho por la dirección que me dijo el orquestador


	return NULL; //para evitar el warning
}

