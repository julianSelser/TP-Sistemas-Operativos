/*
 * mainNivel.c
 *
 *  Created on: 02/06/2013
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

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <tad_items.h>
#include <nivel.h> //nivel gui de la catedra

#include "Nivel.h"
#include "serial.h"

//hilo principal del proceso nivel
//lee el archivo de configuración, crea el logger y lanza los demás hilos
//tambien inicializa los contadores de recursos
char *nombre;
char *ip_orquestador;
int puerto_orquestador, socket_orquestador;
int tiempo_chequeo_deadlock;
int recovery;
int filas, columnas;
uint8_t cantidad_de_recursos;
t_config * configuracion;

t_log *logger;
t_list *lista_cajas;
t_list *lista_personajes;
ITEM_NIVEL * lista_items = NULL;


int main(int argc, char ** argv)
{
	int i;
    pthread_t hilo_deadlock;

    fd_set maestro;    	// fd maestro
    fd_set read_fds;  	// fds de lectura, por ahora solo uso este
    int fdmax;        	// max fd
    int nuevo_fd;   	// fd de un nuevo socket que se conecta

	lista_cajas = list_create();
	lista_personajes = list_create();

	levantar_config(argc,argv);
	iniciar_serializadora();

	//lineas que muestran detalles del nivel antes de lanzar la interfaz grafica
	printf("\n ip orquestador: %s\n nombre: %s\n", ip_orquestador, nombre);
	sleep(1);

    printf("\n...abriendo puerto para escuchar conexiones...\n");
    int escucha = init_socket_escucha(0, 1, logger); //elige puerto libre dinamicamente
    sleep(1);

    printf("...conectando al orquestador...\n");
    socket_orquestador = init_socket_externo(puerto_orquestador, ip_orquestador, logger);
    sleep(1);

	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&filas, &columnas);
	nivel_gui_dibujar(lista_items);

	//lanzar el hilo de deteccion de deadlock
    pthread_create(&hilo_deadlock, NULL, (void*)rutina_chequeo_deadlock, NULL);

	/*	...A PARTIR DE ACA SE COMIENZAN A PROCESAR MENSAJES...	*/

    FD_ZERO(&maestro);
    FD_ZERO(&read_fds);
    FD_SET(escucha, &maestro);

    //necesito tener el fdmax (max valor de socket)
    fdmax = socket_orquestador > escucha? socket_orquestador : escucha;

    while(1){
        read_fds = maestro;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        // loopea los fd's
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // buscar los seteados
                if (i == escucha) {
                    // si es el escucha se tiene un nuevofd
                    nuevo_fd = accept(escucha,NULL,0);
                    if (nuevo_fd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(nuevo_fd, &maestro);
                        //chequear si el nuevo es mas grande que el maximo
                        if (nuevo_fd > fdmax) {
                            fdmax = nuevo_fd;
                        }
                    }
                } else {
                    // si no es el escucha, es una comunicacion
                	if(is_connected(i))  	//viene un mensaje del tipo de "next msg"
                	{
                	manejar_peticion(i);	//y manejarlos
                	}
                	else{ // si no esta conectado, cerrarlo y sacarlo del set maestro
                		close(i);
                		FD_CLR(i, &maestro);
                	}
                } // fin manejo comunicacion
            } // fin conexion entrante
        } // fin loop
    }

    sleep(10); //DESPUES BORRAR ESTE SLEEP, ESTA PARA PODER VER LA PANTALLA

    nivel_gui_terminar();

	return EXIT_SUCCESS;
}


void manejar_peticion(int socket){
	switch(getnextmsg(socket))
	{
	case ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL:
										manejar_ingreso_personaje(socket);
										break;
	case SOLICITUD_MOVIMIENTO_XY:
										manejar_solicitud_movimiento(socket);
										break;
	case SOLICITUD_UBICACION_RECURSO:
										manejar_solicitud_ubicacion_recurso(socket);
										break;
	case SOLICITUD_INSTANCIA_RECURSO:

										manejar_solicitud_instancia_recurso(socket);
										break;
	case NOTIF_ELECCION_VICTIMA:
										manejar_notif_eleccion_victima(socket);
										break;
	case NOTIF_RECURSOS_REASIGNADOS:
										manejar_recursos_reasignados(socket);
										break;
	default:
			printf("\n\n\nANTECION: MENSAJE NO CONSIDERADO, TIPO: %d\n\n\n", getnextmsg(socket));
			break;
	}
}


