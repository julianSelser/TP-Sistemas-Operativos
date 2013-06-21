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
    //vec_serializador[INFO_NIVEL_Y_PLANIFICADOR] = srlz_info_nivel_y_planificador;
	vec_serializador[NOTIF_MOVIMIENTO_PERMITIDO] = srlz_movimiento_permitido;
	vec_serializador[SOLICITUD_MOVIMIENTO_XY]=srlz_solicitud_de_movimiento;
    vec_serializador[RTA_SOLICITUD_MOVIMIENTO_XY]=srlz_resp_a_solicitud_movimiento;
    vec_serializador[SOLICITUD_INSTANCIA_RECURSO]=srlz_solicitud_instancia_recurso;
	vec_serializador[RTA_SOLICITUD_INSTANCIA_RECURSO]=srlz_rspta_solicitud_instancia_recurso;
    vec_serializador[NOTIF_TURNO_CONCLUIDO] = srlz_turno_concluido;
    vec_serializador[NOTIF_PERSONAJE_CONDENADO] = srlz_personaje_condenado;
	vec_serializador[INFO_UBICACION_RECURSO] = srlz_ubicacion_de_recurso;
    vec_serializador[SOLICITUD_INFO_NIVEL] = srlz_solicitud_info_nivel;

}



//setea el vector de de-serializadores
static void init_vec_deserial(){
	//todo ir agregando a medida que se escriben
	//vec_deserializador[ENVIO_DE_DATOS_AL_PLANIFICADOR] = deserializar_datos_delPersonaje_alPlanificador;
	//vec_deserializador[INFO_NIVEL_Y_PLANIFICADOR] = deserializar_info_nivel_y_planificador;
	vec_deserializador[NOTIF_MOVIMIENTO_PERMITIDO] = deserializar_movimiento_permitido;
	vec_deserializador[SOLICITUD_MOVIMIENTO_XY] = deserializar_solicitud_de_movimiento;
    vec_deserializador[RTA_SOLICITUD_MOVIMIENTO_XY] = deserializar_solicitud_de_movimiento;
    vec_deserializador[SOLICITUD_INSTANCIA_RECURSO] = deserializar_solicitud_instancia_recurso;
    vec_deserializador[RTA_SOLICITUD_INSTANCIA_RECURSO]=deserializar_rspta_solicitud_instancia_recurso;
	vec_deserializador[NOTIF_TURNO_CONCLUIDO] = deserializar_turno_concluido;
	vec_deserializador[NOTIF_PERSONAJE_CONDENADO] = deserializar_personaje_condenado;
	vec_deserializador[INFO_UBICACION_RECURSO] = deserializar_ubicacion_de_recurso;
    vec_deserializador[SOLICITUD_INFO_NIVEL]= deserializar_solicitud_info_nivel;
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
	t_cabecera cabecera;
	char *buffer;


	// Primero: Recibir el header para saber cuando ocupa el payload.
	if (recv(socket, &cabecera, sizeof(t_cabecera), MSG_WAITALL) <= 0) {
		/*todo logear*/
		exit(1);
	}


	// Segundo: Alocar memoria suficiente para el payload.
	buffer= malloc(sizeof(cabecera.len));

	// Tercero: Recibir el payload.
	if(recv(socket, buffer, cabecera.len, MSG_WAITALL) < 0){
		/*todo logear*/;
		exit(1);
	}

	//de-serializado y validacion de tipo
	void *deserializado = (*vec_deserializador[tipo])(buffer);
	bool tipo_valido = cabecera.tipo==tipo;

	free(buffer);

	return tipo_valido? deserializado : NULL ; // return el deserializado correspondiente al tipo validado
}


//funcion que recibe un socket, un tipo de mensaje, un struct y un logger
//envia un mensaje de un tipo serializando un struct al socket
void enviar(int socket, int tipo, void *struct_mensaje, t_log *logger)
{
	t_cabecera cabecera;
	int *tamanio = malloc(sizeof(int));
	char *serializado = (*vec_serializador[tipo])(struct_mensaje, tamanio);//contiene el serializado correspondiente dado por el "vec"
	char *buffer = malloc(sizeof(t_cabecera) + *tamanio);//buffer de envio del tamaño de la cabecera + el serializado

	cabecera.tipo = tipo;
	cabecera.len = *tamanio;

	//armado del buffer con cabecera y lo serializado por vec_serializador
	memmove(buffer, &cabecera, sizeof(t_cabecera));
	memcpy(buffer + sizeof(t_cabecera), serializado, *tamanio);

	//envio de mensaje y checkeo de errores
	if(send(socket, buffer, sizeof(t_cabecera) + *tamanio, 0) < 0){
		/*todo: logear*/
		exit(1);
	}
	//liberacion de mallocs
	free(serializado);
	free(tamanio);
	free(buffer);
}



/********************************************** FUNCIONES DE-SERIALIZADORAS ******************************************/
/*  ATENCION: siempre las funciones para deserializar deben ser de la forma:  void *deserializar_algo(char *buffer); */


void *deserializar_solicitud_info_nivel(char *buffer){
	int tmp;
	t_solicitud_info_nivel * solicitud = malloc(sizeof(t_solicitud_info_nivel));

    for(tmp=1;(buffer)[tmp-1]!='\0';tmp++);
    solicitud->nivel_solicitado=malloc(tmp);
    memcpy(&solicitud->nivel_solicitado,buffer,tmp);

		return solicitud;
}
void *deserializar_rspta_solicitud_instancia_recurso(char *buffer){
	t_rspta_solicitud_instancia_recurso * rpta_instancia = malloc(sizeof(t_rspta_solicitud_instancia_recurso));
	memcpy(&rpta_instancia->concedido,buffer,sizeof(uint8_t));
	return rpta_instancia;
}



