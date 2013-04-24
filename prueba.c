/*
 * prueba.c
 *
 *  Created on: 19/04/2013
 *      Author: Kernighan
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

int main(){
    char c,*nombre;
    int m = 0;
    puts("Hola grupo!!!\n");
    printf("Cual es tu nombre?\n");
    for( nombre = malloc(sizeof(char)) ; (*(nombre+ (m++) ) = getchar()) !='\n' ; nombre = realloc( nombre,m*sizeof(char)) );
    *(nombre+(--m))='\0';
    printf("\nTu nombre es %s\n\n",nombre);
    free(nombre);
    return 0;
}
