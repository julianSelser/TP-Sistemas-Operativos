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
#include <stdbool.h>
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
	//todo ir agregando a medida que se escriben
	vec_serializador[NOTIF_MOVIMIENTO_PERMITIDO] = srlz_movimiento_permitido;
	vec_serializador[NOTIF_TURNO_CONCLUIDO] = srlz_turno_concluido;
	vec_serializador[ENVIO_DE_DATOS_AL_PLANIFICADOR] = srlz_datos_delPersonaje_alPlanificador;
    vec_serializador[INFO_NIVEL_Y_PLANIFICADOR] = srlz_info_nivel_y_planificador;
    vec_serializador[NOTIF_PERSONAJE_CONDENADO] = srlz_personaje_condenado;
	vec_serializador[SOLICITUD_UBICACION_RECURSO] = srlz_ubicacion_de_recurso;
	vec_serializador[SOLICITUD_MOVIMIENTO_XY]=srlz_solicitud_de_movimiento;
    vec_serializador[RTA_SOLICITUD_MOVIMIENTO_XY]=srlz_resp_a_solicitud_movimiento;
    vec_serializador[SOLICITUD_INSTANCIA_RECURSO]=srlz_solicitud_de_recurso;
}


//setea el vector de de-serializadores
static void init_vec_deserial(){
	//todo ir agregando a medida que se escriben
	vec_deserializador[NOTIF_TURNO_CONCLUIDO] = deserializar_turno_concluido;
	vec_deserializador[ENVIO_DE_DATOS_AL_PLANIFICADOR] = deserializar_datos_delPersonaje_alPlanificador;
	vec_deserializador[NOTIF_MOVIMIENTO_PERMITIDO] = deserializar_movimiento_permitido;
	vec_deserializador[INFO_NIVEL_Y_PLANIFICADOR] = deserializar_info_nivel_planificador;
	vec_deserializador[NOTIF_PERSONAJE_CONDENADO] = deserializar_personaje_condenado;
	vec_deserializador[SOLICITUD_UBICACION_RECURSO] = deserializar_ubicacion_de_recurso;
	vec_deserializador[SOLICITUD_MOVIMIENTO_XY]=deserializar_solicitud_de_movimiento;
    vec_deserializador[RTA_SOLICITUD_MOVIMIENTO_XY]=deserializar_solicitud_de_movimiento;
    vec_deserializador[SOLICITUD_INSTANCIA_RECURSO]=deserializar_solicitud_de_recurso;
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
	t_cabecera *cabecera;
	char *buffer_cuerpo;
	char *buffer_cabecera = malloc(sizeof(t_cabecera));

	// Primero: Recibir el header para saber cuando ocupa el payload.
	if (recv(socket, buffer_cabecera, sizeof(t_cabecera)-1, MSG_WAITALL) <= 0) {
		/*todo logear*/
		exit(1);
	}

	// Segundo: Alocar memoria suficiente para el payload.
	cabecera = deserializar_cabecera(buffer_cabecera);
	buffer_cuerpo = malloc(cabecera->len);

	// Tercero: Recibir el payload.
	if(recv(socket, buffer_cuerpo, cabecera->len, MSG_WAITALL) < 0){
		/*todo logear*/;
		exit(1);
	}

	//de-serializado y validacion de tipo
	void *deserializado = (*vec_deserializador[tipo])(buffer_cuerpo);
	bool tipo_valido = cabecera->tipo==tipo;

	free(cabecera);
	free(buffer_cuerpo);
	free(buffer_cabecera);

	return tipo_valido? deserializado : NULL ;
}


