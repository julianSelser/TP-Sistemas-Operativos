/*
 * rutina_orquestador.c
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */

#define _GNU_SOURCE

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
#include <signal.h>

#include "rutina_orquestador.h"
#include "rutina_planificador.h"
#include "plataforma.h"

#include <serial.h>


#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

t_list * lista_niveles;
t_log * logger_orquestador;
char * jugadores;
char * jugadores_que_terminaron;

static int puerto_planif = 23000;


void rutina_orquestador(/*?*/)
{
	logger_orquestador = log_create("orquestador.log", "ORQUESTADOR", 1, LOG_LEVEL_TRACE);
	int inotify_fd = inotify_init();
	int socketEscucha = init_socket_escucha(10000, 1, logger_orquestador);
	int i, nuevo_fd, fdmax = socketEscucha>inotify_fd?socketEscucha:inotify_fd;
	fd_set maestro, read_fds;

	//hacemos que el fd de inotify  escuche por el evento de modificacion
	inotify_add_watch(inotify_fd, (char*)get_current_dir_name(), IN_MODIFY);

	//inicializamos las variables globales
	jugadores = strdup("");
	jugadores_que_terminaron = strdup(""); //es lo mismo que jugadores=malloc(1);jugadores[0]='\0';
	lista_niveles=list_create();
	log_info(logger_orquestador, "El orquestador esta comienza a esperar niveles", "INFO");

	FD_SETEO;//macro que setea los fd para select()

	while(1)
	{
		read_fds = maestro;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error(logger_orquestador,"Error de select!","ERRROR");
			exit(1);
		}

		// loopea los fd's
		for(i = 0 ; i <= fdmax ; i++)
		{
			if (FD_ISSET(i, &read_fds)) // buscar los seteados
			{
				if (i == socketEscucha)
				{
					nuevo_fd = accept(socketEscucha,NULL,0); // si es el escucha se tiene un nuevofd

					FD_SET(nuevo_fd, &maestro);

					if (nuevo_fd > fdmax) // chequear si el nuevo fd es mas grande que el maximo
					{
						fdmax = nuevo_fd;
					}
				}

				if(i == inotify_fd) // si es el fd de inotify correr la rutina de inotify
				{
					rutina_inotify(inotify_fd);
				}

				if(i!=inotify_fd && i!=socketEscucha) // si no son los socket especiales es un mensaje...
				{
					if(is_connected(i))  	//si el socket sigue estando conectado
					{
						manejar_peticion(i);
					}
					else // si se desconecto, se nos escapo un nivel o un personaje
					{
						char index;
						t_nodo_nivel *nodo = ubicar_nivel_por_socket(i,&index)!=NULL? list_remove(lista_niveles, index) : NULL;
						if(nodo!=NULL)
						{
							log_info(logger_orquestador, string_from_format("Cerrando el planificador del nivel: %s", nodo->nombre), "INFO");

							pthread_kill(nodo->hilo_planificador, SIGTERM);

							free(nodo->nombre);
							free(nodo->IP);
							free(nodo);
						}
						close(i);
						FD_CLR(i, &maestro);
					}
				}
			} // fin actividad en socket
		} // fin for
	}
}


void manejar_peticion(int socket){
	switch(getnextmsg(socket))
	{

	case SOLICITUD_RECUPERO_DEADLOCK:
										manejar_sol_recovery(socket);
										break;
	case NOTIF_PLAN_TERMINADO:
										manejar_plan_terminado(socket);
										break;
	case SOLICITUD_INFO_NIVEL:
										manejar_sol_info(socket);
										break;
	case NOTIF_RECURSOS_LIBERADOS:
										manejar_recs_liberados(socket);
										break;
	case ENVIO_DE_DATOS_NIVEL_AL_ORQUESTADOR:
										manejar_anuncio_nivel(socket);
										break;
	default:
			log_error(logger_orquestador,"Mensaje inexistente!","ERROR");
			break;
	}
}

