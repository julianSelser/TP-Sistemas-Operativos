/*
 * rutina_orquestador.c
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */

#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "rutina_orquestador.h"
#include "rutina_planificador.h"
#include "plataforma.h"

#include "serial.h"


// El tamaño de un evento es igual al tamaño de la estructura de inotify
// mas el tamaño maximo de nombre de archivo que nosotros soportemos
// en este caso el tamaño de nombre maximo que vamos a manejar es de 24
// caracteres. Esto es porque la estructura inotify_event tiene un array
// sin dimension ( Ver C-Talks I - ANSI C ).
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )

// El tamaño del buffer es igual a la cantidad maxima de eventos simultaneos
// que quiero manejar por el tamaño de cada uno de los eventos. En este caso
// Puedo manejar hasta 1024 eventos simultaneos.
#define BUF_LEN     ( 1024 * EVENT_SIZE )


static int puerto = 7000;

void rutina_orquestador(/*?*/)
{

	pthread_t inotify;
	pthread_create(&inotify,NULL,(void*)rutina_inotify,NULL);

	int socketNuevoNivel;
	int socketEscucha = init_socket_escucha(10000, 1, NULL);

	printf("\n a la espera de nuevos niveles...\n\n");
	while(1){
			socketNuevoNivel = accept(socketEscucha, NULL, 0);
			lanzar_planificador();
	}

}

void lanzar_planificador()
{
	pthread_t nuevo_hilo;
	pthread_create(&nuevo_hilo, NULL, (void*)rutina_planificador, armar_parametro());
}

parametro *armar_parametro()
{
	int i;
	parametro *p = malloc(sizeof(parametro));
	t_nodo_bloq_por_recurso *nodo = malloc(sizeof(t_nodo_bloq_por_recurso));

	for(i=0;i<2;i++){
		p->colas[i] = list_create();//la cola de bloqueados hay que inicializarla de una manera mas complicada
	}

	nodo->char_recurso = '0';
	nodo->personajes = list_create();
	list_add(p->colas[BLOQUEADOS],nodo);

	typedef struct{
		char char_recurso;
		t_list * personajes;
	}t_nodo_bloq_por_recurso;

	p->logger_planificador = NULL;
	p->puerto = puerto++;


	return p;
}

void rutina_inotify()
{
		t_config *plataforma_conf;

		char buffer[BUF_LEN];

		// Al inicializar inotify este nos devuelve un descriptor de archivo
		int file_descriptor = inotify_init();
		if (file_descriptor < 0) {
			perror("inotify_init");
		}

		// Creamos un monitor sobre un path indicando que eventos queremos escuchar

		int watch_descriptor = inotify_add_watch(file_descriptor, (char*)get_current_dir_name(), IN_MODIFY);

		// El file descriptor creado por inotify, es el que recibe la información sobre los eventos ocurridos
		// para leer esta información el descriptor se lee como si fuera un archivo comun y corriente pero
		// la diferencia esta en que lo que leemos no es el contenido de un archivo sino la información
		// referente a los eventos ocurridos

		while(1){
			int length = read(file_descriptor, buffer, BUF_LEN);
			if (length < 0) {
				perror("read");
			}

			int offset = 0;

			// Luego del read buffer es un array de n posiciones donde cada posición contiene
			// un eventos ( inotify_event ) junto con el nombre de este.
			while (offset < length) {

				// El buffer es de tipo array de char, o array de bytes. Esto es porque como los
				// nombres pueden tener nombres mas cortos que 24 caracteres el tamaño va a ser menor
				// a sizeof( struct inotify_event ) + 24.
				struct inotify_event *event = (struct inotify_event *) &buffer[offset];

				// El campo "len" nos indica la longitud del tamaño del nombre
				if (event->len) {
					// Dentro de "mask" tenemos el evento que ocurrio y sobre donde ocurrio
					// sea un archivo o un directorio
					if (event->mask & IN_MODIFY) {
						if(!strcmp(event->name,"arch.conf"))
						{
							plataforma_conf = config_create("arch.conf");

							quantum = config_get_int_value(plataforma_conf, "quantum");
							retraso = config_get_int_value(plataforma_conf, "retraso");
							printf("\n modificacion en el archivo de configuracion...\n quantum: %d\n retraso: %d\n\n",quantum,retraso);

							config_destroy(plataforma_conf);
						}
					}
				}
				offset += sizeof (struct inotify_event) + event->len;
			}
		}
		inotify_rm_watch(file_descriptor, watch_descriptor);
		close(file_descriptor);

}
