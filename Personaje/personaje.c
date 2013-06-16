/*
 * personaje.c
 *
 *  Created on: 18/05/2013
 *      Author: utnso
 */


#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

#include <serial.h>

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

t_log * logger;



char * nombre;
char simbolo;
char * ip_orquestador;
int puerto_orquestador;
int max_vidas;


int game_over = 0;
int contador_vidas;

int conf_es_valida(t_config * configuracion);
void morir();


int main(int argc, char** argv)
{
	t_config * configuracion;
	char * log_name;

	char * ip_puerto_orquestador;
	char * temp_ip_puerto_orq;
	char ** ip_puerto_separados;
	char * temp_nombre;
	char * temp_plan_niveles;
	

	char ** recursos_por_nivel; //vectores paralelos
	char ** plan_de_niveles;  //vectores paralelos
	int cantidad_niveles;
	int niveles_completados;
	int i;

	if(argc != 2) //controlar que haya exactamente un parámetro
	{
		puts("Uso: personaje <arch.conf>\n");
		return -1;
	}

	//configuracion = config_create(argv); //qué devuelve en caso de error?
	configuracion = config_create(argv[1]);

	if (!conf_es_valida(configuracion)) //ver que el archivo de config tenga todito
	{
	puts("Archivo de configuración incompleto o inválido.\n");
	return -2;
	}

	
	temp_ip_puerto_orq = config_get_string_value(configuracion, "orquestador");
	ip_puerto_orquestador = malloc(strlen(temp_ip_puerto_orq)+1);
	strcpy(ip_puerto_orquestador, temp_ip_puerto_orq);
	ip_puerto_separados = string_split(ip_puerto_orquestador, ":");
	free(ip_puerto_orquestador);
	free(temp_ip_puerto_orq);
	
	ip_orquestador = malloc(strlen(ip_puerto_separados[0]+1));
	strcpy(ip_orquestador, ip_puerto_separados[0]);
	puerto_orquestador = atoi(ip_puerto_separados[1]);
	free(ip_puerto_separados);

	temp_nombre = config_get_string_value(configuracion, "nombre");
	nombre = malloc(strlen(temp_nombre)+1);
	strcpy(nombre, temp_nombre);
	free(temp_nombre);
	
	contador_vidas = max_vidas = config_get_int_value(configuracion, "vidas");
	simbolo = (config_get_string_value(configuracion, "simbolo"))[0];
	
	temp_plan_niveles = config_get_string_value(configuracion, "planDeNiveles");
	cantidad_niveles = string_count(temp_plan_niveles, ',') + 1;
	plan_de_niveles = string_split(temp_plan_niveles, ",");
	free(temp_plan_niveles);

	i=0;

	recursos_por_nivel = malloc(cantidad_niveles * sizeof(char *));

	while(i<cantidad_niveles)
	{
		char * clave;
		char * temp_recursos;
		char * string_recursos;
		int pos;

		clave = malloc(1);
		clave[0] = '\0';
		string_append(&clave, "obj[" );
		string_append(&clave, plan_de_niveles[i]);
		string_append(&clave, "]");

		temp_recursos = config_get_string_value(configuracion, clave);
		string_recursos = malloc(1);
		string_recursos[0]='\0';

		pos = 0;
		while(temp_recursos[pos]!=']')
		{
			char rec[2];
			rec[0] = temp_recursos[pos];
			rec[1] = '\0';

			if(rec[0]>='A' && rec[0]<='Z')
			{
				string_append(&string_recursos, rec);
			}
			pos++;
		}
		free(temp_recursos);
		recursos_por_nivel[i] = malloc(strlen(string_recursos)+1);
		strcpy(recursos_por_nivel[i], string_recursos);
		free(string_recursos);
		i++;
	}


	log_name = malloc(strlen(nombre)+1);
	strcpy(log_name, nombre);
	string_append(&log_name, ".log");

	logger = log_create(log_name, "PERSONAJE", 0, LOG_LEVEL_TRACE);
	//se crea una instancia del logger
	//se loguea sobre <personaje>.log

	//ACCION: ESTABLECER HANDLERS DE SEÑALES
	//log_debug(logger_personaje, "Handlers de señales establecidos", "DEBUG");



	niveles_completados=0;
	while (!(cantidad_niveles==niveles_completados))
	{
		static int socket_orquestador;
		char * nivel_a_pedir;
		t_info_nivel_planificador * info_nivel_y_planificador;
		int socket_nivel;
		int socket_planificador;
		int recursos_obtenidos;

		//int destino[2]; por ahora comentado porque no se usa y tira warning
		int sabe_donde_ir;

		nivel_a_pedir = plan_de_niveles[niveles_completados];

		log_info(logger, strcat("Próximo nivel", nivel_a_pedir), "INFO");


		socket_orquestador = init_socket_externo(puerto_orquestador, ip_orquestador, logger);
		log_debug(logger, "Conexión con hilo orquestador establecida", "DEBUG");

		enviar(socket_orquestador, SOLICITUD_INFO_NIVEL, nivel_a_pedir, logger);
		info_nivel_y_planificador = recibir(socket_orquestador, INFO_NIVEL_Y_PLANIFICADOR);
		log_debug(logger, "Recibida la información del nivel y el planificador", "DEBUG");

		close(socket_orquestador);
		log_debug(logger, "Desconectado del hilo orquestador", "DEBUG");

		socket_nivel = init_socket_externo(info_nivel_y_planificador->puerto_nivel, info_nivel_y_planificador->ip_nivel, logger);
		log_info(logger, "Entrando al nivel...", "INFO");

		socket_planificador = init_socket_externo(info_nivel_y_planificador->puerto_planificador, info_nivel_y_planificador->ip_planificador, logger);
		log_debug(logger, "Conectado al hilo planificador del nivel", "DEBUG");



		sabe_donde_ir = 0; //booleano que representa si el personaje tiene un destino válido o no. Se pone en Falso al entrar a un nivel
		recursos_obtenidos = 0;

		while(1)
		{
			uint8_t mje_a_recibir = 255;


			recibir(socket_planificador, NOTIF_MOVIMIENTO_PERMITIDO);
			//el propósito de este "recibir" es puramente que el personaje se bloquee, no necesita ninguna información. De hecho, el mensaje podría ser solamente el header y nada de datos.
			//recibir este mensaje significa que es el turno del personaje

			if(!sabe_donde_ir)
			{
				//t_solicitud_ubicacion_recurso sol_ubicacion_recurso;
				//t_ubicacion_recurso * ubicacion_recursos;

				//ACCION: AVERIGUAR CUÁL ES EL PROXIMO RECURSO A OBTENER
				//el proximo recurso a pedir es recursos_por_nivel[niveles_completados][recursos_obtenidos]
				//ACCION: ELABORAR SOLICITUD DE UBICACION DEL PROXIMO RECURSO

				//enviar(socket_nivel, SOLICITUD_UBICACION_RECURSO, &sol_ubicacion_recurso, logger);
				//ubicacion_recursos = recibir(socket_nivel, INFO_UBICACION_RECURSO);

				//destino[0]=ubicacion_recursos.x //esto podría ser más prolijo si destino lo hicieramos un struct en vez de un vector
				//destino[1]=ubicacion_recursos.y //idem
				//sabe_donde_ir = 1;

				//ELABORAR MENSAJE PARA LOG: INFORMAR CUAL ES EL NUEVO DESTINO Y QUE RECURSO ESTAMOS YENDO A BUSCAR
				//log_info(logger_personaje, mensaje_elaborado_recien, "INFO");
			}

			//CASO EXTREMO A CONSIDERAR: SI EL PERSONAJE, APENAS LLEGA AL NIVEL, YA ESTÁ EN SU PRIMER DESTINO??

			//ACCION: ELABORAR SOLICITUD DE MOVIMIENTO XY
			//ACCION: SERIALIZAR SOLICITUD DE MOVIMIENTO XY

			//send(socket_nivel, msj_solicitud_movimiento_xy, longitud, 0);
			//recibir(socket,nivel, ID_RTA_SOLICITUD_MOVIMIENTO_XY)

			//if (!RTA_SOLICITUD_MOVIMIENTO_XY.aprobado)
			//{
			//	log_error(logger_personaje, "Se intentó realizar un movimiento imposible", "ERROR");
			//	//terminar anormalmente
			//}

			//ACCION: MOVERSE (ACTUALIZAR POSICION DEL PERSONAJE)


			//if(llego(posicion, destino)) //llego es una funcion que simplemente compara la posicion con el destino, componente a componente
			//{
				//ACCION: ELABORAR SOLICITUD INSTANCIA DE RECURSO
				//ACCION: SERIALIZAR SOLICITUD INSTANCIA DE RECURSO

				//send(socket_nivel, msj_solicitud_instancia_recurso, longitud, 0);

				//rta_solicitud_instancia_recurso = recibir(socket_nivel, ID_RTA_SOLICITUD_INSTANCIA_RECURSO);

				//if (rta_solicitud_instancia_recurso.concedido)
				//{
						//ACCION: ACTUALIZAR RECURSOS OBTENIDOS. SI CONSIGUIO EL TOTAL, INDICAR consiguio_total_recursos = 1
						//sabe_donde_ir = 0;
						//log_info(logger, "Se obtuvo el recurso!", "INFO);
				//}

				//else if (!rta_solicitud_instancia_recurso.concedido)
				//{


					//log_info(logger_personaje, "El personaje quedó a la espera del recurso", INFO);
					//
					//mje_a_recibir = getnextmsg(socket_planificador);
					//
					//
					//if(mje_a_recibir == NOTIF_PERSONAJE_CONDENADO) //NOTIF_PERSONAJE_CONDENADO
					//	{
					//		recibir(socket_planificador, NOTIF_PERSONAJE_CONDENADO);
					//		log_info(logger, "Este personaje va a morir para solucionar un interbloqueo", "INFO");
					//		morir(); //morir se encarga de setear game_over si es necesario
					//		break; //sale del nivel
					//	}
					//else if mje == NOTIF_RECURSO_CONCEDIDO then recursos_obtenidos++ y sabe_donde_ir=0(como arriba)
				//}

			//}

			//ACCION: ELABORAR NOTIFICACION DE TURNO CONCLUIDO
			//ACCION: SERIALIZAR NOTIFICACION DE TURNO CONCLUIDO

			//send(socket_planificador, msj_notif_turno_concluido, longitud, 0);

			if(recursos_obtenidos == strlen(recursos_por_nivel[niveles_completados]))
			{
				//log_info(logger_personaje, "Nivel finalizado!", "INFO");
				//ACCION: ELABORAR NOTIFICACION NIVEL CONCLUIDO
				//ACCION: SERIALIZAR NOTIFICACION NIVEL CONCLUIDO
				//ACCION: MARCAR NIVEL COMO CONCLUIDO
				//send(socket_nivel, msj_notif_nivel_concluido, longitud, 0);

				break; //por acá se sale del while(1) [personaje sale del nivel]
			}

		} //fin while(1) [personaje en nivel]
		//en este punto es donde definitivamente se sale de un nivel. esto significa desconectarse del hilo planificador y del nivel

		//ACCION: DESCONECTAR DEL NIVEL
		//ACCION: DESCONECTAR DEL HILO PLANIFICADOR DEL NIVEL


		//después del fin while(1), el personaje pide info del próximo nivel
		//si el personaje murió, entonces no marcó el nivel como concludio, y va a pedir la info del nivel en el que estaba. esto es dudoso, ya que la consigna dice que "notifica su intención de reiniciar el nivel"
		if(game_over) //a menos que el personaje haya perdido todas sus vidas
		{
			//REINICIAR PLAN DE NIVELES
			//log_info(logger_personaje, "Game over - reiniciando plan de niveles". "INFO");
		}
	}

	//el personaje, al terminar su plan de niveles, se conecta al hilo orquestador y se lo notifica

	//ACCION: CONECTAR CON EL HILO ORQUESTADOR
	//log_debug(logger_personaje, "Conexión con hilo orquestador establecida", "DEBUG");

	//ELABORAR NOTIFICACION DE PLAN TERMINADO
	//SERIALIZAR NOTIFICACION DE PLAN TERMINADO
	//send(socket_orquestador, msj_notif_plan_terminado, longitud, 0);

	while(1); //y queda a la espera indefinidamente? no debería terminar el proceso cuando termina el plan de niveles, así que supongo que hay que dejarlo ahí
	return 0; //para evitar el warning, igual no se si tienen que ser tipo int o si pueden ser tipo void nuestros main
}

void morir()
{
	//ELABORAR NOTIFICACION DE MUERTE PERSONAJE
	//SERIALIZAR NOTFICACION DE MUERTE PERSONAJE

	//send(socket_nivel, ID_NOTIF_MUERTE_PERSONAJE);

	if(contador_vidas > 0) contador_vidas--;

	else
	{
		//VOLVER VIDAS AL VALOR INICIAL LEIDO EN EL ARCHIVO DE CONFIG
		//REINICIAR PLAN DE NIVELES
		game_over = 1;
	}
}

int conf_es_valida(t_config * configuracion)
{
	return( config_has_property(configuracion, "nombre") &&
		  config_has_property(configuracion, "simbolo") &&
		  config_has_property(configuracion, "planDeNiveles") &&
		  config_has_property(configuracion, "vidas") &&
		  config_has_property(configuracion, "orquestador"));
}
