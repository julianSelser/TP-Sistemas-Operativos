/*
 * serial.c
 *
 *  Created on: Jun 7, 2013
 *      Author: julian
 */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/string.h>
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
#include <fcntl.h>
#include "serial.h"


static p_funcion_deserial vec_deserializador[N_MENSAJES];
static p_funcion_serial vec_serializador[N_MENSAJES];


static void init_vec_deserial();
static void init_vec_serial();


/******************************************* FUNCIONES DE INICIALIZADO ********************************************/



//setea los vec de la serializadora
void iniciar_serializadora(){
	init_vec_serial();
	init_vec_deserial();
}


//setea el vector de serializadores
static void init_vec_serial(){
    vec_serializador[SOLICITUD_INFO_NIVEL] = srlz_solicitud_info_nivel;
    vec_serializador[INFO_NIVEL_Y_PLANIFICADOR] = srlz_info_nivel_y_planificador;
	vec_serializador[NOTIF_MOVIMIENTO_PERMITIDO] = srlz_movimiento_permitido;
	vec_serializador[SOLICITUD_MOVIMIENTO_XY]=srlz_solicitud_de_movimiento;
    vec_serializador[RTA_SOLICITUD_MOVIMIENTO_XY]=srlz_resp_a_solicitud_movimiento;
    vec_serializador[SOLICITUD_UBICACION_RECURSO] = srlz_solicitud_ubicacion_recurso;
    vec_serializador[SOLICITUD_INSTANCIA_RECURSO]=srlz_solicitud_instancia_recurso;
	vec_serializador[RTA_SOLICITUD_INSTANCIA_RECURSO]=srlz_rspta_solicitud_instancia_recurso;
    vec_serializador[NOTIF_TURNO_CONCLUIDO] = srlz_turno_concluido;
    vec_serializador[NOTIF_NIVEL_CUMPLIDO] = srlz_notificacion_nivel_cumplido;
    vec_serializador[NOTIF_RECURSOS_LIBERADOS] = srlz_notif_recursos_liberados;
    vec_serializador[NOTIF_RECURSOS_REASIGNADOS] = srlz_notif_recursos_reasignados;
    vec_serializador[SOLICITUD_RECUPERO_DEADLOCK] = srlz_solicitud_recupero_deadlock;
	vec_serializador[NOTIF_ELECCION_VICTIMA] = srlz_notif_eleccion_de_victima;
    vec_serializador[NOTIF_PERSONAJE_CONDENADO] = srlz_personaje_condenado;
    vec_serializador[NOTIF_PLAN_TERMINADO] = srlz_notificacion_plan_terminado;
	vec_serializador[ENVIO_DE_DATOS_AL_PLANIFICADOR] = srlz_datos_delPersonaje_alPlanificador;
	vec_serializador[INFO_UBICACION_RECURSO] = srlz_ubicacion_de_recurso;
	vec_serializador[ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL] = srlz_datos_delPersonaje_alNivel;
	vec_serializador[ENVIO_DE_DATOS_NIVEL_AL_ORQUESTADOR] = srlz_envio_deDatos_delNivel_alOrquestador;
	vec_serializador[NOTIF_RECURSO_CONCEDIDO] = srlz_concesion_recurso;
}


