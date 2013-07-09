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

	#define N_MENSAJES 23

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
	#define NOTIF_MUERTE_PERSONAJE 11					//PP->PN
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
	#define ENVIO_DE_DATOS_NIVEL_AL_ORQUESTADOR 22		//PN->HO
	#define	NOTIF_RECURSO_CONCEDIDO 23					//HO->PP

	//typedefs de punteros a funciones
	typedef void *(*p_funcion_deserial)(char *buffer);
	typedef char *(*p_funcion_serial)(void *data, int *tamanio);

	//extern p_funcion_deserial vec_deserializador[N_MENSAJES];
	//extern p_funcion_serial vec_serializador[N_MENSAJES];


	/**************************** STRUCTS  ***************************/

	//1 - struct del mensaje que le envia el personaje al orquestador para recibir como respuesta la informacion que necesita (ips/puertos)
    typedef struct {
    	char *nivel_solicitado;
    	char solicitor;
    }__attribute__((packed)) t_solicitud_info_nivel;


    //2 - respuesta del planificador al personaje con las ips/puertos pedidos
	typedef struct{
		uint16_t puerto_nivel;
		uint16_t puerto_planificador;
		char *ip_nivel;
		char *ip_planificador;
	}__attribute__((packed)) t_info_nivel_planificador ;


	//3 - struct simbolico del mensaje movimiento permitido(no se usa)/*	FIJARSE DESPUES EN PERSONAJE SI HAY UN RECIBIR DESPUES DEL GETNEXTMSG	*/
	typedef struct {
		char permitido;
	} __attribute__((packed)) t_mov_permitido ;


	//4 - struct que simboliza una solicitud de movimiento del personaje al nivel
	typedef struct {
		char char_personaje;
		char x;
		char y;
	} __attribute__((packed)) t_solicitud_movimiento;


	//5 - struct del mensaje que envia el nivel al personaje, aprobando o no una solicitud de movimiento
	typedef struct{
		char aprobado;
	} __attribute__((packed)) t_resp_solicitud_movimiento;


	//6 - struct del mensaje que envia el personaje al nivel pidiendo la ubicacion de un recurso, "recurso" es el char de aquel
	typedef struct{
		char recurso;
	} __attribute__((packed)) t_solicitud_ubicacion_recurso;


	//7 - struct del mensaje que envia el personaje al nivel pidiendo una instancia de un recurso
	typedef struct{
		char instancia_recurso;
	} __attribute__((packed)) t_solcitud_instancia_recurso;


	//8 - struct del mensaje que envia el nivel al personaje en respuesta al pedido de instancia de un recurso, concediendolo o no
	typedef struct{
		char concedido;
	} __attribute__((packed)) t_rspta_solicitud_instancia_recurso;


	//9 - struct del mensaje turno concluido que el personaje envia al hilo planificador en cada dada de quantum
	typedef struct{
		char bloqueado ;// usarlo como booleano
		char recurso_de_bloqueo;
	} __attribute__((packed)) t_turno_concluido;


	//10 - struct del mensaje que envia el personaje al nivel cuando termino de reunir sus objetivos
	typedef struct{
		char char_personaje;
	} __attribute__((packed)) t_notificacion_nivel_cumplido;


	/*11 - FALTA	*/


	/*12 - FALTA 	*/


	//13 - struct del mensaje que envia el nivel al orquestador con los recursos liberados de la forma "ACDC"
	typedef struct{
		char *recursos_liberados;
	}__attribute__((packed)) t_notif_recursos_liberados;


	//14 - struct del mensaje que envia el orquestador al nivel con los recursos reasignados de la forma "#A&B" y remanentes: "CACA"
	typedef struct{
		char *asignaciones;
		char *remanentes;
	}__attribute__((packed)) t_notif_recursos_reasignados;


	//15 - struct del mensaje que envia el hilo que chequea deadlock del nivel al orquestador con los personajes en deadlock
	typedef struct{
		char *pjes_deadlock;
	} __attribute__((packed)) t_solicitud_recupero_deadlock;


	//16 - struct del mensaje que envia el orquestador al nivel para notificarlo de la muerte de un personaje
	typedef struct{
		char char_personaje;
	}__attribute__((packed)) t_notif_eleccion_de_victima;


	//17 - struct del mensaje que envia el orquestador al personaje cuando debe morir a causa de interbloqueo
	typedef struct{
		char condenado ; // lo tratamos como booleano
		// creo que también es simbólico porque ni bien recibe esto, el personaje va y se muere
	} __attribute__((packed)) t_personaje_condenado;


	//18 - struct del mensaje que envia el personaje al orquestador cuando termino su plan de nivel...ojo con este...
	typedef struct{
		char * personaje;
	} __attribute__((packed)) t_notificacion_plan_terminado;


	//19 - struct del mensaje que envia el personaje al planificador cuando conecta por primera vez
	typedef struct {
		char char_personaje;
		char *nombre_personaje;
	} __attribute__((packed)) t_datos_delPersonaje_alPlanificador;


	//20 - struct del mensaje que envia el nivel al personaje para que sepa donde esta ubicado el recurso
	typedef struct{
		char x;
		char y;
	} __attribute__((packed)) t_ubicacion_recurso ;
	

	//21 - mensaje que un personaje envia al nivel cada vez que se quiere conectar por primera vez
	typedef struct{
		char char_personaje;
		char *nombre_personaje;
		char *necesidades;
	} __attribute__((packed)) t_datos_delPersonaje_alNivel;


	//22 - mensaje que el nivel le envia al orquestador con sus datos apenas arranca
	typedef struct{
		char *nombre;
		char *recursos_nivel;
		uint16_t puerto_nivel;
	} __attribute__((packed)) t_envio_deDatos_delNivel_alOrquestador;

	//23 - mensaje que le manda al orquestador al personaje cuando le da el recurso por el que estaba bloqueado
	typedef struct{
		char recurso; //esto es simbolico,probablemente no lo necesite
	}__attribute__((packed)) t_concesion_recurso;


	//la cabecera que se lee en todos los mensajes
	typedef struct {
		char tipo;
		uint16_t len;
	} __attribute__((packed)) t_cabecera;


	/**************************** FUNCIONES  ***************************/


	//funciones serializadoras
	char *srlz_solicitud_info_nivel(void *data, int *tamanio);				//1
	char *srlz_info_nivel_y_planificador(void *data, int *tamanio);			//2
	char *srlz_movimiento_permitido(void *data, int *tamanio);				//3
    char *srlz_solicitud_de_movimiento(void *data, int *tamanio);			//4
    char *srlz_resp_a_solicitud_movimiento(void *data, int *tamanio);		//5
    char *srlz_solicitud_ubicacion_recurso(void *data, int *tamanio);		//6
    char *srlz_solicitud_instancia_recurso(void *data,int *tamanio);		//7
    char *srlz_rspta_solicitud_instancia_recurso(void *data,int *tamanio);	//8
	char *srlz_turno_concluido(void *data, int *tamanio);					//9
    char *srlz_notificacion_nivel_cumplido(void *data,int *tamanio);		//10
    char *srlz_notif_recursos_liberados(void *data, int *tamanio);			//13
    char *srlz_notif_recursos_reasignados(void *data, int *tamanio);		//14
    char *srlz_envio_deDatos_delNivel_alOrquestador(void *data,int*tamanio);//15
    char *srlz_notif_eleccion_de_victima(void *data, int *tamanio);			//16
    char *srlz_personaje_condenado(void *data, int *tamanio);				//17
    char *srlz_notificacion_plan_terminado(void *data,int *tamanio);		//18
    char *srlz_datos_delPersonaje_alPlanificador(void *data, int *tamanio);	//19
    char *srlz_ubicacion_de_recurso(void *data, int *tamanio);				//20
    char *srlz_datos_delPersonaje_alNivel(void *data, int *tamanio);		//21
    char *srlz_solicitud_recupero_deadlock(void *data, int *tamanio);		//22
    char *srlz_concesion_recurso(void *data, int *tamanio);					//23

    //faltan en el .c y capaz no se usa
    char *srlz_notificacion_muerte_personaje(void *data,int *tamanio);		//11


    //listo
    void *deserializar_solicitud_info_nivel(char *buffer); 					//1
	void *deserializar_info_nivel_y_planificador(char *buffer);				//2
	void *deserializar_movimiento_permitido(char *buffer);					//3
    void *deserializar_solicitud_de_movimiento(char *buffer);				//4
    void *deserializar_resp_a_solicitud_movimiento(char *buffer);			//5
    void *deserializar_solicitud_ubicacion_recurso(char *buffer);			//6
    void *deserializar_solicitud_instancia_recurso(char *buffer);			//7
    void *deserializar_rspta_solicitud_instancia_recurso(char *buffer);		//8
	void *deserializar_turno_concluido(char *buffer);						//9
    void *deserializar_notificacion_nivel_cumplido(char *buffer);			//10
    void *deserializar_notif_recursos_liberados(char *buffer);				//13
    void *deserializar_notif_recursos_reasignados(char *buffer);			//14
    void *deserializar_envio_deDatos_delNivel_alOrquestador(char *buffer);	//15
    void *deserializar_notif_eleccion_de_victima(char *buffer);				//16
	void *deserializar_personaje_condenado(char *buffer);					//17
    void *deserializar_notificacion_plan_terminado(char *buffer);			//18
	void *deserializar_datos_delPersonaje_alPlanificador(char *buffer);		//19
    void *deserializar_ubicacion_de_recurso(char *buffer);					//20
    void *deserializar_datos_delPersonaje_alNivel(char *buffer);			//21
    void *deserializar_solicitud_recupero_deadlock(char *buffer);			//22
    void *deserializar_concesion_recurso(char *buffer);						//23

    //faltan en el .c y capaz no se usa
    void *deserializar_notificacion_muerte_personaje(char *buffer);			//11


	//funciones de envio/recepcion
	int getnextmsg(int socket);
	void *recibir(int socket, int tipo);
	int enviar(int socket, int tipo, void *struct_mensaje, t_log *logger);
	t_cabecera *deserializar_cabecera(char *buffer);

	//funciones de socket
	bool is_connected(int socket);
	char *get_ip_string(int socket);
	int init_socket_externo(int puerto, char *direccion, t_log *logger);
	int init_socket_escucha(int puerto, int optval, t_log *logger);

	// funcion de inicializacion
	void iniciar_serializadora();



#endif /* SERIAL_H_ */