void manejar_anuncio_nivel(int socket_nivel)
{
	t_link_element *aux;
	t_envio_deDatos_delNivel_alOrquestador * datos_nivel_entrante = recibir(socket_nivel, ENVIO_DE_DATOS_NIVEL_AL_ORQUESTADOR);

	for(aux=lista_niveles->head; aux!=NULL && strcmp(datos_nivel_entrante->nombre,((t_nodo_nivel*)aux->data)->nombre)!=0 ; aux=aux->next);

	if(aux==NULL)
	{
		int i=0;
		t_log * logger_planif;
		t_nodo_nivel * nuevo_nivel = malloc(sizeof (t_nodo_nivel));

		log_info(logger_orquestador, string_from_format("Se conecto el nivel %s, creando su planificador...", datos_nivel_entrante->nombre), "INFO");

		nuevo_nivel->socket = socket_nivel;
		nuevo_nivel->IP = get_ip_string(socket_nivel);
		nuevo_nivel->colas[LISTOS] = list_create();
		nuevo_nivel->colas[BLOQUEADOS] = list_create();
		nuevo_nivel->nombre = datos_nivel_entrante->nombre;
		nuevo_nivel->puerto = datos_nivel_entrante->puerto_nivel;
		nuevo_nivel->puerto_planif = puerto_planif;

		//aca armo el logger que va a usar el planificador
		logger_planif = log_create(string_from_format("planif_%s.log", nuevo_nivel->nombre), string_from_format("PLANIF_%s", nuevo_nivel->nombre), 1, LOG_LEVEL_TRACE);
		//creado el logger, pelada la gallina (ajajaja)

		while(datos_nivel_entrante->recursos_nivel[i]!='\0') //recorrer los recursos que presenta el nivel
		{
			t_nodo_bloq_por_recurso * info_recurso = malloc(sizeof(t_nodo_bloq_por_recurso));
			info_recurso->char_recurso=datos_nivel_entrante->recursos_nivel[i];
			info_recurso->personajes=list_create();
			list_add(nuevo_nivel->colas[BLOQUEADOS], info_recurso); //crear cola de bloqueados para el recurso actual
			i++;
		}


		list_add(lista_niveles, nuevo_nivel);

		pthread_create(&nuevo_nivel->hilo_planificador, NULL, (void*)rutina_planificador, armar_parametro(nuevo_nivel, logger_planif));
		puerto_planif++;
	}
	else{
		log_error(logger_orquestador, string_from_format("El nivel: %s trato de entrar, pero ya existía una instancia de este en el sistema", datos_nivel_entrante->nombre), "ERROR");
		shutdown(socket_nivel, 2);
	}
	free(datos_nivel_entrante->recursos_nivel);
	free(datos_nivel_entrante);
}

parametro *armar_parametro(t_nodo_nivel * nuevo_nivel, t_log * logger)
{
	parametro *p = malloc(sizeof(parametro));

	//ya arme las colas desde antes
	p->colas[LISTOS]=nuevo_nivel->colas[LISTOS];
	p->colas[BLOQUEADOS]=nuevo_nivel->colas[BLOQUEADOS];
	p->logger_planificador = NULL;
	p->puerto = puerto_planif;
	p->logger_planificador = logger;

	//ahora que arme la estructura "parametro", me guardo la direccion a los semaforos
	nuevo_nivel->sem_listos = &(p->semaforos[0]);
	nuevo_nivel->sem_vacia = &(p->semaforos[1]);
	nuevo_nivel->sem_bloqueados = &(p->semaforos[2]);

	return p;
}

void manejar_sol_info(int socket)
{
	t_info_nivel_planificador * info;
	t_solicitud_info_nivel * solicitud = recibir(socket, SOLICITUD_INFO_NIVEL);

	log_info(logger_orquestador, string_from_format("El personaje %c quiere saber donde esta el nivel %s", solicitud->solicitor ,solicitud->nivel_solicitado), "INFO");

	info = crear_info_nivel(solicitud->nivel_solicitado);

	if (strcmp(info->ip_nivel,"NIVEL NO ENCONTRADO") && agregar_sin_repetidos(&jugadores, solicitud->solicitor))
	{
		log_info(logger_orquestador, string_from_format("%c entra al juego!", solicitud->solicitor), "INFO");
	}

	enviar(socket, INFO_NIVEL_Y_PLANIFICADOR, info, logger_orquestador);
	log_info(logger_orquestador, "Se respondio con la informacion", "INFO");

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
		if(strcmp(nivel_actual->nombre, nombre)==0)
		{
			temp->ip_nivel=strdup(nivel_actual->IP);
			temp->puerto_nivel=nivel_actual->puerto;
			temp->puerto_planificador=nivel_actual->puerto_planif;
			return temp;
		}
		i++;
	}

	log_info(logger_orquestador, string_from_format("Un personaje pidio por el nivel: %s  que NO EXISTIA", nombre), "INFO");
	temp->puerto_nivel=0;
	temp->puerto_planificador=0;
	temp->ip_nivel = strdup("NIVEL NO ENCONTRADO");

	return temp; //si sale por aca, no se encontro el nivel
}


