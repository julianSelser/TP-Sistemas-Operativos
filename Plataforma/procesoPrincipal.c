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

pthread_mutex_t semaforo;

void * rutina_productor();
void * rutina_consumidor();

int8_t shared_var;



int main()
{
	pthread_t Productor, Consumidor;
	char n_productor[20] = "Thread_Productor";
	char n_consumidor[20] = "Thread_Consumidor";
	int16_t r_prod, r_cons;

	r_prod = pthread_create(&Productor, NULL, rutina_productor, (void *) n_productor);
	r_cons = pthread_create(&Consumidor, NULL, rutina_consumidor, (void *) n_consumidor);
	//el hilo principal debería lanzar los otros dos hilos
	while(1);
	return 0;
}

// por ahora pruebo que se lancen bien los hilos...

void * rutina_productor()
{
	while(1) puts("Hola\n");
	return 0;
}

void * rutina_consumidor()
{
	while(1) puts("Que tal\n");
	return 0;
}
