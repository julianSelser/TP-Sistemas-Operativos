/*
 * procesoPrincipal.c
 *
 *  Created on: 26/04/2013
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <log.h>
#include <unistd.h>

#define BUFFER_SIZE 20

pthread_mutex_t habilitar_productor = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t habilitar_consumidor = PTHREAD_MUTEX_INITIALIZER;

void * rutina_productor();
void * rutina_consumidor();

int8_t shared_var = 0;
char shared_buffer[BUFFER_SIZE];


int main()
{
	pthread_t Productor, Consumidor;
	char n_productor[20] = "Thread_Productor";
	char n_consumidor[20] = "Thread_Consumidor";
	int r_prod, r_cons;
	t_log * logger_plataforma;

	logger_plataforma = log_create("plataforma.log", "Plataforma", 1, LOG_LEVEL_TRACE);



	pthread_mutex_lock(&habilitar_consumidor); //el consumidor empieza bloqueado

	r_prod = pthread_create(&Productor, NULL, rutina_productor, (void *) n_productor);
	log_debug(logger_plataforma, "Se lanza el hilo productor!", "DEBUG");

	r_cons = pthread_create(&Consumidor, NULL, rutina_consumidor, (void *) n_consumidor);
	log_debug(logger_plataforma, "Se lanza el hilo consumidor!", "DEBUG");
	//el hilo principal debería lanzar los otros dos hilos

	pthread_join(Productor, NULL);
	pthread_join(Consumidor, NULL);

	log_destroy(logger_plataforma);

	return 0;
}


void * rutina_productor()
{

	while (1)
	{
		pthread_mutex_lock(&habilitar_productor);
		puts("ingrese el texto: ");
		fgets(shared_buffer, BUFFER_SIZE, stdin);
		puts("\n");
		pthread_mutex_unlock(&habilitar_consumidor);
		sleep(1);
	}

	return 0;

}

void * rutina_consumidor()
{
	sleep(1);
	while(1)
	{
		pthread_mutex_lock(&habilitar_consumidor);
		printf("Se recibió el texto: %s", shared_buffer);
		pthread_mutex_unlock(&habilitar_productor);
		sleep(1);
	}
	return 0;
}