void manejar_recs_liberados(int socket)
{
	char * liberados;
	char * resto;
	char * reasignaciones;
	char rec;
	int rec_ant=300;// :|
	t_nodo_nivel * nivel;
	t_notif_recursos_liberados * notificacion;
	t_notif_recursos_reasignados * informe;
	int i = 0;

	nivel = ubicar_nivel_por_socket(socket,&rec);

	resto=strdup("");

	reasignaciones=strdup("");

	notificacion = recibir(socket, NOTIF_RECURSOS_LIBERADOS);
	log_info(logger_orquestador, string_from_format("El nivel %s libero los siguientes recursos: %s", nivel->nombre, notificacion->recursos_liberados), "INFO");
	liberados=notificacion->recursos_liberados;
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

			sem_wait(nivel->sem_bloqueados);
			personaje = desencolar(nodo_cola->personajes, NULL, NULL);
			sem_post(nivel->sem_bloqueados);

			if(enviar(personaje->socket, NOTIF_RECURSO_CONCEDIDO, concedido, logger_orquestador) < 0)
			{ //si enviar<0, significa que el personaje no esta (se desconecto por x razon)
				free(personaje->nombre);
				free(personaje); //elimino al nodo del personaje
				continue; //quise darle algo a un personaje ausente, vuelvo a empezar para este recurso
			}

			else //el enviar fue exitoso: el personaje estaba
			{
				informe_parcial[0]=personaje->char_personaje;
				informe_parcial[1]=rec;
				informe_parcial[2]='\0';
				string_append(&reasignaciones, informe_parcial); //agrego la reasignacion al mensaje que va a ir al nivel
				sem_wait(nivel->sem_listos);
				encolar(nivel->colas[LISTOS], personaje, NULL, NULL); //paso el personaje a listos.
				sem_post(nivel->sem_listos);
				sem_post(nivel->sem_vacia);
				log_info(logger_orquestador, string_from_format("Se libero a %s, que estaba esperando un recurso %c", personaje->nombre, rec), "INFO");
			}
		}

		rec_ant=rec;
		i++;
	} //fin de procesamiento del string de recursos liberados
	free(liberados);

	informe=malloc(sizeof(t_notif_recursos_reasignados));
	informe->asignaciones=reasignaciones;
	informe->remanentes= resto;
	log_info(logger_orquestador, string_from_format("Sobraron los siguientes recursos: %s", resto), "INFO");

	enviar(nivel->socket, NOTIF_RECURSOS_REASIGNADOS, informe, logger_orquestador);
}

void manejar_sol_recovery(int socket)
{
	char ID_victima, ID_victima_excepcional;
	t_nodo_personaje * victima;
	t_nodo_nivel * nivel;

	t_solicitud_recupero_deadlock * solicitud;
	t_notif_eleccion_de_victima * respuesta;
	t_personaje_condenado * condena=malloc(sizeof(t_personaje_condenado));

	nivel=ubicar_nivel_por_socket(socket, NULL);

	solicitud=recibir(socket, SOLICITUD_RECUPERO_DEADLOCK);
	log_info(logger_orquestador, string_from_format("Hay un interbloqueo en el nivel %s, estan involucrados los personajes %s", nivel->nombre, solicitud->pjes_deadlock));

	ID_victima=decidir(solicitud->pjes_deadlock);
	ID_victima_excepcional=decidir(solicitud->pjes_deadlock+1);//mecanismo en caso de no encontrar la primera victima

	free(solicitud->pjes_deadlock);
	free(solicitud);

	sem_wait(nivel->sem_bloqueados);
	if( NULL == (victima=extraer(ID_victima, nivel->colas[BLOQUEADOS], 3)) )	victima=extraer(ID_victima_excepcional, nivel->colas[BLOQUEADOS], 2);
	sem_post(nivel->sem_bloqueados);

	if(victima != NULL){
		log_info(logger_orquestador, string_from_format("Se mato a %s para solucionar el interbloqueo", victima->nombre), "INFO");

		respuesta=malloc(sizeof(t_notif_eleccion_de_victima));
		respuesta->char_personaje=ID_victima;

		enviar(socket, NOTIF_ELECCION_VICTIMA, respuesta, logger_orquestador);

		condena->condenado = true;

		enviar(victima->socket, NOTIF_PERSONAJE_CONDENADO, condena, logger_orquestador);
		close(victima->socket);
		free(victima->nombre);
		free(victima);
	}
}

