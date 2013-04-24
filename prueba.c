/*
 * prueba.c
 *
 *  Created on: 19/04/2013
 *      Author: utnso
 */

#include <stdio.h>
#include <malloc.h>

int main()
{
	char * nombre;

	if(!(nombre=(char *)malloc(20))) return 1;
	puts("Hola grupo!!!\n");
    printf("hola gente, como estan?\n");
    printf("Cual es tu nombre?\n");
    scanf("%s",nombre);
    printf("Tu nombre es %s",nombre);
    free(nombre);
    return 0;
}