void *deserializar_solicitud_instancia_recurso(char *buffer){

	t_solcitud_instancia_recurso * instancia = malloc(sizeof(t_solcitud_instancia_recurso));
	memcpy(&instancia->instancia_recurso,buffer,sizeof(uint8_t));
	return instancia;

}

void *deserializar_resp_a_solicitud_movimiento(char* buffer){

	t_resp_solicitud_movimiento * resp = malloc(sizeof(t_resp_solicitud_movimiento));
	memcpy(&resp->aprobado,buffer,sizeof(uint8_t));
	return resp;
}

void *deserializar_solicitud_de_movimiento(char *buffer){
	  int tmp=0,offset=0;

   t_solicitud_movimiento * solicitud = malloc(sizeof(t_solicitud_movimiento));
   memcpy(&solicitud->eje_x,buffer,tmp=sizeof(uint8_t));
   offset=tmp;
   memcpy(&solicitud->eje_y,buffer+offset,sizeof(uint8_t));
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


void * deserializar_info_nivel_y_planificador(char *buffer){

	int tmp = 0,offset=0;

	t_info_nivel_planificador *info = malloc(sizeof(t_info_nivel_planificador));
	memcpy(&info->puerto_nivel,buffer,tmp=sizeof(uint16_t));
	offset+=tmp;
	memcpy(&info->puerto_planificador,buffer + offset,tmp=sizeof(uint16_t));
    offset+=tmp;

    for(tmp=1;(buffer+offset)[tmp-1]!='\0';tmp++);
    info->ip_nivel=malloc(tmp);
    memcpy(&info->ip_nivel,buffer+offset,tmp);

    offset+=tmp;
    for(tmp=1;(buffer+offset)[tmp-1]!='\0';tmp++);
    info->ip_planificador=malloc(tmp);
    memcpy(&info->ip_planificador,buffer+offset,tmp);

	return info;  // devuelve el struct de t_info_nivel_planificador
}


void *deserializar_turno_concluido(char *buffer){
	int tmp = 0,offset = 0;
	t_turno_concluido *turno_fin = malloc(sizeof(t_turno_concluido));

	memcpy(&turno_fin->bloqueado, buffer, tmp = sizeof(uint8_t));
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
char *srlz_solicitud_info_nivel(void *data, int *tamanio){
	t_solicitud_info_nivel * sol = data;
	//char* buffer = malloc(sizeof(t_solicitud_info_nivel));
	char* buffer = malloc(strlen(sol->nivel_solicitado)+1);
    memcpy(buffer,sol->nivel_solicitado,*tamanio = strlen(sol->nivel_solicitado)+1);
	free(data);
	return buffer;
}

char *srlz_rspta_solicitud_instancia_recurso(void *data,int *tamanio){
	char* buffer = malloc(sizeof(t_rspta_solicitud_instancia_recurso));
	memcpy(buffer,data,*tamanio=sizeof(t_rspta_solicitud_instancia_recurso));
    free(data);
    return buffer;
}


char *srlz_solicitud_instancia_recurso(void *data, int *tamanio){
	char* buffer = malloc(sizeof(t_solcitud_instancia_recurso));
	memcpy(buffer, data, *tamanio = sizeof(t_solcitud_instancia_recurso));
	free(data);
	return buffer;

}


char *srlz_resp_a_solicitud_movimiento(void *data, int *tamanio){
	char * buffer = malloc(sizeof(t_resp_solicitud_movimiento));
	memcpy(buffer, data, *tamanio = sizeof(uint8_t));
	free(data);
	return buffer;

}


char *srlz_solicitud_de_movimiento(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_solicitud_movimiento));
	memcpy(buffer, data, *tamanio = sizeof(t_solicitud_movimiento));
	free(data);
	return buffer;
}


char *srlz_ubicacion_de_recurso(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_ubicacion_recurso));
	memcpy(buffer, data, *tamanio=sizeof(t_ubicacion_recurso));
	free(data);
	return buffer;
}


char *srlz_personaje_condenado(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_personaje_condenado));
	memcpy(buffer,data, *tamanio = sizeof(t_personaje_condenado));
	free(data);
	return buffer;
}


char *srlz_info_nivel_y_planificador(void *data, int *tamanio)
{
	return NULL;
}


/*//char *srlz_info_nivel_y_planificador(void *data){
//   int tmp =0,offset=0;
//
//	t_info_nivel_planificador *d = data;
//	char *buffer = malloc(sizeof(uint32_t));
//
//	memcpy(buffer,&d->info_nivel,tmp=sizeof(uint16_t));
//    offset+=tmp;
//    memcpy(buffer+offset,&d->info_planificador,sizeof(uint16_t));
//
//	return buffer;
//}
//////////////////*/

char *srlz_movimiento_permitido(void* data, int *tamanio){
	char *buffer = malloc(sizeof(t_mov_permitido));
	memcpy(buffer, data, *tamanio = sizeof(t_mov_permitido));
	free(data);
	return buffer;
}


char *srlz_turno_concluido(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_turno_concluido));
	memcpy(buffer, data, *tamanio = sizeof(t_turno_concluido));
	free(data);
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