//setea el vector de de-serializadores
static void init_vec_deserial(){
    vec_deserializador[SOLICITUD_INFO_NIVEL] = deserializar_solicitud_info_nivel;
	vec_deserializador[INFO_NIVEL_Y_PLANIFICADOR] = deserializar_info_nivel_y_planificador;
	vec_deserializador[NOTIF_MOVIMIENTO_PERMITIDO] = deserializar_movimiento_permitido;
	vec_deserializador[SOLICITUD_MOVIMIENTO_XY]= deserializar_solicitud_de_movimiento;
    vec_deserializador[RTA_SOLICITUD_MOVIMIENTO_XY]= deserializar_resp_a_solicitud_movimiento;
    vec_deserializador[SOLICITUD_UBICACION_RECURSO] = deserializar_solicitud_ubicacion_recurso;
    vec_deserializador[SOLICITUD_INSTANCIA_RECURSO]= deserializar_solicitud_instancia_recurso;
	vec_deserializador[RTA_SOLICITUD_INSTANCIA_RECURSO]= deserializar_rspta_solicitud_instancia_recurso;
    vec_deserializador[NOTIF_TURNO_CONCLUIDO] = deserializar_turno_concluido;
    vec_deserializador[NOTIF_NIVEL_CUMPLIDO] = deserializar_notificacion_nivel_cumplido;
    vec_deserializador[NOTIF_RECURSOS_LIBERADOS] = deserializar_notif_recursos_liberados;
    vec_deserializador[NOTIF_RECURSOS_REASIGNADOS] = deserializar_notif_recursos_reasignados;
    vec_deserializador[SOLICITUD_RECUPERO_DEADLOCK] = deserializar_solicitud_recupero_deadlock;
    vec_deserializador[NOTIF_ELECCION_VICTIMA] = deserializar_notif_eleccion_de_victima;
    vec_deserializador[NOTIF_PERSONAJE_CONDENADO] = deserializar_personaje_condenado;
    vec_deserializador[NOTIF_PLAN_TERMINADO] = deserializar_notificacion_plan_terminado;
	vec_deserializador[ENVIO_DE_DATOS_AL_PLANIFICADOR] = deserializar_datos_delPersonaje_alPlanificador;
	vec_deserializador[INFO_UBICACION_RECURSO] = deserializar_ubicacion_de_recurso;
	vec_deserializador[ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL] = deserializar_datos_delPersonaje_alNivel;
	vec_deserializador[ENVIO_DE_DATOS_NIVEL_AL_ORQUESTADOR] = deserializar_envio_deDatos_delNivel_alOrquestador;
	vec_deserializador[NOTIF_RECURSO_CONCEDIDO] = deserializar_concesion_recurso;
}



/********************************************* FUNCIONES QUE RECIBEN **********************************************/
/********************************************* siempre usar recibir ***********************************************/
/********************************************* despues de getnxtmsg ***********************************************/



//mira el tipo de dato entrante en el flujo de entrada sin vaciar el socket
int getnextmsg(int socket){
	char tipo;
	char buffer[sizeof(char)];

	if(recv(socket, buffer, sizeof(t_cabecera), MSG_PEEK) < 0){
		exit(1);
	}

	memcpy(&tipo, buffer, sizeof(char));

	return tipo;
}


//lee de un socket y devuelve un struct validando por "tipo"
void *recibir(int socket, int tipo)
{
	char *buffer;
	t_cabecera cabecera;

	// Primero: Recibir el header para saber cuando ocupa el payload.
	if (recv(socket, &cabecera, sizeof(t_cabecera), MSG_WAITALL) <= 0) {
		exit(1);
	}

	// Segundo: Alocar memoria suficiente para el payload.
	buffer= malloc(cabecera.len);

	// Tercero: Recibir el payload.
	if(recv(socket, buffer, cabecera.len, MSG_WAITALL) < 0){
		exit(1);
	}

	//de-serializado y validacion de tipo
	void *deserializado = (*vec_deserializador[tipo])(buffer);
	bool tipo_valido = cabecera.tipo==tipo;
	free(buffer);
	return tipo_valido? deserializado : NULL ; // return el deserializado correspondiente al tipo validado
}


//funcion que recibe un socket, un tipo de mensaje, un struct y un logger
//envia un mensaje de un tipo serializando un struct al socket, retorna el resultado de send()
int enviar(int socket, int tipo, void *struct_mensaje, t_log *logger)
{
	t_cabecera cabecera;
	int resultado, *tamanio = malloc(sizeof(int));
	char *serializado = (*vec_serializador[tipo])(struct_mensaje, tamanio);//contiene el serializado correspondiente dado por el "vec"
	char *buffer = malloc(sizeof(t_cabecera) + *tamanio);//buffer de envio del tamaño de la cabecera + el serializado

	cabecera.tipo = tipo;
	cabecera.len = *tamanio;

	//armado del buffer con cabecera y lo serializado por vec_serializador
	memmove(buffer, &cabecera, sizeof(t_cabecera));

	memcpy(buffer + sizeof(t_cabecera), serializado, *tamanio);

	//envio de mensaje, aca capaz implementar un sendall()
	resultado = send(socket, buffer, sizeof(t_cabecera) + *tamanio, MSG_NOSIGNAL);

	//liberacion de mallocs y retorno
	free(serializado);
	free(tamanio);
	free(buffer);
	return resultado;
}



/********************************************** FUNCIONES DE-SERIALIZADORAS ******************************************/
/*  ATENCION: siempre las funciones para deserializar deben ser de la forma:  void *deserializar_algo(char *buffer); */



