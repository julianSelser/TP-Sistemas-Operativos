/*
 * serial.h
 *
 *  Created on: Jun 7, 2013
 *      Author: julian
 */

#ifndef SERIAL_H_
#define SERIAL_H_

	#define NOTIF_MOVIMIENTO_PERMITIDO 3
	#define NOTIF_TURNO_CONCLUIDO 9

	//typedefs de punteros a funciones
	typedef void *(*p_funcion_deserial)(char *buffer);
	typedef char *(*p_funcion_serial)(void *data);


	/**************************** STRUCTS  ***************************/

	//struct simbolico del mensaje movimiento permitido(no se usa)
	typedef struct {
		uint8_t permitido;
	} __attribute__((packed)) t_mov_permitido;

	// struct del mensaje turno concluido
	typedef struct{
		uint8_t bloqueado;
		uint8_t caracter;
		uint8_t termino;
	} __attribute__((packed)) t_turno_concluido;

	// la cabecera que se lee en todos los mensajes
	typedef struct {
			uint8_t tipo;
			uint16_t len;
	} __attribute__((packed)) t_cabecera;


	/**************************** FUNCIONES  ***************************/

	//funciones serializadoras
	char *srlz_movimiento_permitido(void* data);

	//funciones de-serializadoras
	void *deserializar_cabecera(char *buffer);
	void *deserializar_turno_concluido(char *buffer);

	//funciones de envio/recepcion
	void *recibir(int socket, int tipo);
	void enviar(int socket, int tipo, void *struct_mensaje, t_log *logger);
	int getnextmsg(int socket);

	//funciones de socket
	int init_socket_externo(int puerto, char *direccion, t_log *logger);
	int init_socket_escucha(int puerto, int optval, t_log *logger);

	// funciones de inicializacion
	void iniciar_serializadora();
	static void init_vec_deserial();
	static void init_vec_serial();



#endif /* SERIAL_H_ */
