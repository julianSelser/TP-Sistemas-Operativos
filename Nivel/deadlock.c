/*
 * deadlock.c
 *
 *  Created on: 16/06/2013
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

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

#include <serial.h>
#include "Nivel.h"

int detectar_deadlock()
{
	while (1)
	{
		usleep(tiempo_chequeo_deadlock*1000);


	}



	return 1;
}


