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

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <tad_items.h>
#include <nivel.h> //nivel gui de la catedra

#include "Nivel.h"
#include "serial.h"
#include "rutina_chequeo_deadlock.h"

//hilo principal del proceso nivel
//lee el archivo de configuración, crea el logger y lanza los demás hilos
//tambien inicializa los contadores de recursos
char * nombre;
char * ip_orquestador;
int puerto_orquestador;
int tiempo_chequeo_deadlock;
int recovery;
uint8_t cantidad_de_recursos;
t_config * configuracion;
t_vec_finish *


t_log * logger;
t_list * lista_cajas;
ITEM_NIVEL * lista_items = NULL;;

void manejar_peticion(int socket, int tipo, void *data);

int tomar_config_por_parametro(int argc, char **argv);
int conf_es_valida(t_config * configuracion);
int imprimir_nodo_caja(t_caja * nodo_caja);
int crear_item_caja_desde_nodo(t_caja * nodo_caja);

int main(int argc, char ** argv)
{
	char *log_name;
	char **ip_puerto_separados;
	char *ip_puerto_orquestador;
	char *temp_ip_puerto_orq;



	if(argc > 1){
		int ret;
		if(!(ret = tomar_config_por_parametro(argc,argv))) exit(ret);
	}
	else configuracion = config_create("arch.conf");


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

	lista_cajas = list_create();

	int i;
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

	//en este punto, se termino de leer el archivo de config y se enlistaron todos los recursos
	config_destroy(configuracion);

	for (i=0; i< cantidad_de_recursos; i++)	imprimir_nodo_caja((t_caja *)list_get(lista_cajas, i));
	//linea para controlar que se haya enlistado todito bien

/*	REFACTOREAR ANTERIOR; A PARTIR DE ACA SELECT	*/


    fd_set maestro;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    int nuevo_fd;        // newly accept()ed socket descriptor
    int escucha = init_socket_escucha(5000,1,NULL);

    pthread_t hilo_deadlock;

    pthread_create(&hilo_deadlock, NULL, rutina_chequeo_deadlock, &tiempo_chequeo_deadlock);

    FD_ZERO(&maestro);
    FD_ZERO(&read_fds);

    FD_SET(escucha, &maestro);

    // keep track of the biggest file descriptor
    fdmax = escucha; // so far, it's this one

    while(1){
        read_fds = maestro;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
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
                        if (nuevo_fd > fdmax) {
                            fdmax = nuevo_fd;
                        }
                    }
                } else {
                    // si no es el escucha, es una comunicacion
                	int tipo;

                	if(( tipo = getnextmsg(i) ))  //viene un mensaje del tipo de "next msg"
                	{
                	void* data = recibir(i,getnextmsg(i)); 	//recibir los datos
                	manejar_peticion(i, tipo, data);		//y manejarlos
                	}
                	else{ // tipo cero: desconexion
                		close(i);
                		FD_CLR(i, &maestro);
                	}
                } // fin manejo comunicacion
            } // fin conexion entrante
        } // fin loop
    }



	return 0; //para evitar el warning
}


void manejar_peticion(int socket, int tipo, void *data){
	switch(tipo){
	}
}


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
	printf("Esta caja contiene %d %s de %d, su símbolo es %c y se encuentra en la posición (%d,%d)\n", nodo_caja->disp, nodo_caja->nombre, nodo_caja->disp, nodo_caja->ID, nodo_caja->x, nodo_caja->y);
	return 0;
}


int crear_item_caja_desde_nodo(t_caja * nodo_caja)
//permite pasar desde NUESTRA estructura de caja, a la estructura NIVEL_ITEMS requerida para dibujar
{
	CrearCaja(&lista_items, nodo_caja->ID, nodo_caja->x, nodo_caja->y, nodo_caja->disp);
	return 0;
}


int tomar_config_por_parametro(int argc, char **argv){
	if(argc != 2) //controlar que haya exactamente un parámetro
	{
		puts("Uso: nivel <arch.conf>\n");
		return -1;
	}

	configuracion = config_create(argv[1]);

	if (!conf_es_valida(configuracion)) //ver que el archivo de config tenga todito
	{
		puts("Archivo de configuración incompleto o inválido.\n");
		return -2;
	}
	return 0;
}