#include <stdlib.h>
#include "memoria.h"

t_memoria crear_memoria(int tamanio) {
	return NULL;
}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido) {
	return -1;
}

int eliminar_particion(t_memoria segmento, char id) {
	return 0;
}

void liberar_memoria(t_memoria segmento) {
	
}

t_list* particiones(t_memoria segmento) {
	t_list* list = list_create();
	return list;
}