void manejar_ingreso_personaje(int socket){
	int i;
	t_list *lista;
	t_link_element *aux;

	t_datos_delPersonaje_alNivel *datos = recibir(socket, ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL);
	t_nodo_personaje *nodo_p = malloc(sizeof(t_nodo_personaje));

	nodo_p->ID = datos->char_personaje;
	nodo_p->nombre = (char *)datos->nombre_personaje;
	nodo_p->socket = socket;
	nodo_p->x = 0;
	nodo_p->y = 0;
	lista = nodo_p->necesidades = list_create();

	for(i=0; datos->necesidades[i] != '\0' ;i++)
	{
		//busca si la necesidad estaba enlistada
		for( aux = lista->head; aux!=NULL && ((t_necesidad*)aux->data)->ID_recurso != datos->necesidades[i] ; aux = aux->next);

		//si estaba: aumenta el maximo; sino se enlista
		if(aux != NULL) ((t_necesidad*)aux->data)->max++;
		else{
			t_necesidad *nec = malloc(sizeof(t_necesidad));
			nec->ID_recurso = datos->necesidades[i];
			nec->asig = 0;
			nec->max = 1;
			list_add(nodo_p->necesidades, nec);
		}
	}

	CrearPersonaje(&lista_items, nodo_p->ID, 0, 0);
	nivel_gui_dibujar(lista_items);

	free(datos->necesidades);
	free(datos);
}


void manejar_solicitud_movimiento(int socket){
	t_link_element *aux;
	t_nodo_personaje *personaje;
	t_resp_solicitud_movimiento *respuesta = malloc(sizeof(t_resp_solicitud_movimiento));

	t_solicitud_movimiento *solicitud = recibir(socket, SOLICITUD_MOVIMIENTO_XY);

	if(solicitud->x<columnas && solicitud->y<filas) //si esta dentro del nivel...
	{
		//buscamos el personaje en la lista de personajes...
		for(aux=lista_personajes->head; aux!=NULL && ((t_nodo_personaje*)aux->data)->ID != solicitud->char_personaje; aux=aux->next);

		personaje = ((t_nodo_personaje*)aux->data);

		personaje->x = solicitud->x;
		personaje->y = solicitud->y;

		respuesta->resp_solicitud = true; 	//permiso dado
	}
	else respuesta->resp_solicitud = false;	//comela

	enviar(socket, RTA_SOLICITUD_MOVIMIENTO_XY, respuesta, logger); //contestacion con la respuesta

	free(solicitud);
}

void manejar_solicitud_ubicacion_recurso(int socket){

}
void manejar_solicitud_instancia_recurso(int socket){}
void manejar_notif_eleccion_victima(int socket){}
void manejar_recursos_reasignados(int socket){}

/*	Estos hay que contestar	*/
/*#define RTA_SOLICITUD_INSTANCIA_RECURSO 8			//PN->PP
#define NOTIF_RECURSOS_LIBERADOS 13					//PN->HO
#define SOLICITUD_RECUPERO_DEADLOCK 15				//PN->HO
#define INFO_UBICACION_RECURSO 20					//PN->PP*/


int conf_es_valida(t_config * configuracion)
{
	return(config_has_property(configuracion, "Nombre") &&
			config_has_property(configuracion, "TiempoChequeoDeadlock") &&
			config_has_property(configuracion, "Recovery") &&
			config_has_property(configuracion, "Caja1") && //al menos una caja de recursos
			config_has_property(configuracion, "orquestador")
			);
}


int imprimir_nodo_caja(t_caja * nodo_caja)
{
	printf("Hay una caja que contiene %d %s de %d, su símbolo es %c y se encuentra en la posición (%d,%d)\n", nodo_caja->disp, nodo_caja->nombre, nodo_caja->disp, nodo_caja->ID, nodo_caja->x, nodo_caja->y);
	sleep(1);
	return 0;
}


