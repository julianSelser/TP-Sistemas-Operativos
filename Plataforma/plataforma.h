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
		sem_t semaforos[3];//los 3 semaforos que se usan en cada planificador
		t_log *logger_planificador;
		t_list *colas[2];
		int puerto;
	}parametro;

#endif /* PLATAFORMA_H_ */