//1
void *deserializar_solicitud_info_nivel(char *buffer){
	t_solicitud_info_nivel *solicitud = malloc(sizeof(t_solicitud_info_nivel));
	solicitud->nivel_solicitado = strdup(buffer);
	memcpy(&solicitud->solicitor, buffer+strlen(buffer)+1, sizeof(char));
    return solicitud;
}


//2
void * deserializar_info_nivel_y_planificador(char *buffer){
	int tmp = 0,offset=0;
	t_info_nivel_planificador *info = malloc(sizeof(t_info_nivel_planificador));

	memcpy(&info->puerto_nivel, buffer ,tmp=sizeof(uint16_t));
	offset+=tmp;
	memcpy(&info->puerto_planificador,buffer + offset,tmp=sizeof(uint16_t));
	offset+=tmp;

	info->ip_nivel = strdup(buffer+offset);

	return info;
}


//3
void *deserializar_movimiento_permitido(char *buffer){
	t_mov_permitido *mov_permitido = malloc(sizeof(t_mov_permitido));
	memcpy(&mov_permitido->permitido, buffer, sizeof(char));
	return mov_permitido;
}


//4
void *deserializar_solicitud_de_movimiento(char *buffer){
	t_solicitud_movimiento * solicitud = malloc(sizeof(t_solicitud_movimiento));
	memcpy(solicitud, buffer, sizeof(t_solicitud_movimiento));
	return solicitud;
}


//5
void *deserializar_resp_a_solicitud_movimiento(char* buffer){
	t_resp_solicitud_movimiento * resp = malloc(sizeof(t_resp_solicitud_movimiento));
	memcpy(&resp->aprobado,buffer,sizeof(char));
	return resp;
}


//6
void *deserializar_solicitud_ubicacion_recurso(char *buffer){
	t_solicitud_ubicacion_recurso *solicitud_ubicacion = malloc(sizeof(t_solicitud_ubicacion_recurso));
	memcpy(&solicitud_ubicacion->recurso, buffer, sizeof(char));
	return solicitud_ubicacion;
}


//7
void *deserializar_solicitud_instancia_recurso(char *buffer){
	t_solcitud_instancia_recurso *instancia = malloc(sizeof(t_solcitud_instancia_recurso));
	memcpy(&instancia->instancia_recurso,buffer,sizeof(char));
	return instancia;
}


//8
void *deserializar_rspta_solicitud_instancia_recurso(char *buffer){
	t_rspta_solicitud_instancia_recurso *rpta_instancia = malloc(sizeof(t_rspta_solicitud_instancia_recurso));
	memcpy(&rpta_instancia->concedido,buffer,sizeof(char));
	return rpta_instancia;
}


//9
void *deserializar_turno_concluido(char *buffer){
	int tmp = 0,offset = 0;
	t_turno_concluido *turno_fin = malloc(sizeof(t_turno_concluido));

	memcpy(&turno_fin->bloqueado, buffer, tmp = sizeof(char));
	offset += tmp;
	memcpy(&turno_fin->recurso_de_bloqueo, buffer + offset, sizeof(char));

	return turno_fin;
}


//10
void *deserializar_notificacion_nivel_cumplido(char *buffer){
	t_notificacion_nivel_cumplido *cumplio = malloc(sizeof(t_notificacion_nivel_cumplido));
	memcpy(&cumplio->char_personaje,buffer,sizeof(char));
	return cumplio;
}


//11
void *deserializar_notificacion_muerte_personaje(char *buffer){return NULL;}


//13
void *deserializar_notif_recursos_liberados(char *buffer){
	t_notif_recursos_liberados * rec_liberados = malloc(sizeof(t_notif_recursos_liberados));
	rec_liberados->recursos_liberados = strdup(buffer);
	return rec_liberados;
}


//14
void *deserializar_notif_recursos_reasignados(char *buffer){
	t_notif_recursos_reasignados *reasignados = malloc(sizeof(t_notif_recursos_reasignados));

	reasignados->asignaciones = strdup(buffer);
	reasignados->remanentes = strdup(( buffer+strlen(reasignados->asignaciones)+1 ));

	return reasignados;
}


//15
void *deserializar_solicitud_recupero_deadlock(char *buffer){
	t_solicitud_recupero_deadlock *solicitud = malloc(sizeof(t_solicitud_recupero_deadlock));
	solicitud->pjes_deadlock = strdup(buffer);
	return solicitud;
}


//16
void *deserializar_notif_eleccion_de_victima(char *buffer){
	t_notif_eleccion_de_victima *eleccion = malloc(sizeof(t_notif_eleccion_de_victima));
	memcpy(&eleccion->char_personaje,buffer,sizeof(char));
	return eleccion;
}


