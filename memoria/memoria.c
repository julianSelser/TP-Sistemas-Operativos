#include <stdlib.h>
#include "memoria.h"
#include <string.h>
#include <stdio.h>
#include <commons/string.h>
										
static t_list *mem_manager;					// lista que administra las particiones del segmento de memoria
static int memoria_total;					// el "tamanio" de la memoria que pasa a koopa por parametro
										
t_memoria crear_memoria(int tamanio) {
	t_memoria segmento = malloc(sizeof(char)*tamanio);
	memoria_total = tamanio;
	inicializar_particiones(segmento); 	// prepara mem_manager: creando la lista y la primera particion que tendra toda la memoria
	return segmento;
}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido) {
	t_link_element *nodo_best_fit;
	t_particion *best_fit;

	if( memoria_total<tamanio || es_id_duplicado(id) ) return -1; 	 //comprueba si el pedido de memoria es mayor que el total y si id es repetido
	if( (nodo_best_fit = buscar_best_fit(tamanio) )==NULL) return 0; //retorna 0 si no habia particion util(si nodo==NULL ciclo la lista y no encontro)

	best_fit = nodo_best_fit->data; // la data del NODO best fit es la PARTICION best fit

	//si el fit es exacto: reusa la particion sobrescribiendola
	//sino en "else" se usa la parte de arriba de la particion, dejando el resto libre
	if( best_fit->tamanio == tamanio ) 	sobrescribir_particion(best_fit, segmento, id, tamanio, contenido);
	else {
		dividir_particion(nodo_best_fit, best_fit, segmento, id, tamanio, contenido);
	}
	return 1;
}

int eliminar_particion(t_memoria segmento, char id) {
	t_particion *particion = buscar_particion_por_id(id);	//inicializa la particion con la encontrada por id, si no existe trae NULL
	if(particion == NULL) return 0; 						//si particion es NULL, la particion mandada a eliminar no existia
	else  particion->libre = true;							//si particion NO es NULL: marcar la particion como libre
	return 1;
}

void liberar_memoria(t_memoria segmento) {
	list_destroy_and_destroy_elements(mem_manager,free); //funcion de la commons que libera todo
	free(segmento);
}

t_list* particiones(t_memoria segmento) {
	t_list* particiones = list_create();
	list_add_all(particiones,mem_manager); //funcion de la commons que copia listas
	return particiones;
}

/***********************************************  funciones adicionales  *********************************************/

static t_link_element *buscar_best_fit(int tamanio){//busca best fit en mem_manager, devuelve NULL si no encuentra
	t_link_element *nodo_best_fit;
	t_link_element *aux = mem_manager->head;

	for( nodo_best_fit=NULL ; aux!=NULL ; aux = aux->next){
		// si la particion esta libre y su tamaño es mayor o igual al necesitado es candidata...
		if( ((t_particion*)aux->data)->libre && ((t_particion*)aux->data)->tamanio>=tamanio)
			//si todavia el nodo best fit esta vacio se llena con la candidata O si la candidata es menor que la best fit, pasa a ser la best fit
			if(nodo_best_fit==NULL || ((t_particion*)nodo_best_fit->data)->tamanio > ((t_particion*)aux->data)->tamanio) nodo_best_fit = aux;
	}
	return nodo_best_fit; //devuelve el nodo con la particion adecuada, NULL si NO habia
}

static void inicializar_particiones(t_memoria segmento){//crea la lista que manejara las particiones con la primera particion con toda la memoria
	t_particion *primera_particion = crear_particion(true,0,segmento,' ',memoria_total,string_repeat('0',memoria_total));
	mem_manager = list_create();
	list_add(mem_manager,primera_particion);
}

static t_particion *crear_particion(bool disponibilidad, int inicio, t_memoria segmento, char id, int tamanio, char* contenido){
	//crea una nueva particion y la devuelve
	t_particion *nueva_particion = malloc(sizeof(t_particion));
	nueva_particion->id = id;
	nueva_particion->inicio = inicio;
	nueva_particion->tamanio = tamanio;
	nueva_particion->dato = memmove(segmento+inicio,contenido,tamanio);
	nueva_particion->libre = disponibilidad;
	return nueva_particion;
}

static void sobrescribir_particion(t_particion *particion, t_memoria segmento, char id, int tamanio , char* contenido){
	particion->id = id;
	particion->tamanio = tamanio;
	particion->libre = false;
	particion->dato = memmove(segmento+(particion->inicio),contenido,particion->tamanio);
}

static void dividir_particion(t_link_element *nodo_best_fit, t_particion *best_fit, t_memoria segmento, char id, int tamanio, char* contenido){
	//necesitamos crear una nueva particion libre para darle la primera parte a la particion que se esta creando
	//se calculan los valores de tamaño e inicio para la nueva aprticion libre a crear, y se crea
	int tam_nueva_particion_libre = best_fit->tamanio - tamanio;
	int inicio_nueva_particion_libre = best_fit->inicio+tamanio;
	t_particion *nueva_particion_libre = crear_particion(true,inicio_nueva_particion_libre,segmento,'0',tam_nueva_particion_libre,string_repeat('0',tam_nueva_particion_libre));

	//se sobrescribe la particion que quedo por arriba de la libre y se inserta antes de la nueva libre creada
	sobrescribir_particion(best_fit,segmento,id,tamanio,contenido);
	lista_insertar_entre_nodos(nodo_best_fit, crear_nodo(nueva_particion_libre));
}

static t_particion *buscar_particion_por_id(char id){
	//busca y devuelve una particion por id, devuelve NULL si no encuentra
	t_link_element *aux = mem_manager->head;
	while( aux!=NULL && ((t_particion*)aux->data)->id!=id ) aux = aux->next;
	return aux==NULL? NULL : aux->data;
}

static bool es_id_duplicado(char id){
	//si se encuentra una particion con ese ID: el ID esta dupĺicado
	return buscar_particion_por_id(id)!=NULL? true : false;
}

/***************************************  funciones para manejo de listas  *******************************************/

static t_link_element* crear_nodo(void* data) {
	t_link_element *element = malloc(sizeof(t_link_element));
	element->data = data;
	element->next = NULL;
	return element;
}

static void lista_insertar_entre_nodos(t_link_element* prev,t_link_element* nodo){ //incorpora "nodo" a la lista detras de "prev"
	nodo->next = prev->next;
	prev->next = nodo;
	mem_manager->elements_count++;
}




