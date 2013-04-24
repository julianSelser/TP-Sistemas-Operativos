/*
 * prueba.c
 *
 *  Created on: 19/04/2013
 *      Author: Kernighan
 *
 */

#include <stdio.h>
#include <malloc.h>

void ingresarNombre(char*);

int main(){
    char c,*nombre;
    int m = 0;

    puts("Hola grupo!!!\n");
    printf("Cual es tu nombre?\n");

   	while( (c = getchar()) !='\n'){
		if( (nombre = (m == 0)? malloc(sizeof(char)) : realloc( nombre,m*sizeof(char) )) == NULL) return 1;
		*(nombre + m++) = c;		
	}

    printf("\nTu nombre es %s\n\n",nombre);
    free(nombre);
    return 0;
}
