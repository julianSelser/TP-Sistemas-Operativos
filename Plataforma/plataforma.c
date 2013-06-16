/*
 * plataforma.c
 *
 *  Created on: Jun 6, 2013
 *      Author: julian
 */

/*
 * plataforma.c
 *
 *  Created on: 26/04/2013
 *      Author: julian
 */

#include <semaphore.h>
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

#include "rutina_orquestador.h"
#include "rutina_planificador.h"
#include "plataforma.h"

int quantum;
int retraso;

int main()
{
	iniciar_serializadora();

	t_config *plataforma_conf = config_create("conf.arch");
	quantum = config_get_int_value(plataforma_conf, "quantum");
	retraso = config_get_int_value(plataforma_conf, "retraso");

	pthread_t orquestador;

	t_log * logger_plataforma = log_create("plataforma.log", "Plataforma", 1, LOG_LEVEL_TRACE);

	log_debug(logger_plataforma, "Se lanza el hilo orquestador!", "DEBUG");
	pthread_create(&orquestador, NULL,(void*)rutina_orquestador, NULL);

	pthread_join(orquestador, NULL);

	log_destroy(logger_plataforma);

	return 0;
}
