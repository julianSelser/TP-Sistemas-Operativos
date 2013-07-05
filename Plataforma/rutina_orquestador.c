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
#include <commons/string.h>
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
#include "rutina_planificador.h"  //y bueno, ya fue
#include "plataforma.h"

#include <serial.h>


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

t_list * lista_niveles;

static int puerto_planif = 7000;
void manejar_anuncio_nivel(int socket_nivel);
void manejar_sol_info(int socket_nivel);
t_info_nivel_planificador * crear_info_nivel(char * nombre);
void manejar_recs_liberados(int socket);
t_nodo_nivel * ubicar_nivel_por_socket(int socket);
t_nodo_bloq_por_recurso * ubicar_cola_por_rec(t_list * lista_colas, char ID_rec);


void rutina_orquestador(/*?*/)
{
	pthread_t inotify;
	pthread_create(&inotify,NULL,(void*)rutina_inotify,NULL);

	int socketNuevoNivel;
	int socketEscucha = init_socket_escucha(10000, 1, NULL);

	lista_niveles=list_create();

	printf("\n a la espera de nuevos niveles...\n\n");
	while(1){
			socketNuevoNivel = accept(socketEscucha, NULL, 0);
			//no lanzo el planificador inmediatamente, sino que espero a que el nivel me mande el mensaje de anuncio
	}

}

void manejar_anuncio_nivel(int socket_nivel) //faltan las ip, pero funciona
{
	t_envio_deDatos_delNivel_alOrquestador * datos_nivel_entrante;
	t_nodo_nivel * nuevo_nivel;
	int i=0;

	datos_nivel_entrante = recibir(socket_nivel, ENVIO_DE_DATOS_NIVEL_AL_ORQUESTADOR);

	nuevo_nivel = malloc(sizeof (t_nodo_nivel));
	nuevo_nivel->socket = socket_nivel;

	nuevo_nivel->colas[0] = list_create();
	nuevo_nivel->colas[1] = list_create();
	//nuevo_nivel->IP todo asignar a esta variable la IP del nivel!!! no se como se hace
	nuevo_nivel->puerto = datos_nivel_entrante->puerto_nivel;
	/*nuevo_nivel->IP = malloc(strlen(IP_local)+1)
	strcpy(nuevo_nivel->IP, IP_local)*/
	nuevo_nivel->puerto_planif = puerto_planif;
	nuevo_nivel->nombre=(char *)(datos_nivel_entrante->nombre);
	//esa ultima linea funciona solo si la informacion a la que apunta nombre no se libera

	while(datos_nivel_entrante->recursos_nivel[i]!='\0') //recorrer los recursos que presenta el niel
	{
		t_nodo_bloq_por_recurso * info_recurso;
		info_recurso = malloc(sizeof(t_nodo_bloq_por_recurso));
		info_recurso->char_recurso=datos_nivel_entrante->recursos_nivel[i];
		info_recurso->personajes=list_create();
		list_add(nuevo_nivel->colas[BLOQUEADOS], info_recurso); //crear cola de bloqueados para el recurso actual
		i++;
	}

	nuevo_nivel->colas[LISTOS] = list_create();

	free(datos_nivel_entrante->recursos_nivel);
	free(datos_nivel_entrante);

	list_add(lista_niveles, nuevo_nivel);

	lanzar_planificador(nuevo_nivel->colas);
	puerto_planif++;
}

void lanzar_planificador(t_list ** colas)
{
	pthread_t nuevo_hilo;
	pthread_create(&nuevo_hilo, NULL, (void*)rutina_planificador, armar_parametro(colas));
}

parametro *armar_parametro(t_list ** colas)
{
	parametro *p = malloc(sizeof(parametro));

	//ya arme las colas desde antes

	p->colas[LISTOS]=colas[LISTOS];
	p->colas[BLOQUEADOS]=colas[BLOQUEADOS];
	p->logger_planificador = NULL;
	p->puerto = puerto_planif;

	return p;
}


void manejar_sol_info(int socket) //todo testear
{
	t_info_nivel_planificador * info;
	t_solicitud_info_nivel * solicitud;

	solicitud = recibir(socket, SOLICITUD_INFO_NIVEL);
	info = crear_info_nivel((char *)solicitud->nivel_solicitado);

	enviar(socket, INFO_NIVEL_Y_PLANIFICADOR, info, NULL); //TODO AGREGAR LOGGER!!!
	free(solicitud->nivel_solicitado);
	free(solicitud);
}

