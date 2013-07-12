/*
 * mainNivel.c
 *
 *  Created on: 02/06/2013
 *      Author: utnso
 */
 
#include <ctype.h>
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
#include <serial.h>

//hilo principal del proceso nivel
//lee el archivo de configuración, crea el logger y lanza los demás hilos
//tambien inicializa los contadores de recursos
char *nombre, *recursos;
char *ip_orquestador;
int puerto_orquestador, socket_orquestador, escucha;
int tiempo_chequeo_deadlock;
int recovery;
int filas, columnas;
int cantidad_de_recursos;
t_config * configuracion;

t_log *logger;
t_list *lista_cajas;
t_list *lista_personajes;
ITEM_NIVEL * lista_items = NULL;

sem_t sem_general;
sem_t sem_recovery;

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
    printf("\n...abriendo puerto para escuchar conexiones...\n");
    escucha = init_socket_escucha(0, 1, logger); //elige puerto libre dinamicamente
    sleep(1);


    printf("...conectando al orquestador...\n");
    socket_orquestador = init_socket_externo(puerto_orquestador, ip_orquestador, logger);
    sleep(1);

    printf("...enviando datos del nivel al orquestador...\n");
    enviar(socket_orquestador, ENVIO_DE_DATOS_NIVEL_AL_ORQUESTADOR, msg_datos_delNivel_alOrquestador(), logger);//todo el mensaje al orquestador con los datos
    sleep(1);


    sem_init(&sem_general, 0, 1);
    sem_init(&sem_recovery, 0, 0);

	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&filas, &columnas);

	//lanzar el hilo de deteccion de deadlock
    pthread_create(&hilo_deadlock, NULL, (void*)rutina_chequeo_deadlock, NULL);

	/*	...A PARTIR DE ACA SE COMIENZAN A PROCESAR MENSAJES...	*/

    FD_ZERO(&maestro);
    FD_ZERO(&read_fds);
    FD_SET(escucha, &maestro);
    FD_SET(socket_orquestador, &maestro);

    //necesito tener el fdmax (max valor de socket)
    fdmax = socket_orquestador > escucha? socket_orquestador : escucha;

    while(1)
    {
        nivel_gui_dibujar(lista_items);

        read_fds = maestro;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
         //   perror("select");//todo loguear: error de select...
           log_error(logger,"error select","ERROR");
        	exit(1);
        }

        // loopea los fd's
        for(i = 0 ; i <= fdmax ; i++) {
            if (FD_ISSET(i, &read_fds)) { // buscar los seteados
                if (i == escucha) {
                    // si es el escucha se tiene un nuevofd
                    nuevo_fd = accept(escucha,NULL,0);
                    if (nuevo_fd == -1) {
                        //todo loguear: error aceptando nueva conexion
                    	log_error(logger,"error:aceptando nueva conexion","ERROR");
                    }
                    else {
                        FD_SET(nuevo_fd, &maestro);
                        //chequear si el nuevo fd es mas grande que el maximo
                        if (nuevo_fd > fdmax) {
                            fdmax = nuevo_fd;
                        }
                    }
                }
                else // si no es el escucha, es una comunicacion
                {
                	if(is_connected(i))  	//si el socket sigue estando conectado
                	{
                	manejar_peticion(i);	//es un mensaje, manejarlo
                	}
                	else // si se desconecto, ver si es el orquestador(en ese caso termina el juego), si es un personaje manejarlo
                	{
                		if(i==socket_orquestador)
                		{
                			/*	todo: FALTA DEDINIR QUE PASA CUANDO SE LLEGA A EXECVE Y EL ORQUESTADOR SE DESCONECTA	*/
                			nivel_gui_terminar();
                			exit(EXIT_SUCCESS);
                		}
                		else //se desconecto un personaje
                		{
                			int j;
                			t_link_element *aux;
                			t_nodo_personaje *p;

                			for(aux=lista_personajes->head,j=0 ; aux!=NULL && ((t_nodo_personaje*)aux->data)->socket!=i ; aux=aux->next,j++);

                			if(aux!=NULL)//si el personaje todavia se encontraba en la lista no se manejo su salida, hay que manejarla
                			{
                				p = list_remove(lista_personajes, j);

                				reubicar_recursos(p->necesidades);
                				BorrarItem(&lista_items, p->ID);

                				free(p->nombre);
                				free(p);
                			}
                		}
                		close(i);
                		FD_CLR(i, &maestro);
                	}
                } // fin manejo comunicacion
            } // fin conexion entrante
        } // fin loop
    }

    //todo: liberar la lista de items

    nivel_gui_terminar();

	return EXIT_SUCCESS;
}