char decidir(char * involucrados)
{
	return involucrados[0];
}

t_nodo_personaje * extraer(char ID, t_list * lista_colas, int intentos)
{
	t_nodo_bloq_por_recurso * cola_actual;
	int j=0;
	int cant_colas;

	cant_colas = list_size(lista_colas);

	while(j<cant_colas)
	{
		t_nodo_personaje * pje_actual;
		int i=0;
		int cant_pjes;

		cola_actual=(t_nodo_bloq_por_recurso *)list_get(lista_colas, j);
		cant_pjes = list_size(cola_actual->personajes);

		while(i<cant_pjes)
		{
			pje_actual=(t_nodo_personaje *)list_get(cola_actual->personajes, i);
			if(pje_actual->char_personaje==ID)
			{
				list_remove(cola_actual->personajes, i);
				return pje_actual;
			}
			i++;
		}
		j++;
	}

	//si no lo encontramos, esperamos 0,33 segundos y volvemos a intentar
	usleep(333333);
	return intentos>0? extraer(ID, lista_colas, intentos-1) : NULL;
}

void manejar_plan_terminado(int socket)
{
	t_notificacion_plan_terminado * notificacion;

	notificacion = recibir(socket, NOTIF_PLAN_TERMINADO);

	log_info(logger_orquestador, string_from_format("%s termino su plan de niveles!", notificacion->personaje), "INFO");

	agregar_sin_repetidos(&jugadores_que_terminaron, notificacion->char_id);

	if (strlen(jugadores_que_terminaron) == strlen(jugadores))
	{
		log_info(logger_orquestador, "Todos los personajes terminaron su plan de niveles! Terminando el hilo orquestador...", "INFO");
		pthread_exit(NULL);
	}
}


void rutina_inotify(int inotify_fd)
{
	int offset = 0;
	char buffer[BUF_LEN];
	int length = read(inotify_fd, buffer, BUF_LEN);usleep(100000);
	if (length < 0) {
		log_error(logger_orquestador, "Error leyendo en inotify", "ERROR");
	}

	while (offset < length)
	{
		struct inotify_event *event = (struct inotify_event *) &buffer[offset];

		if (event->len)
		{
			if (event->mask & IN_MODIFY)
			{
				if(!strcmp(event->name,config_name))
				{
					t_config *plataforma_conf = config_create(config_name);
					quantum = strtod(config_get_string_value(plataforma_conf, "quantum"), NULL);
					retraso = strtod(config_get_string_value(plataforma_conf, "retraso"), NULL);
					log_info(logger_orquestador, string_from_format("\n modificacion en el archivo de configuracion...\n quantum: %.0f\n retraso: %.2f\n\n",quantum,retraso), "INFO");
					config_destroy(plataforma_conf);
				}
			}
		}
		offset += sizeof (struct inotify_event) + event->len;
	}
}

t_nodo_nivel * ubicar_nivel_por_socket(int socket, char *index)
{
	int cant_niveles;
	int i=0;

	cant_niveles=list_size(lista_niveles);

	while(i<cant_niveles)
	{
		t_nodo_nivel * nv_actual;
		if(index!=NULL) *index = i;
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

int agregar_sin_repetidos(char **string, char c)
{
	int i=0;
	int len = strlen(*string);
	char * aux;

	while(i<len)
	{
		if ((*string)[i] == c) return false;
		++i;
	}

	aux = malloc(2);
	aux[0]=c;
	aux[1]='\0';

	string_append(string, aux);
	free(aux);

	return true;
}