t_info_nivel_planificador * crear_info_nivel(char * nombre)
{
	t_info_nivel_planificador * temp;
	int cant_niveles;
	int i=0;

	temp=malloc(sizeof(t_info_nivel_planificador));
	cant_niveles=list_size(lista_niveles);


	while(i<cant_niveles)
	{
		t_nodo_nivel * nivel_actual;

		nivel_actual = (t_nodo_nivel *)list_get(lista_niveles, i);
		if(strcmp(nivel_actual->nombre, nombre))
		{
			temp->ip_nivel=nivel_actual->IP;
			temp->puerto_nivel=nivel_actual->puerto;
			temp->ip_planificador=nivel_actual->IP_planif;
			temp->puerto_planificador=nivel_actual->puerto_planif;
			return temp;
		}
		i++;
	}

	return NULL;
}


void manejar_recs_liberados(int socket) //todo testear
{
	char * liberados;
	char * resto;
	char * reasignaciones;
	char rec;
	char rec_ant;
	t_nodo_nivel * nivel;
	t_notif_recursos_liberados * notificacion;
	t_notif_recursos_reasignados * informe;
	int i;

	nivel = ubicar_nivel_por_socket(socket);

	resto=malloc(1);
	resto[0]='\0';

	reasignaciones=malloc(1);
	reasignaciones[0]='\0';

	rec_ant='\0';

	notificacion = recibir(socket, NOTIF_RECURSOS_LIBERADOS);
	liberados=(char *)notificacion->recursos_liberados;
	free(notificacion); //aunque libere notificacion, liberados sigue existiendo


	while(liberados[i]!='\0')
	{
		static t_nodo_bloq_por_recurso * nodo_cola;

		rec=liberados[i];

		if (rec!=rec_ant) nodo_cola = ubicar_cola_por_rec(nivel->colas[BLOQUEADOS], rec);

		if(list_size(nodo_cola->personajes)==0) //si no hay nadie bloqueado por ese recurso
		{
			char aux[2];
			aux[0]=rec;
			aux[1]='\0';
			string_append(&resto, aux); //agrego el recurso al string de recursos que quedan definitivamente libres
		}

		else //si hay alguien para desencolar
		{
			char informe_parcial[3]; //esto es un string del tipo "@F", o sea "le di una flor a Mario"
			t_nodo_personaje * personaje;
			t_concesion_recurso * concedido;

			concedido=malloc(sizeof(t_concesion_recurso));
			concedido->recurso=rec;
			//------GUARDA, ACA ES DONDE HAY QUE SINCRONIZAR!!!!!--------// TODO
			personaje = desencolar(nodo_cola->personajes);

			informe_parcial[0]=personaje->char_personaje;
			informe_parcial[1]=rec;
			informe_parcial[2]='\0';
			string_append(&reasignaciones, informe_parcial);

			enviar(personaje->socket, NOTIF_RECURSO_CONCEDIDO, concedido, NULL); //todo agregar logger
			encolar(nivel->colas[LISTOS], personaje);
			//-----FIN SECCION CRITICA----///
		}

		rec_ant=rec;
		i++;
	} //fin de procesamiento del string de recursos liberados
	free(liberados);

	informe=malloc(sizeof(t_notif_recursos_reasignados));
	informe->asignaciones=(uint8_t *)reasignaciones;
	informe->remanentes=(uint8_t *) resto; //estos casteos, la verdad....

	enviar(nivel->socket, NOTIF_RECURSOS_REASIGNADOS, informe, NULL); //todo agregar logger

	//todo free reasignados and resto? creeeo que si
	//that's all folks
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

t_nodo_nivel * ubicar_nivel_por_socket(int socket)
{
	int cant_niveles;
	int i=0;

	cant_niveles=list_size(lista_niveles);

	while(i<cant_niveles)
	{
		t_nodo_nivel * nv_actual;

		nv_actual = (t_nodo_nivel *) list_get(lista_niveles, i);
		if (nv_actual->socket == socket) return nv_actual;
		i++;
	}

	return NULL; //deberia ser una busqueda segura y no llegar nunca aca
}

t_nodo_bloq_por_recurso * ubicar_cola_por_rec(t_list * lista_colas, char ID_rec)
{
	int cant_colas;
	int i=0;

	cant_colas = list_size(lista_colas);

	while(i<cant_colas)
	{
		t_nodo_bloq_por_recurso * cola_actual;
		cola_actual = (t_nodo_bloq_por_recurso *) list_get(lista_colas, i);
		if(cola_actual->char_recurso == ID_rec) return cola_actual;
		i++;
	}

	return NULL; //deberia ser una busqueda segura y no llegar nunca aca
}
