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
extern char **environ;

int main() //todo tomar archivo de config por argv? no habria que pasarle tambien el archivo de solicitudes para koopa?
{
	char *argv[] = { "koopa", "solicitudes.lst" , NULL };
	pthread_t orquestador;
	t_log * logger_plataforma = log_create("plataforma.log", "Plataforma", 1, LOG_LEVEL_TRACE);
	t_config *plataforma_conf = config_create("arch.conf");

	quantum = config_get_int_value(plataforma_conf, "quantum");
	retraso = config_get_int_value(plataforma_conf, "retraso");
	config_destroy(plataforma_conf);

	iniciar_serializadora();

	log_debug(logger_plataforma, "Se lanza el hilo orquestador!", "DEBUG"); // todo estamos logeando solo esto...es medio al pedo el loger de Plataforma

	pthread_create(&orquestador, NULL,(void*)rutina_orquestador, NULL);
	pthread_join(orquestador, NULL);

	log_destroy(logger_plataforma);

	execve("koopa", argv, environ);

	return EXIT_SUCCESS;
}


t_list *buscar_lista_de_recurso(t_list *bloqueados, char recurso_de_bloqueo)
{
	t_link_element *element = bloqueados->head;
	t_nodo_bloq_por_recurso *nodo = element->data; //nunca van a ser NULL, el orquestador siempre crea esta lista
	
	while ( element != NULL && !(nodo->char_recurso == recurso_de_bloqueo)) {
	element = element->next;
	nodo = element->data;
	}
	return nodo->personajes;
}
