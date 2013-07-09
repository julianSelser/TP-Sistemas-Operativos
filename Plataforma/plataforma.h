/*
 * plataforma.h
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */


#ifndef PLATAFORMA_H_
#define PLATAFORMA_H_

	//defines de los indices para las colas
	#define LISTOS 0
	#define BLOQUEADOS 1

	extern int quantum;
	extern int retraso;

	//la info que le llega por aprametro al planificador
	typedef struct info_planificador{
		sem_t semaforos[3]; //se usan 3 semaforos en cada planificador
		t_log *logger_planificador;
		t_list *colas[2];
		int puerto;
	}parametro;
	

	//estructura de cada nodo de la lista de personajes
	//contiene el socket de c/proceso personaje y su caracter identificador
	typedef struct{
	int socket;
	char char_personaje;
	char *nombre;
	}t_nodo_personaje;
	
	//estructura de cada nodo de la lista de bloqueados por recuros
	//tiene un caracter identificador del recurso y una lista de personajes
	typedef struct{
	char char_recurso;
	t_list * personajes;
	}t_nodo_bloq_por_recurso;

	//funciones
	t_list *buscar_lista_de_recurso(t_list *bloqueados, char recurso_de_bloqueo);


#endif /* PLATAFORMA_H_ */
