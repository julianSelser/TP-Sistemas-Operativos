/*
 * serial.c
 *
 *  Created on: Jun 7, 2013
 *      Author: julian
 */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include "serial.h"

// falta setear los sockets para cerrarse con execve
#define N_MENSAJES 20 //por ahora son 20

static p_funcion_deserial vec_deserializador[N_MENSAJES];
static p_funcion_serial vec_serializador[N_MENSAJES];


/******************************************* FUNCIONES DE INICIALIZADO ********************************************/
/*									ESTO ES TEMPORAL; HAY UNA MEJOR MANERA DE HACERLO							  */


//setea los vec de la serializadora
void iniciar_serializadora(){
	init_vec_serial();
	init_vec_deserial();
}

//setea el vector de serializadores
static void init_vec_serial(){
	vec_serializador[NOTIF_MOVIMIENTO_PERMITIDO] = srlz_movimiento_permitido;//todo ir agregando a medida que se escriben
}
//setea el vector de de-serializadores
static void init_vec_deserial(){
	vec_deserializador[NOTIF_TURNO_CONCLUIDO] = deserializar_turno_concluido;//todo ir agregando a medida que se escriben
}



/********************************************* FUNCIONES QUE RECIBEN **********************************************/
/********************************************* siempre usar recibir ***********************************************/
/********************************************* despues de getnxtmsg ***********************************************/



//mira el tipo de dato entrante en el flujo de entrada sin vaciar el socket
int getnextmsg(int socket){
	uint8_t tipo;
	char buffer[sizeof(uint8_t)];

	if(recv(socket, buffer, sizeof(t_cabecera), MSG_PEEK) < 0){
		/*todo logear*/;
		exit(1);
	}

	memcpy(&tipo, buffer, sizeof(uint8_t));

	return tipo;
}

//lee de un socket y devuelve un struct validando por "tipo"
void *recibir(int socket, int tipo)
{
	char buffer_cabecera[sizeof(t_cabecera)];

	// Primero: Recibir el header para saber cuando ocupa el payload.
	if (recv(socket, buffer_cabecera, sizeof(t_cabecera), MSG_WAITALL) <= 0) {
		/*todo logear*/
		exit(1);
	}

	// Segundo: Alocar memoria suficiente para el payload.
	t_cabecera cabecera = deserializar_cabecera(buffer_cabecera);
	char buffer_cuerpo[cabecera.len];

	// Tercero: Recibir el payload.
	if(recv(socket, buffer_cuerpo, cabecera.len, MSG_WAITALL) < 0){
		/*todo logear*/;
		exit(1);
	}

	return cabecera.tipo==tipo? vec_deserializador[tipo](buffer_cuerpo) : NULL ;
}

//funcion que recibe un socket, un tipo de mensaje, un struct y un logger
//envia un mensaje de un tipo serializando un struct al socket
void enviar(int socket, int tipo, void *struct_mensaje, t_log *logger)
{
	int tmp,offset = 0;
	char *serializado = vec_serializador[tipo](struct_mensaje);
	char buffer[sizeof(t_cabecera)+strlen(serializado)];

	//variables para castear
	uint16_t len_cast = strlen(serializado);
	uint8_t tipo_cast = tipo;

	//armado del buffer  con cabecera y lo serializado por vec_serializador
	memmove(buffer, &tipo_cast, tmp = sizeof(uint8_t));
	memmove(buffer + offset, &len_cast, tmp = sizeof(uint16_t));
	offset += tmp;
	memmove(buffer + offset, serializado, strlen(serializado));

	//envio de mensaje y limpieza de mallocs
	if(send(socket, buffer, strlen(buffer), 0) < 0){
		/*todo: logear*/
		exit(1);
	}
	free(struct_mensaje);
	free(serializado);
}

//OJO ACA:  deserializar cabecera no es parte del vector de de-serializacion y se usa en recibir
//			no necesitamos tenerla en memoria ni pasarla como void*
t_cabecera deserializar_cabecera(char *buffer){
	t_cabecera cabecera;

	memcpy(&cabecera.tipo, buffer, sizeof(uint8_t));
	memcpy(&cabecera.len, buffer, sizeof(uint16_t));

	return cabecera;
}



/********************************************** FUNCIONES DE-SERIALIZADORAS ******************************************/
/*  ATENCION: siempre las funciones para deserializar deben ser de la forma:  void *deserializar_algo(char *buffer); */



void *deserializar_turno_concluido(char *buffer){
	int tmp,offset = 0;
	t_turno_concluido *turno_fin = malloc(sizeof(t_turno_concluido));

	memcpy(&turno_fin->bloqueado, buffer, offset = sizeof(uint8_t));
	memcpy(&turno_fin->caracter, buffer + offset, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(&turno_fin->termino, buffer + offset, sizeof(uint8_t));

	return turno_fin;
}



/********************************************** FUNCIONES SERIALIZADORAS *********************************************/
/*  LOS MALLOCS SE LIBERAN EN LA FUNCION ENVIAR, TODOS: LOS DEL BUFFER Y LOS PASADOS COMO ESTRUCT PORQUE SE USAN AHI */



char *srlz_movimiento_permitido(void* data){
	t_mov_permitido *temp = data;
	char *buffer = malloc(sizeof(uint8_t));

	memcpy(buffer, &temp->permitido, sizeof(uint8_t));

	return buffer;
}



/********************************************** FUNCIONES DE SOCKET **************************************************/



//devuelve un socket para comunicacion listo para usar, escribiendo en el logger pasado y terminando el programa ante errores
int init_socket_externo(int puerto, char *direccion, t_log *logger)
{
	int unSocket;
	struct sockaddr_in socketInfo; // aca eclipse me dice que la variable no se usa...esta loco...se usa abajo...

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		/*todo logear:Error al crear socket*/
		exit(1);
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(direccion);
	socketInfo.sin_port = htons(puerto);

	return unSocket;
}


//devuelve un socket escucha listo para usar, escribiendo en el logger pasado y terminando el programa ante errores
int init_socket_escucha(int puerto, int optval, t_log *logger){
	int socketEscucha;
	struct sockaddr_in socketInfo;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		/*todo logear:error creando socket*/
		exit(1);
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(puerto);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))	!= 0) {
		/*todo logear:Error al bindear socket escucha*/
		exit(1);
	}
	return socketEscucha;
}