//permite pasar desde NUESTRA estructura de caja, a la estructura NIVEL_ITEMS requerida para dibujar
int crear_item_caja_desde_nodo(t_caja * nodo_caja)
{
	CrearCaja(&lista_items, nodo_caja->ID, nodo_caja->x, nodo_caja->y, nodo_caja->disp);
	return 0;
}


void levantar_config(int argc, char ** argv){
	int i;
	char *log_name;
	char **ip_puerto_separados;
	char *ip_puerto_orquestador;
	char *temp_ip_puerto_orq;

	if(argc != 2) //controlar que haya exactamente un parámetro
	{
		puts("Uso: nivel <arch.conf>\n");//todo: logear
		exit(1);
	}

	configuracion = config_create(argv[1]);

	if (!conf_es_valida(configuracion)) //ver que el archivo de config tenga todito
	{
		puts("Archivo de configuración incompleto o inválido.\n"); //todo: logear
		exit(1);
	}

	char *temp_nombre = config_get_string_value(configuracion, "Nombre");

	nombre = malloc(strlen(temp_nombre)+1);
	strcpy(nombre, temp_nombre);

	//ahora que ya se como se llama el nivel, puedo crear el log
	log_name = malloc(strlen(nombre)+1);
	strcpy(log_name, nombre);

	string_append(&log_name, ".log");
	logger = log_create(log_name, "NIVEL", 0, LOG_LEVEL_TRACE);

	//podra el personaje reutilizar este codigo?
	temp_ip_puerto_orq = config_get_string_value(configuracion, "orquestador");
	ip_puerto_orquestador = malloc(strlen(temp_ip_puerto_orq)+1);
	strcpy(ip_puerto_orquestador, temp_ip_puerto_orq);
	ip_puerto_separados = string_split(ip_puerto_orquestador, ":");
	free(ip_puerto_orquestador);

	ip_orquestador = malloc(strlen(ip_puerto_separados[0]+1));
	strcpy(ip_orquestador, ip_puerto_separados[0]);
	puerto_orquestador = atoi(ip_puerto_separados[1]);
	free(ip_puerto_separados);

	tiempo_chequeo_deadlock = config_get_int_value(configuracion, "TiempoChequeoDeadlock");
	recovery = strcmp("On", config_get_string_value(configuracion, "Recovery"));

	cantidad_de_recursos = config_keys_amount(configuracion) - 4; //este 4 esta hardcodeado, pero la realidad es que siempre el archivo de config tiene la cantidad de cajas y cuatro entradas más

	for(i=1; i<=cantidad_de_recursos; i++)
	{
		char ** datos_caja;
		char * nombre_caja;
		char numero_caja[2];
		t_caja * nodo_caja;

		//todo revisar condiciones de error de malloc?

		numero_caja[0] = i+0x30; //quick fix a falta de itoa
		numero_caja[1] = '\0';

		nombre_caja = malloc(5); //porque la palabra Caja ocupa 4 bytes
		strcpy(nombre_caja, "Caja");
		string_append(&nombre_caja, numero_caja); //o sea, Caja1, Caja2, etc

		datos_caja = config_get_array_value(configuracion, nombre_caja);

		nodo_caja = malloc(sizeof(t_caja));

		nodo_caja->nombre = malloc (strlen(datos_caja[0]));
		strcpy(nodo_caja->nombre, datos_caja[0]); //nombre es un puntero

		nodo_caja->ID = datos_caja[1][0];
		nodo_caja->disp = nodo_caja->total = atoi(datos_caja[2]); //todo verificar condicion de error de atoi?
		nodo_caja->x = atoi(datos_caja[3]); //todo idem
		nodo_caja->y = atoi(datos_caja[4]); //todo idem

		crear_item_caja_desde_nodo(nodo_caja);
		list_add(lista_cajas, (void *) nodo_caja);

		free(datos_caja);
	}
	printf("\n Listado de cajas:\n\n");
	sleep(1);
	for (i=0; i < cantidad_de_recursos; i++)	imprimir_nodo_caja((t_caja *)list_get(lista_cajas, i));
	//linea para controlar que se haya enlistado todito bien

	//en este punto, se termino de leer el archivo de config y se enlistaron todos los recursos
	config_destroy(configuracion);
}