//17
void *deserializar_personaje_condenado(char *buffer){
	t_personaje_condenado *per = malloc(sizeof(t_personaje_condenado));
	memcpy(&per->condenado,buffer,sizeof(char));
	return per;
}


//18
void *deserializar_notificacion_plan_terminado(char *buffer){
	t_notificacion_plan_terminado *termino = malloc(sizeof(t_notificacion_plan_terminado));
	termino->char_id = *buffer;
	termino->personaje = strdup(buffer+sizeof(char));
	return termino;
}


//19
void *deserializar_datos_delPersonaje_alPlanificador(char *buffer){
	int offset;
	t_datos_delPersonaje_alPlanificador *datos = malloc(sizeof(t_datos_delPersonaje_alPlanificador));
	memcpy(&datos->char_personaje, buffer, offset = sizeof(char));
	datos->nombre_personaje= strdup((buffer+offset));
	return datos;
}


//20
void *deserializar_ubicacion_de_recurso(char *buffer){
	int tmp,offset = 0;
	t_ubicacion_recurso * ubicacion = malloc(sizeof(t_ubicacion_recurso));

	memcpy(&ubicacion->x,buffer,tmp=sizeof(char));
	offset+=tmp;
	memcpy(&ubicacion->y,buffer+offset,sizeof(char));

	return ubicacion;
}


//21
void *deserializar_datos_delPersonaje_alNivel(char *buffer){
	t_datos_delPersonaje_alNivel *datos = malloc(sizeof(t_datos_delPersonaje_alNivel));

	memcpy(&datos->char_personaje, buffer, sizeof(char));
	datos->nombre_personaje = strdup((buffer+sizeof(char)));
	datos->necesidades = strdup( ( buffer  +  sizeof(char)  +  strlen(datos->nombre_personaje)  +  1 ) );

	return datos;
}


//22
void *deserializar_envio_deDatos_delNivel_alOrquestador(char *buffer){
	t_envio_deDatos_delNivel_alOrquestador *datos = malloc(sizeof(t_envio_deDatos_delNivel_alOrquestador));

	datos->nombre = strdup(buffer);
	datos->recursos_nivel = strdup(buffer+strlen(datos->nombre)+1);
	memcpy(&datos->puerto_nivel, buffer+strlen(datos->nombre)+1+strlen(datos->recursos_nivel)+1, sizeof(uint16_t) );

	return datos;
}


//23
void *deserializar_concesion_recurso(char *buffer){
	t_concesion_recurso *datos = malloc(sizeof(t_concesion_recurso));
	memcpy(&datos->recurso, buffer, sizeof(char));
	return datos;
}



/********************************************** FUNCIONES SERIALIZADORAS *********************************************/
/*  LOS MALLOCS SE LIBERAN EN LA FUNCION ENVIAR, TODOS: LOS DEL BUFFER Y LOS PASADOS COMO STRUCT PORQUE SE USAN AHI  */



//1
char *srlz_solicitud_info_nivel(void *data, int *tamanio){
	int offset;
	t_solicitud_info_nivel *sol = data;
	char *buffer = malloc(*tamanio = strlen(sol->nivel_solicitado)+1 + sizeof(char));

    memcpy(buffer, sol->nivel_solicitado, offset = strlen(sol->nivel_solicitado)+1 );
    memcpy(buffer+offset, &sol->solicitor, sizeof(char));

    free(sol->nivel_solicitado);
	free(data);
	return buffer;
}


//2
char *srlz_info_nivel_y_planificador(void *data, int *tamanio){
	int tmp =0,offset=0;
	t_info_nivel_planificador *d = data;
	char *buffer = malloc(*tamanio = sizeof(uint16_t) + sizeof(uint16_t) + strlen(d->ip_nivel)+1 );

	memcpy(buffer, &d->puerto_nivel,tmp=sizeof(uint16_t));
    offset+=tmp;
    memcpy(buffer+offset, &d->puerto_planificador,tmp=sizeof(uint16_t));
    offset+=tmp;
    memcpy(buffer+offset, d->ip_nivel, tmp = strlen(d->ip_nivel)+1);

    free(d->ip_nivel);
    free(d);
	return buffer;
}


//3
char *srlz_movimiento_permitido(void* data, int *tamanio){
	char *buffer = malloc(sizeof(t_mov_permitido));
	memcpy(buffer, data, *tamanio = sizeof(t_mov_permitido));
	free(data);
	return buffer;
}