//funcion que recibe un socket, un tipo de mensaje, un struct y un logger
//envia un mensaje de un tipo serializando un struct al socket
void enviar(int socket, int tipo, void *struct_mensaje, t_log *logger)
{
	int tmp = 0,offset = 0;
	char *serializado = (*vec_serializador[tipo])(struct_mensaje);//contiene el serializado correspondiente dado por el "vec"
	char *buffer = malloc(sizeof(t_cabecera)+strlen(serializado));//buffer de envio del tamaño de la cabecera + el serializado

	//variables para castear
	uint16_t len_cast = strlen(serializado);
	uint8_t tipo_cast = tipo;

	//armado del buffer  con cabecera y lo serializado por vec_serializador
	memcpy(buffer, &tipo_cast, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(buffer + offset, &len_cast, tmp = sizeof(uint16_t));
	offset += tmp;
	memcpy(buffer + offset-1, serializado, strlen(serializado));

	//envio de mensaje y checkeo de errores
	if(send(socket, buffer, strlen(buffer), 0) < 0){
		/*todo: logear*/
		exit(1);
	}

	//liberacion de mallocs
	free(struct_mensaje);
	free(serializado);
	free(buffer);
}


//OJO ACA:  deserializar cabecera no es parte del vector de de-serializacion y se usa en recibir
//			no necesitamos tenerla en memoria ni pasarla como void*
t_cabecera *deserializar_cabecera(char *buffer){
	t_cabecera *cabecera = malloc(sizeof(t_cabecera));

	memcpy(&cabecera->tipo, buffer, sizeof(uint8_t));
	memcpy(&cabecera->len, buffer + sizeof(uint8_t), sizeof(uint16_t));

	return cabecera;
}



/********************************************** FUNCIONES DE-SERIALIZADORAS ******************************************/
/*  ATENCION: siempre las funciones para deserializar deben ser de la forma:  void *deserializar_algo(char *buffer); */


///////////////////////////////
void *deserializar_solicitud_de_recurso(char *buffer){

	t_solcitud_instancia_recurso * instancia = malloc(sizeof(t_solcitud_instancia_recurso));
	memcpy(&instancia,buffer,sizeof(uint8_t));
	return instancia;

}

void *deserializar_resp_a_solicitud_movimiento(char* buffer){

	t_resp_solicitud_movimiento * resp = malloc(sizeof(t_resp_solicitud_movimiento));
	memcpy(&resp,buffer,sizeof(uint8_t));
	return resp;
}

void *deserializar_solicitud_de_movimiento(char *buffer){

   t_solicitud_movimiento * solicitud = malloc(sizeof(t_solicitud_movimiento));
   memcpy(&solicitud,buffer,sizeof(uint8_t));
   return solicitud;
}


void *deserializar_ubicacion_de_recurso(char *buffer){
	  int tmp=0,offset=0;

	t_ubicacion_recurso * ubicacion = malloc(sizeof(t_ubicacion_recurso));
	memcpy(&ubicacion->x,buffer,tmp=sizeof(uint8_t));
	offset+=tmp;
	memcpy(&ubicacion->y,buffer+offset,sizeof(uint8_t));

	return ubicacion;

  }

void *deserializar_personaje_condenado(char *buffer){


	t_personaje_condenado *per = malloc(sizeof(t_personaje_condenado));
	memcpy(&per->condenado,buffer,sizeof(uint8_t));

	return per;

}


void * deserializar_info_nivel_planificador(char *buffer){

	int tmp =0,offset=0;

	t_info_nivel_planificador *info = malloc(sizeof(t_info_nivel_planificador));
	memcpy(&info->info_nivel,buffer,tmp=sizeof(uint16_t));
	offset+=tmp;
	memcpy(&info->info_planificador,buffer + offset,sizeof(uint16_t));

	return info;  // devuelve el struct de t_info_nivel_planificador
}

//////////////////////////
void *deserializar_turno_concluido(char *buffer){
	int tmp = 0,offset = 0;
	t_turno_concluido *turno_fin = malloc(sizeof(t_turno_concluido));

	memcpy(&turno_fin->bloqueado, buffer, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(&turno_fin->termino_nivel, buffer + offset, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(&turno_fin->recurso_de_bloqueo, buffer + offset, sizeof(uint8_t));

	return turno_fin;
}


void *deserializar_datos_delPersonaje_alPlanificador(char *buffer){
	t_datos_delPersonaje_alPlanificador *datos = malloc(sizeof(t_datos_delPersonaje_alPlanificador));

	memcpy(&datos->char_personaje, buffer, sizeof(uint8_t));

	return datos;
}


void *deserializar_movimiento_permitido(char *buffer){
	t_mov_permitido *mov_permitido = malloc(sizeof(t_mov_permitido));

	memcpy(&mov_permitido->permitido, buffer, sizeof(t_mov_permitido));

	return mov_permitido;
}



/********************************************** FUNCIONES SERIALIZADORAS *********************************************/
/*  LOS MALLOCS SE LIBERAN EN LA FUNCION ENVIAR, TODOS: LOS DEL BUFFER Y LOS PASADOS COMO STRUCT PORQUE SE USAN AHI  */



char *srlz_solicitud_de_recurso(void *data){

	char* buffer = malloc(sizeof(uint8_t));
	t_solcitud_instancia_recurso * inst = data;
	memcpy(buffer,&inst->recurso,sizeof(uint8_t));
	return buffer;

}
char *srlz_resp_a_solicitud_movimiento(void *data){

	char * buffer = malloc(sizeof(uint8_t));
	t_resp_solicitud_movimiento *resp = data;
	memcpy(buffer,&resp->resp_solicitud,sizeof(uint8_t));

	return buffer;

}


char *srlz_solicitud_de_movimiento(void *data){

	char *buffer = malloc(sizeof(uint8_t));
	t_solicitud_movimiento *solicitud = data;
	memcpy(buffer,&solicitud->solicito_moverme,sizeof(uint8_t));

	return buffer;
}
char *srlz_ubicacion_de_recurso(void *data){
	int tmp=0,offset=0;

	t_ubicacion_recurso *ubicacion = data;
	char *buffer = malloc(sizeof(uint16_t));
	memcpy(buffer,&ubicacion->x,tmp=sizeof(uint8_t));
	offset+=tmp;
	memcpy(buffer+offset,&ubicacion->y,sizeof(uint8_t));
	return buffer;
}



char *srlz_personaje_condenado(void *data){

	t_personaje_condenado *per = data;
	char *buffer =malloc(sizeof(uint8_t));

	memcpy(buffer,&per->condenado,sizeof(uint8_t));

	return buffer;
}


char *srlz_info_nivel_y_planificador(void *data){
   int tmp =0,offset=0;

	t_info_nivel_planificador *d = data;
	char *buffer = malloc(sizeof(uint32_t));

	memcpy(buffer,&d->info_nivel,tmp=sizeof(uint16_t));
    offset+=tmp;
    memcpy(buffer+offset,&d->info_planificador,sizeof(uint16_t));

	return buffer;
}


char *srlz_datos_delPersonaje_alPlanificador(void *data){
	t_datos_delPersonaje_alPlanificador *d = data;
	char *buffer = malloc(sizeof(uint8_t));

	memcpy(buffer, &d->char_personaje, sizeof(uint8_t));

	return buffer;
}


char *srlz_movimiento_permitido(void* data){
	t_mov_permitido *d = data;
	char *buffer = malloc(sizeof(uint8_t));

	memcpy(buffer, &d->permitido, sizeof(uint8_t));

	return buffer;
}


char *srlz_turno_concluido(void *data){
	int tmp = 0 ,offset = 0;
	t_turno_concluido *d = data;
	char *buffer = malloc(3*sizeof(uint8_t));

	memcpy(buffer, &d->bloqueado, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(buffer + offset, &d->termino_nivel, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(buffer + offset, &d->recurso_de_bloqueo, sizeof(uint8_t));

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

	connect(unSocket, (struct sockaddr*) &socketInfo, sizeof(socketInfo));

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

	listen(socketEscucha, 10);

	return socketEscucha;
}
