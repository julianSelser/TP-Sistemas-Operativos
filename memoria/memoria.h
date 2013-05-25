#ifndef LIBMEMORIA_H_
#define LIBMEMORIA_H_
    #include "commons/collections/list.h"


    typedef char* t_memoria;

    typedef struct {
        char id;
        int inicio;
        int tamanio;
        char* dato;
        bool libre;
    } t_particion;

    t_memoria crear_memoria(int tamanio);
    int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido);
    int eliminar_particion(t_memoria segmento, char id);
    void liberar_memoria(t_memoria segmento);
    t_list* particiones(t_memoria segmento);


/***************************************  Funciones adicionales  ****************************************/


    static void sobrescribir_particion(t_particion *particion, t_memoria segmento, char id, int tamanio , char* contenido);
    static void dividir_particion(t_link_element *nodo_best_fit, t_particion *best_fit, t_memoria segmento, char id, int tamanio, char* contenido);
    static void inicializar_particiones(t_memoria segmento);
    static t_link_element *buscar_best_fit(int tamanio);
    static t_particion *crear_particion(bool disponibilidad, int inicio, t_memoria segmento, char id, int tamanio, char* contenido);
    static t_particion *buscar_particion_por_id(char id);
    static bool es_id_duplicado(char id);

/***************************************  Funciones de listas  ****************************************/

    static void lista_insertar_entre_nodos(t_link_element* prev,t_link_element* nodo);
    static t_link_element* crear_nodo(void* data);


#endif /* LIBMEMORIA_H_ */


