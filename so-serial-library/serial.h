/*
 * serial.h
 *
 *  Created on: Jun 7, 2013
 *      Author: julian
 */

#include <stdint.h>
#include <commons/log.h>

#ifndef SERIAL_H_
#define SERIAL_H_

	#define N_MENSAJES 20

	#define SOLICITUD_INFO_NIVEL 1						//PP->HO
	#define INFO_NIVEL_Y_PLANIFICADOR 2					//HO->PP
	#define NOTIF_MOVIMIENTO_PERMITIDO 3				//HP->PP
	#define SOLICITUD_MOVIMIENTO_XY 4					//PP->PN
	#define RTA_SOLICITUD_MOVIMIENTO_XY 5				//PN->PP
	#define SOLICITUD_UBICACION_RECURSO 6				//PP->PN
	#define SOLICITUD_INSTANCIA_RECURSO 7				//PP->PN
	#define RTA_SOLICITUD_INSTANCIA_RECURSO 8			//PN->PP
	#define NOTIF_TURNO_CONCLUIDO 9						//PP->HP
	#define NOTIF_NIVEL_CUMPLIDO 10						//PP->PN
	#define NOTIF_MUERTE_PERSONAJE 11					//HO->PN
	#define INTENCION_REINICIAR_NIVEL 12				//PP->HO
	#define NOTIF_RECURSOS_LIBERADOS 13					//PN->HO
	#define NOTIF_RECURSOS_REASIGNADOS 14				//HO->PN
	#define SOLICITUD_RECUPERO_DEADLOCK 15				//PN->HO
	#define NOTIF_ELECCION_VICTIMA 16					//HO->PN
	#define NOTIF_PERSONAJE_CONDENADO 17				//HO->PP
	#define NOTIF_PLAN_TERMINADO 18						//PP->HO
	#define ENVIO_DE_DATOS_AL_PLANIFICADOR 19			//PP->HP
	#define INFO_UBICACION_RECURSO 20					//PN->PP
	#define ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL 21		//PP->PN

	//typedefs de punteros a funciones
	typedef void *(*p_funcion_deserial)(char *buffer);
	typedef char *(*p_funcion_serial)(void *data, int *tamanio);

	//extern p_funcion_deserial vec_deserializador[N_MENSAJES];
	//extern p_funcion_serial vec_serializador[N_MENSAJES];


	/**************************** STRUCTS  ***************************/

	//1 - struct del mensaje que le envia el personaje al orquestador para recibir como respuesta la informacion que necesita (ips/puertos)
    typedef struct {
    	uint8_t * nivel_solicitado;
    }__attribute__((packed)) t_solicitud_info_nivel;


    //2 - respuesta del planificador al personaje con las ips/puertos pedidos
	typedef struct{
		uint16_t puerto_nivel;
		uint16_t puerto_planificador;
		uint8_t *ip_nivel;
		uint8_t *ip_planificador;
	}__attribute__((packed)) t_info_nivel_planificador ;


	//3 - struct simbolico del mensaje movimiento permitido(no se usa)/*	FIJARSE DESPUES EN PERSONAJE SI HAY UN RECIBIR DESPUES DEL GETNEXTMSG	*/
	typedef struct {
		uint8_t permitido;
	} __attribute__((packed)) t_mov_permitido ;


	//4 - struct que simboliza una solicitud de movimiento del personaje al nivel
	typedef struct {
		uint8_t char_personaje;
		uint8_t x;
		uint8_t y;
	} __attribute__((packed)) t_solicitud_movimiento;


	//5 - struct del mensaje que envia el nivel al personaje, aprobando o no una solicitud de movimiento
	typedef struct{
		uint8_t aprobado;
	} __attribute__((packed)) t_resp_solicitud_movimiento;


	//6 - struct del mensaje que envia el personaje al nivel pidiendo la ubicacion de un recurso, "recurso" es el char de aquel
	typedef struct{
		uint8_t recurso;
	} __attribute__((packed)) t_solicitud_ubicacion_recurso;


	//7 - struct del mensaje que envia el personaje al nivel pidiendo una instancia de un recurso
	typedef struct{
		uint8_t instancia_recurso;
	} __attribute__((packed)) t_solcitud_instancia_recurso;


	//8 - struct del mensaje que envia el nivel al personaje en respuesta al pedido de instancia de un recurso, concediendolo o no
	typedef struct{
		uint8_t concedido;
	} __attribute__((packed)) t_rspta_solicitud_instancia_recurso;


	//9 - struct del mensaje turno concluido que el personaje envia al hilo planificador en cada dada de quantum
	typedef struct{
		uint8_t bloqueado ;// usarlo como booleano
		uint8_t recurso_de_bloqueo;
	} __attribute__((packed)) t_turno_concluido;


	//10 - struct del mensaje que envia el personaje al nivel cuando termino de reunir sus objetivos
	typedef struct{
		uint8_t char_personaje;
	} __attribute__((packed)) t_notificacion_nivel_cumplido;


	//11 - ESTE LE LLEGA AL NIVEL DE PARTE DEL ORQUESTADOR Y ESTA MAL
	typedef struct{
		uint8_t mori;
	}__attribute__((packed)) t_notificacion_muerte_personaje;


	/*12 - FALTA 	*/

	/*13 - FALTA	*/

	/*14 - FALTA	*/

	/*15 - FALTA	*/

	/*16 - FALTA	*/

	//17 - struct del mensaje que envia el orquestador al personaje cuando debe morir a causa de interbloqueo
	typedef struct{
		uint8_t condenado ; // lo tratamos como booleano
		// creo que también es simbólico porque ni bien recibe esto, el personaje va y se muere
	} __attribute__((packed)) t_personaje_condenado;


	//18 - struct del mensaje que envia el personaje al orquestador cuando termino su plan de nivel...ojo con este...
	typedef struct{
		uint8_t * personaje;
	} __attribute__((packed)) t_notificacion_plan_terminado;


	//19 - struct del mensaje que envia el personaje al planificador cuando conecta por primera vez
	typedef struct {
		uint8_t char_personaje;
		uint8_t *nombre_personaje;
	} __attribute__((packed)) t_datos_delPersonaje_alPlanificador;


	//20 - struct del mensaje que envia el nivel al personaje para que sepa donde esta ubicado el recurso
	typedef struct{
		uint8_t x;
		uint8_t y;
	} __attribute__((packed)) t_ubicacion_recurso ;
	

	//21 - mensaje que un personaje envia al nivel cada vez que se quiere conectar por primera vez
	typedef struct{
		uint8_t char_personaje;
		uint8_t *nombre_personaje;
		uint8_t *necesidades;
	} __attribute__((packed)) t_datos_delPersonaje_alNivel;


	//la cabecera que se lee en todos los mensajes
	typedef struct {
			uint8_t tipo;
			uint16_t len;
	} __attribute__((packed)) t_cabecera;


	/**************************** FUNCIONES  ***************************/


	//funciones serializadoras
	char *srlz_turno_concluido(void *data, int *tamanio);
	char *srlz_movimiento_permitido(void* data, int *tamanio);
	char *srlz_info_nivel_y_planificador(void *data, int *tamanio);
    char *srlz_personaje_condenado(void *data, int *tamanio);
    char *srlz_ubicacion_de_recurso(void *data, int *tamanio);
    char *srlz_solicitud_de_movimiento(void *data, int *tamanio);
    char *srlz_resp_a_solicitud_movimiento(void *data, int *tamanio);
    char *srlz_datos_delPersonaje_alPlanificador(void *data, int *tamanio);
    char *srlz_solicitud_de_recurso(void *data, int *tamanio);
    char *srlz_rta_solicitud_de_recurso(void *data, int *tamanio);


	char *srlz_solicitud_info_nivel(void *data, int *tamanio);
    char *srlz_solicitud_instancia_recurso(void *data,int *tamanio);
    char *srlz_rspta_solicitud_instancia_recurso(void *data,int *tamanio);
    char *srlz_notificacion_nivel_cumplido(void *data,int *tamanio);
    char *srlz_notificacion_plan_terminado(void *data,int *tamanio);
    char *srlz_notificacion_muerte_personaje(void *data,int *tamanio);

    //funciones de-serializadoras
	void *deserializar_movimiento_permitido(char *buffer);
	void *deserializar_turno_concluido(char *buffer);
	void *deserializar_info_nivel_y_planificador(char *buffer);
	void *deserializar_personaje_condenado(char *buffer);
    void *deserializar_ubicacion_de_recurso(char *buffer);
    void *deserializar_solicitud_de_movimiento(char *buffer);
    void *deserializar_resp_a_solicitud_movimiento(char *buffer);
	void *deserializar_datos_delPersonaje_alPlanificador(char *buffer);
    void *deserializar_solicitud_de_recurso(char *buffer);
    void *deserializar_resp_solicitud_de_recurso(char *buffer);
    
    void *deserializar_solicitud_info_nivel(char *buffer);
	void *deserializar_info_nivel_y_planificador(char *buffer);
    void *deserializar_solicitud_instancia_recurso(char *buffer);
    void *deserializar_rspta_solicitud_instancia_recurso(char *buffer);
    void *deserializar_notificacion_nivel_cumplido(char *buffer);
    void *deserializar_notificacion_plan_terminado(char *buffer);
    void *deserializar_notificacion_muerte_personaje(char *buffer);

	//funciones de envio/recepcion
	int getnextmsg(int socket);
	void *recibir(int socket, int tipo);
	int enviar(int socket, int tipo, void *struct_mensaje, t_log *logger);
	t_cabecera *deserializar_cabecera(char *buffer);

	//funciones de socket
	int init_socket_externo(int puerto, char *direccion, t_log *logger);
	int init_socket_escucha(int puerto, int optval, t_log *logger);
	int is_connected(int socket);

	// funciones de inicializacion
	void iniciar_serializadora();
	static void init_vec_deserial();
	static void init_vec_serial();



#endif /* SERIAL_H_ */