void manejar_peticion(int socket){
	sem_wait(&sem_general); //no atiendo ningun mensaje mientras se esta detectando deadlock
	switch(getnextmsg(socket))
	{

	case ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL:
										manejar_ingreso_personaje(socket);
										break;
	case SOLICITUD_MOVIMIENTO_XY:
										manejar_solicitud_movimiento(socket);
										break;
	case NOTIF_NIVEL_CUMPLIDO:
										manejar_nivel_concluido(socket);
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
			//printf("\n\n\nANTECION: MENSAJE NO CONSIDERADO, TIPO: %d\n\n\n", getnextmsg(socket));//todo esto deberia loguearse como error
		  //    log_error(logger,string_from_format());
		break;
	} //end switch
	sem_post(&sem_general); //ya atendi el mensaje, permito detectar deadlock
}


void manejar_ingreso_personaje(int socket){
	//variables auxiliares y malloc de nuevo nodo
	int i;
	t_list *lista;
	t_link_element *aux;
	t_nodo_personaje *nodo_p = malloc(sizeof(t_nodo_personaje));

	//recibir
	t_datos_delPersonaje_alNivel *datos = recibir(socket, ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL);

	//inicializar nuevo nodo
	nodo_p->ID = datos->char_personaje;
	nodo_p->nombre = (char *)datos->nombre_personaje;
	nodo_p->socket = socket;
	nodo_p->x = 0;
	nodo_p->y = 0;
	lista = nodo_p->necesidades = list_create();

	//por cada necesidad enviada
	for(i=0 ; datos->necesidades[i] != '\0' ; i++)
	{
		//busca si la necesidad estaba enlistada
		for( aux = lista->head ; aux!=NULL && ((t_necesidad*)aux->data)->ID_recurso != datos->necesidades[i] ; aux = aux->next);

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

	list_add(lista_personajes, nodo_p);
	CrearPersonaje(&lista_items, nodo_p->ID, 1, 1);//crea el personaje en (1,1)

	free(datos->necesidades);
	free(datos);
}


void manejar_solicitud_movimiento(int socket){
	//variables auxiliares y malloc de respuesta
	t_link_element *aux;
	t_nodo_personaje *personaje;
	t_resp_solicitud_movimiento *respuesta = malloc(sizeof(t_resp_solicitud_movimiento));

	//recibir
	t_solicitud_movimiento *solicitud = recibir(socket, SOLICITUD_MOVIMIENTO_XY);

	if(solicitud->x<=columnas && solicitud->y<=filas) //si esta dentro del nivel...
	{
		//buscamos el personaje en la lista de personajes...
		for(aux=lista_personajes->head ; aux!=NULL && ((t_nodo_personaje*)aux->data)->ID != solicitud->char_personaje ; aux=aux->next);

		//si no se encontro personaje...
		if(aux==NULL)
			/*todo logear: error grotesco, el personaje que solicita moverse no estaba en la lista*/;

		personaje = ((t_nodo_personaje*)aux->data); //asignacion por claridad...

		//actualizar el personaje y dibujarlo
		personaje->x = solicitud->x;
		personaje->y = solicitud->y;

		respuesta->aprobado = true; 	//permiso dado

		MoverPersonaje(lista_items, personaje->ID, personaje->x, personaje->y);
	}
	else respuesta->aprobado = false;	//permiso negado

	enviar(socket, RTA_SOLICITUD_MOVIMIENTO_XY, respuesta, logger); //contestacion con la respuesta

	free(solicitud);
}


void manejar_nivel_concluido(int socket){
	//variables auxiliares
	int i;
	t_list *necesidades;
	t_nodo_personaje *personaje;
	t_link_element *aux = lista_personajes->head;

	//recibir
	t_notificacion_nivel_cumplido *conclusion = recibir(socket, NOTIF_NIVEL_CUMPLIDO);

	//buscar en la lista de personajes su nodo con el caracter identificador
	for(i=0 ; aux!=NULL &&((t_nodo_personaje*)aux->data)->ID != conclusion->char_personaje ; aux = aux->next, i++);

	//si no encontro personaje...
	if(aux==NULL)
		/*todo logear: error grotesco, no se encontro en la lista de personajes al que informa de haber cumplido el nivel*/;

	personaje = list_remove(lista_personajes, i); //saco el nodo de la lista y me lo guardo temporalmente
	necesidades = personaje->necesidades;

	//esta funcion le manda al orquestador los recursos
	reubicar_recursos(necesidades);

	BorrarItem(&lista_items, personaje->ID);

	free(personaje->nombre);
	free(personaje);
	free(conclusion);
}


void manejar_solicitud_ubicacion_recurso(int socket){
	//variables auxiliares y malloc de respuesta
	t_caja *caja;
	t_link_element *aux;
	t_ubicacion_recurso *ubicacion = malloc(sizeof(t_ubicacion_recurso));

	//recibir mensaje
	t_solicitud_ubicacion_recurso *solicitud_ubicacion = recibir(socket, SOLICITUD_UBICACION_RECURSO);

	//busco el recurso solicitado entre las cajas...
	for(aux=lista_cajas->head ; aux!=NULL && ((t_caja*)aux->data)->ID!=solicitud_ubicacion->recurso ; aux=aux->next);

	//si ciclo todas las cajas...
	if(aux == NULL)
		/*todo loguear error grotesco: no hay una caja del recurso pedido*/;

	//asignar valores a la respuesta
	caja = aux->data;
	ubicacion->x = caja->x;
	ubicacion->y = caja->y;

	enviar(socket, INFO_UBICACION_RECURSO, ubicacion, logger);

	free(solicitud_ubicacion);
}


void manejar_solicitud_instancia_recurso(int socket){
	//variables auxiliars y malloc de la respuesta
	t_caja *caja;
	t_necesidad *nec;
	t_link_element *aux;
	t_nodo_personaje *personaje;
	t_rspta_solicitud_instancia_recurso *respuesta_solicitud_instancia = malloc(sizeof(t_rspta_solicitud_instancia_recurso));

	//recibir mensaje
	t_solcitud_instancia_recurso *solicitud_instancia = recibir(socket, SOLICITUD_INSTANCIA_RECURSO);

	//buscamos la caja correspondiente al recurso solicitado
	for(aux=lista_cajas->head ; aux!=NULL && ((t_caja*)aux->data)->ID!=solicitud_instancia->instancia_recurso ; aux=aux->next);

	if(aux == NULL)
		/*todo loguear error grotesco: se esta pidiendo un recurso que no esta en ninguna caja*/;

	caja = aux->data; 	//asignacion por claridad


	if(caja->disp > 0)	//si hay recursos disponibles en la caja: restarlo, concederlo y dibujar; sino negarlo
	{
		caja->disp--;

		//buscar el personaje para asignarle el recurso concedido
		for(aux=lista_personajes->head ; aux!=NULL && ((t_nodo_personaje*)aux->data)->socket!=socket ; aux=aux->next);

		if(aux==NULL)/*todo loguear: error grotesco, el personaje pidiendo un recruso no estaba en la lista*/;

		personaje = aux->data;

		//buscar la necesidad a ser satisfecha para asignarla al personaje
		for(aux=personaje->necesidades->head ; aux!=NULL && ((t_necesidad*)aux->data)->ID_recurso!=caja->ID ; aux=aux->next);

		nec = aux->data;

		if(nec->asig < nec->max) nec->asig++;
		else
			/*todo loguear: un personaje esta pidiendo mas recursos que los inicialmente declarados*/;

		restarRecurso(lista_items, caja->ID);
		respuesta_solicitud_instancia->concedido = true;
	}
	else respuesta_solicitud_instancia->concedido=false;

	enviar(socket, RTA_SOLICITUD_INSTANCIA_RECURSO, respuesta_solicitud_instancia, logger);

	free(solicitud_instancia);
}


void manejar_notif_eleccion_victima(int socket){
	int i;
	char* nombre_victima;

	t_nodo_personaje *nodo_victima;
	t_link_element *aux = lista_personajes->head;
	t_notif_eleccion_de_victima *notif_victima = recibir(socket_orquestador, NOTIF_ELECCION_VICTIMA);

	for(i=0 ; aux!=NULL && ((t_nodo_personaje*)aux->data)->ID!=notif_victima->char_personaje ; aux=aux->next,i++);

	if(aux==NULL)
		/*todo loguear: la victima elegida no estaba en la lista, ver si esto puede llegar a pasar*/;
	else{
		nodo_victima = list_remove(lista_personajes, i);
		nombre_victima=strdup(nodo_victima->nombre);
        log_info(logger,string_from_format("la victima es :%s",nombre_victima));
		reubicar_recursos(nodo_victima->necesidades); //esto ya manda el mensaje de recursos liberados al orquestador

		BorrarItem(&lista_items, nodo_victima->ID);

		free(nodo_victima->nombre);
		free(nodo_victima);
		free(notif_victima);
	}
}


void manejar_recursos_reasignados(int socket){
	//aca el orquestador nos contesta la notificacion de recursos liberados
	//con los identificadores de los personajes y los recursos asignados
	//variable auxiliar para c/lista
	char *c;
	t_caja *caja;
	t_necesidad *necesidad;
	t_nodo_personaje *personaje;
	t_link_element *paux, *naux, *caux;

	t_notif_recursos_reasignados *reasignados = recibir(socket, NOTIF_RECURSOS_REASIGNADOS);

	//a cada personaje indicado por el orquestador le damos el recurso que le corresponde
	//recordar que tratamos con un string donde los caracteres pares son personajes y los impares recursos
	for(c=reasignados->asignaciones ; *c!='\0' ; c=c+2)
	{
		//buscamos el personaje por su id en la lista de personajes
		for(paux=lista_personajes->head ; paux!=NULL && ((t_nodo_personaje*)paux->data)->ID!=*c ; paux=paux->next);
		if(paux==NULL) /*todo loguear: personaje al que se le reasigno recursos no estaba en la lista de personajes*/;
		personaje = paux->data;

		//dentro de las necesidades del personaje buscamos el recurso asignado y se lo damos (incrementa asignacion)
		for(naux=personaje->necesidades->head ; naux!=NULL && ((t_necesidad*)naux->data)->ID_recurso!=*(c+1) ; naux=naux->next);
		if(naux==NULL) /*todo loguear: no se encontro el recurso a reasignar dentro de las necesidades del personaje*/;
		necesidad = naux->data;
		necesidad->asig++;
	}

	for(c=reasignados->remanentes; *c!='\0' ; c++){
		for(caux=lista_cajas->head ; caux!=NULL && ((t_caja*)caux->data)->ID!=*c ; caux=caux->next);
		if(caux==NULL) /*todo loguear: no se encontro una caja para asignar el recurso */;

		caja = caux->data;
		caja->disp++;

		sumarRecurso(lista_items, caja->ID);
	}

	free(reasignados->asignaciones);
	free(reasignados->remanentes);
	free(reasignados);

	sem_post(&sem_recovery); //con esto finaliza el procedimiento de recovery
}


void reubicar_recursos(t_list *necesidades){
	//variable auxiliares
	t_link_element *a;
	t_necesidad *nec_aux;
	t_notif_recursos_liberados *notificacion = malloc(sizeof(t_notif_recursos_liberados));
	char * recursos_liberados = strdup("");

	//mientras no se acabo la lista de necesidades
	for(a=necesidades->head ; a!=NULL ; a=a->next)
	{
		char * repeticion_recurso;
		nec_aux = a->data;

		if (nec_aux->asig > 0)
		{
			repeticion_recurso = string_repeat(nec_aux->ID_recurso, nec_aux->asig);
			string_append(&recursos_liberados, repeticion_recurso);
			free(repeticion_recurso);
		}
	}
	//borrar lista de necesidades y sus nodos de memoria
	list_destroy_and_destroy_elements(necesidades, free);

	notificacion->recursos_liberados = recursos_liberados;

	enviar(socket_orquestador, NOTIF_RECURSOS_LIBERADOS, notificacion, logger);
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


void *msg_datos_delNivel_alOrquestador(){
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	t_envio_deDatos_delNivel_alOrquestador *datos = malloc(sizeof(t_envio_deDatos_delNivel_alOrquestador));

	getsockname(escucha, (struct sockaddr *)&sin, &len);

	//asumiendo que no se vuelven a usar son liberados por el enviar que usa la funcion
	datos->nombre = nombre;
	datos->recursos_nivel = recursos;
	datos->puerto_nivel = ntohs(sin.sin_port); //el puerto asignado automaticamente

	return datos;
}


int imprimir_nodo_caja(t_caja * nodo_caja)
{
	printf("Caja en (%d,%d) con %d %s, su símbolo es %c\n", nodo_caja->x, nodo_caja->y, nodo_caja->disp, nodo_caja->nombre, nodo_caja->ID);
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

	recursos = malloc(cantidad_de_recursos + 1), recursos[cantidad_de_recursos] = '\0';

	for(i=1; i<=cantidad_de_recursos; i++)
	{
		char ** datos_caja;
		t_caja * nodo_caja;
		char * nombre_caja =  string_from_format("Caja%d", i);

		datos_caja = config_get_array_value(configuracion, nombre_caja);

		nodo_caja = malloc(sizeof(t_caja));

		nodo_caja->nombre = strdup(datos_caja[0]);
		nodo_caja->ID = datos_caja[1][0];
		nodo_caja->disp = nodo_caja->total = atoi(datos_caja[2]); //todo verificar condicion de error de atoi?
		nodo_caja->x = atoi(datos_caja[3]); //todo idem
		nodo_caja->y = atoi(datos_caja[4]); //todo idem

		//sumo el recurso al string de recursos
		recursos[i-1] = nodo_caja->ID;

		crear_item_caja_desde_nodo(nodo_caja);
		list_add(lista_cajas, (void *) nodo_caja);

		free(datos_caja);
	}
	printf("\n Listado de cajas:\n\n");
	sleep(1);
	for (i=0; i < cantidad_de_recursos; i++)	imprimir_nodo_caja((t_caja *)list_get(lista_cajas, i));
	//linea para controlar que se haya enlistado todito bien

	if(isspace(ip_orquestador[0]))
	{
		char *aux = strdup((ip_orquestador+1));
		free(ip_orquestador);
		ip_orquestador = aux;
	}

	//en este punto, se termino de leer el archivo de config y se enlistaron todos los recursos
	config_destroy(configuracion);
}