//4
char *srlz_solicitud_de_movimiento(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_solicitud_movimiento));
	memcpy(buffer, data, *tamanio = sizeof(t_solicitud_movimiento));
	free(data);
	return buffer;
}


//5
char *srlz_resp_a_solicitud_movimiento(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_resp_solicitud_movimiento));
	memcpy(buffer, data, *tamanio = sizeof(t_resp_solicitud_movimiento));
	free(data);
	return buffer;
}


//6
char *srlz_solicitud_ubicacion_recurso(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_solicitud_ubicacion_recurso));
	memcpy(buffer, data, *tamanio = sizeof(t_solicitud_ubicacion_recurso));
	free(data);
	return buffer;
}


//7
char *srlz_solicitud_instancia_recurso(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_solcitud_instancia_recurso));
	memcpy(buffer, data, *tamanio = sizeof(t_solcitud_instancia_recurso));
	free(data);
	return buffer;
}


//8
char *srlz_rspta_solicitud_instancia_recurso(void *data,int *tamanio){
	char* buffer = malloc(sizeof(t_rspta_solicitud_instancia_recurso));
	memcpy(buffer,data,*tamanio=sizeof(t_rspta_solicitud_instancia_recurso));
    free(data);
    return buffer;
}


//9
char *srlz_turno_concluido(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_turno_concluido));
	memcpy(buffer, data, *tamanio = sizeof(t_turno_concluido));
	free(data);
	return buffer;
}


//10
char *srlz_notificacion_nivel_cumplido(void *data,int *tamanio){
	char *buffer = malloc(sizeof(t_notificacion_nivel_cumplido));
	memcpy(buffer,data, *tamanio = sizeof(t_notificacion_nivel_cumplido));
	free(data);
	return buffer;
}


//11
char *srlz_notificacion_muerte_personaje(void *data,int *tamanio){return NULL;}


//13
char *srlz_notif_recursos_liberados(void *data, int *tamanio){
	t_notif_recursos_liberados *d = data;
	char *buffer = malloc( *tamanio = strlen(d->recursos_liberados) + 1 );
	memcpy(buffer, d->recursos_liberados, *tamanio);
	free(d->recursos_liberados);
	free(data);
	return buffer;
}


//14
char *srlz_notif_recursos_reasignados(void *data, int *tamanio){
	int offset;
	t_notif_recursos_reasignados *d = data;
	char *buffer = malloc( *tamanio = strlen(d->asignaciones)+1  +  strlen(d->remanentes)+1 );

	memcpy(buffer, d->asignaciones, offset = strlen(d->asignaciones)+1 );
	memcpy(buffer+offset, d->remanentes, strlen(d->remanentes)+1 );

	free(d->asignaciones);
	free(d->remanentes);
	free(d);
	return buffer;
}


//15
char *srlz_solicitud_recupero_deadlock(void *data, int *tamanio){
	t_solicitud_recupero_deadlock *d = data;
	char *buffer = malloc(*tamanio = strlen(d->pjes_deadlock)+1);
	memcpy(buffer, d->pjes_deadlock, strlen(d->pjes_deadlock)+1);
	free(d->pjes_deadlock);
	free(d);
	return buffer;
}


//16
char *srlz_notif_eleccion_de_victima(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_notif_eleccion_de_victima));
	memcpy(buffer, data, *tamanio = sizeof(t_notif_eleccion_de_victima));
	free(data);
	return buffer;
}


//17
char *srlz_personaje_condenado(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_personaje_condenado));
	memcpy(buffer,data, *tamanio = sizeof(t_personaje_condenado));
	free(data);
	return buffer;
}


//18
char *srlz_notificacion_plan_terminado(void *data,int *tamanio){
	t_notificacion_plan_terminado *d = data;
	char *buffer = malloc( *tamanio = sizeof(char) + strlen(d->personaje)+1 );

	memcpy(buffer, &d->char_id, sizeof(char));
	memcpy(buffer+sizeof(char), d->personaje, strlen(d->personaje)+1 );

	free(d->personaje);
	free(d);
	return buffer;
}


//19
char *srlz_datos_delPersonaje_alPlanificador(void *data, int *tamanio){
	int offset;
	t_datos_delPersonaje_alPlanificador *d = data;
	char *buffer = malloc( sizeof(char) + strlen(d->nombre_personaje) + 1 );

	memcpy(buffer, &d->char_personaje, offset = sizeof(char));
	memcpy(buffer+offset, d->nombre_personaje, *tamanio = 1 + strlen(d->nombre_personaje));
	*tamanio += offset;

	free(d->nombre_personaje);
	free(data);
	return buffer;
}


