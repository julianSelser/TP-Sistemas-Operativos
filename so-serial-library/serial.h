/*
 * serial.h
 *
 *  Created on: Jun 7, 2013
 *      Author: julian
 */

#include <stdint.h>
#include "commons/log.h"

#ifndef SERIAL_H_
#define SERIAL_H_

	#define SOLICITUD_INFO_NIVEL 1						//PP->HO
	#define INFO_NIVEL_Y_PLANIFICADOR 2
	#define NOTIF_MOVIMIENTO_PERMITIDO 3				//HP->PP
	#define SOLICITUD_MOVIMIENTO_XY 4
	#define RTA_SOLICITUD_MOVIMIENTO_XY 5
	#define SOLICITUD_UBICACION_RECURSO 6
	#define SOLICITUD_INSTANCIA_RECURSO 7
	#define RTA_SOLICITUD_INSTANCIA_RECURSO 8
	#define NOTIF_TURNO_CONCLUIDO 9						//PP->HP
	#define NOTIF_NIVEL_CUMPLIDO 10
	#define NOTIF_MUERTE_PERSONAJE 11
	#define INTENCION_REINICIAR_NIVEL 12
	#define NOTIF_RECURSOS_LIBERADOS 13
	#define NOTIF_RECURSOS_REASIGNADOS 14
	#define SOLICITUD_RECUPERO_DEADLOCK 15
	#define NOTIF_ELECCION_VICTIMA 16
	#define NOTIF_PERSONAJE_CONDENADO 17
	#define NOTIF_PLAN_TERMINADO 18
	#define ENVIO_DE_DATOS_AL_PLANIFICADOR 19			//PP->HP

	//typedefs de punteros a funciones
	typedef void *(*p_funcion_deserial)(char *buffer);
	typedef char *(*p_funcion_serial)(void *data);


	/**************************** STRUCTS  ***************************/


	//struct del mensaje que envia el personaje al planificador cuando conecta por primera vez
	typedef struct {
		uint8_t char_personaje;
	} __attribute__((packed)) t_datos_delPersonaje_alPlanificador;

	//struct simbolico del mensaje movimiento permitido(no se usa)
	typedef struct {
		uint8_t permitido;
	} __attribute__((packed)) t_mov_permitido;

	//struct del mensaje turno concluido
	typedef struct{
		uint8_t bloqueado ;// usarlo como booleano
		uint8_t termino_nivel; // usarlo como booleano
		uint8_t recurso_de_bloqueo;
	} __attribute__((packed)) t_turno_concluido;

	typedef struct{
		uint16_t info_nivel;
		uint16_t info_planificador;
	}__attribute__((packed)) t_info_nivel_planificador;

	typedef struct{

		uint8_t condenado ; // lo tratamos como booleano

	} __attribute__((packed)) t_personaje_condenado;

	typedef struct{

		uint8_t x;
		uint8_t y;

	} __attribute__((packed)) t_ubicacion_recurso;

	typedef struct {
		uint8_t solicito_moverme;

	} __attribute__((packed)) t_solicitud_movimiento;

	typedef struct{

		uint8_t resp_solicitud;

	} __attribute__((packed)) t_resp_solicitud_movimiento;

	typedef struct{

		uint8_t recurso;

	} __attribute__((packed)) t_solcitud_instancia_recurso;

	//la cabecera que se lee en todos los mensajes
	typedef struct {
			uint8_t tipo;
			uint16_t len;
	} __attribute__((packed)) t_cabecera;


	/**************************** FUNCIONES  ***************************/


	//funciones serializadoras
	char *srlz_turno_concluido(void *data);
	char *srlz_movimiento_permitido(void* data);
	char *srlz_info_nivel_y_planificador(void *data);
    char *srlz_personaje_condenado(void *data);
    char *srlz_ubicacion_de_recurso(void *data);
    char *srlz_solicitud_de_movimiento(void *data);
    char *srlz_resp_a_solicitud_movimiento(void *data);
    char *srlz_solicitud_de_recurso(void *data);

    //funciones de-serializadoras
	void *deserializar_movimiento_permitido(char *buffer);
	void *deserializar_turno_concluido(char *buffer);
	void *deserializar_datos_delPersonaje_alPlanificador(char *buffer);
	void *deserializar_info_nivel_planificador(char *buffer);
	void *deserializar_personaje_condenado(char *buffer);
    void *deserializar_ubicacion_de_recurso(char *buffer);
    void *deserializar_solicitud_de_movimiento(char *buffer);
    void *deserializar_resp_a_solicitud_movimiento(char *buffer);
    void *deserializar_solicitud_de_recurso(char *buffer);

	//funciones de envio/recepcion
	int getnextmsg(int socket);
	void *recibir(int socket, int tipo);
	void enviar(int socket, int tipo, void *struct_mensaje, t_log *logger);
	t_cabecera *deserializar_cabecera(char *buffer);

	//funciones de socket
	int init_socket_externo(int puerto, char *direccion, t_log *logger);
	int init_socket_escucha(int puerto, int optval, t_log *logger);

	// funciones de inicializacion
	void iniciar_serializadora();
	static void init_vec_deserial();
	static void init_vec_serial();



#endif /* SERIAL_H_ */
