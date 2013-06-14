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

	#define NOTIF_MOVIMIENTO_PERMITIDO 3				//HP->PP
	#define NOTIF_TURNO_CONCLUIDO 9						//PP->HP
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
		uint8_t termino_nivel; ;// usarlo como booleano
		uint8_t recurso_de_bloqueo;
	} __attribute__((packed)) t_turno_concluido;

	//la cabecera que se lee en todos los mensajes
	typedef struct {
			uint8_t tipo;
			uint16_t len;
	} __attribute__((packed)) t_cabecera;


	/**************************** FUNCIONES  ***************************/


	//funciones serializadoras
	char *srlz_turno_concluido(void *data);
	char *srlz_movimiento_permitido(void* data);

	//funciones de-serializadoras
	void *deserializar_movimiento_permitido(char *buffer);
	void *deserializar_turno_concluido(char *buffer);
	void *deserializar_datos_delPersonaje_alPlanificador(char *buffer);

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