//20
char *srlz_ubicacion_de_recurso(void *data, int *tamanio){
	char *buffer = malloc(sizeof(t_ubicacion_recurso));
	memcpy(buffer, data, *tamanio=sizeof(t_ubicacion_recurso));
	free(data);
	return buffer;
}


//21
char *srlz_datos_delPersonaje_alNivel(void *data, int *tamanio){
	int tmp, offset = 0;
	t_datos_delPersonaje_alNivel *d = data;
	char *buffer = malloc( *tamanio = sizeof(char)  +  strlen(d->necesidades)+1  +  strlen(d->nombre_personaje)+1 );

	memcpy(buffer, &d->char_personaje, tmp = sizeof(char));
	offset += tmp;
	memcpy(buffer+offset, d->nombre_personaje, tmp = strlen(d->nombre_personaje)+1);
	offset += tmp;
	memcpy(buffer+offset, d->necesidades, strlen(d->necesidades)+1);

	free(d->necesidades);
	free(d->nombre_personaje);
	free(d);
	return buffer;
}


//22
char *srlz_envio_deDatos_delNivel_alOrquestador(void *data, int *tamanio){
	int tmp, offset = 0;
	t_envio_deDatos_delNivel_alOrquestador *d = data;
	char *buffer = malloc(*tamanio = sizeof(uint16_t) + strlen(d->nombre)+1 + strlen(d->recursos_nivel)+1);

	memcpy(buffer, d->nombre, tmp = strlen(d->nombre)+1);
	offset += tmp;
	memcpy(buffer+offset, d->recursos_nivel, tmp = strlen(d->recursos_nivel)+1);
	offset += tmp;
	memcpy(buffer+offset, &d->puerto_nivel, sizeof(uint16_t));

	free(d->nombre);
	free(d->recursos_nivel);
	free(d);
	return buffer;
}


//23
char *srlz_concesion_recurso(void *data, int *tamanio){
	char* buffer = malloc(sizeof(t_concesion_recurso));
	memcpy(buffer,data,*tamanio=sizeof(t_concesion_recurso));
    free(data);
    return buffer;
}



/********************************************** FUNCIONES DE SOCKET **************************************************/



//recibe un socket y devuelve su IP como un string
char *get_ip_string(int socket){
	struct sockaddr_storage addr;
	socklen_t len = sizeof addr;
	char *IP = malloc(INET_ADDRSTRLEN);

	getpeername(socket, (struct sockaddr*)&addr, &len);

	struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	inet_ntop(AF_INET, &s->sin_addr, IP, INET_ADDRSTRLEN);

	return IP;
}


//devuelve un socket para comunicacion listo para usar, escribiendo en el logger pasado y terminando el programa ante errores
int init_socket_externo(int puerto, char *direccion, t_log *logger)
{
	int unSocket;
	struct linger lo = { 1, 0 };
	struct sockaddr_in socketInfo;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		if(logger!=NULL) log_error(logger, "Error creando socket", "ERROR");
		exit(1);
	}
	fcntl(unSocket, F_SETFL, FD_CLOEXEC);

	setsockopt(unSocket, SOL_SOCKET, SO_LINGER, &lo, sizeof(lo));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(direccion);
	socketInfo.sin_port = htons(puerto);

	connect(unSocket, (struct sockaddr*) &socketInfo, sizeof(socketInfo));
	if(logger!=NULL) log_info(logger, string_from_format("Conectando con la direccion %s", direccion), "INFO");
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
		if(logger!=NULL) log_error(logger, "Error creando socket", "ERROR");
		exit(1);
	}
	fcntl(socketEscucha, F_SETFL, FD_CLOEXEC);

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(puerto);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))	!= 0) {
		if(logger!=NULL) log_error(logger, "Error bindeando socket", "ERROR");
		exit(1);
	}
	listen(socketEscucha, 10);
	if(logger!=NULL)log_info(logger, string_from_format("Se comienza a escuchar por el puerto %d", ntohs(socketInfo.sin_port)),"INFO");
	return socketEscucha;
}

//cuidado con esta funcion: solo usenla cuando saben que tienen que recibir algo (por ejemplo en select)
bool is_connected(int socket){
	char buffer[1];
	return recv(socket, buffer, sizeof buffer, MSG_PEEK)>0;
}
