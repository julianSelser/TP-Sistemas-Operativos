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


t_log * logger_personaje;
int termino_plan_niveles;


int main()
{

	logger_personaje = log_create("personaje.log", "PERSONAJE", 0, LOG_LEVEL_TRACE);
	//se crea una instancia del logger
	//se va a crear un logger por personaje. por lo tanto, deberiamos crear un archivo .log por personaje?
	//crear un solo archivo de log o varios segun el nivel de logueo?


	//ACCION: ESTABLECER HANDLERS DE SEÑALES
	//log_debug(logger_personaje, "Handlers de señales establecidos", "DEBUG");


	//ACCION: LEER EL ARCHIVO DE CONFIGURACION
	//log_debug(logger_personaje, "Archivo de configuración leido", "DEBUG")

	termino_plan_niveles = 0;
	//hay que ver cómo determinamos si el personaje terminó o no su plan, y evaluarlo acá, en vez de inicializar esto en 0

	while (!termino_plan_niveles)
	{
		int destino[2];
		int sabe_donde_ir, consiguio_total_recursos;

		//ACCION: UBICAR EL PROXIMO NIVEL A PEDIR
		//log_info(logger_personaje, strcat("Próximo nivel", nivel_a_pedir), "INFO");

		//ACCION: CONECTAR CON EL HILO ORQUESTADOR
		//log_debug(logger_personaje, "Conexión con hilo orquestador establecida", "DEBUG");

		//ACCION: ELABORAR SOLICITUD DE DATOS DE NIVEL
		//ACCION: SERIALIZAR SOLICITUD DE DATOS DE NIVEL

		//send (socket_orquestador, msj_solicitud_datos_nivel, longitud_msj, 0);
		//info_nivel_y_planificador = (t_info_nivel_y_planificador *) recibir(socket_orquestador, ID_INFO_NIVEL_Y_PLANIFICADOR); //ID_INFO_NIVEL Y PLANIFICADOR ES EL ID DEL TIPO DE MENSAJE, SE PUEDE DEFINIR EN UN .h, LO ENVIAMOS A LA FUNCION RECIBIR PARA VALIDAR QUE SE RECIBA LO QUE ESPERAMOS
		//recibir es la función mágica que, dado un socket, devuelve como puntero a void la dirección del struct que armó des-serializando lo que había en el socket
		//log_debug(logger_personaje, "Recibida la información del nivel y el planificador", "DEBUG");

		//ACCION: DESCONECTARSE DEL HILO ORQUESTADOR
		//log_debug(logger_personaje, "Desconectado del hilo orquestador", "DEBUG");

		//ACCION: CONECTAR CON EL NIVEL
		//log_info(logger_personaje, "Entrando al nivel...", "INFO");
		//ACCION: CONECTAR CON EL PLANIFICADOR DEL NIVEL
		//log_debug(logger_personaje, "Conectado al hilo planificador del nivel", "DEBUG);

		sabe_donde_ir = 0; //booleano que representa si el personaje tiene un destino válido o no. Se pone en Falso al entrar a un nivel
		consiguio_total_recursos = 0;

		while(1)
		{
			//recibir(socket_planificador, ID_INFO_NIVEL_Y_PLANIFICADOR)
			//el propósito de este "recibir" es puramente que el personaje se bloquee, no necesita ninguna información. De hecho, el mensaje podría ser solamente el header y nada de datos.
			//recibir este mensaje significa que es el turno del personaje

			if(!sabe_donde_ir)
			{
				//ACCION: AVERIGUAR CUÁL ES EL PROXIMO RECURSO A OBTENER
				//ACCION: ELABORAR SOLICITUD DE UBICACION DEL PROXIMO RECURSO
				//ACCION: SERIALIZAR SOLICITUD DE UBICACION DEL PROXIMO RECURSO

				//send(socket_nivel, msj_solicitud_ubicacion_recurso, longitud_msj, 0);
				//ubicacion_recursos = recibir(socket_nivel, ID_INFO_UBICACION_RECURSOS);

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
						//log_info(logger_personaje, "Se obtuvo el recurso!", "INFO);
				//}

				//else if (!rta_solicitud_instancia_recurso.concedido)
				//{
						//ACCION: BLOQUEARSE (¿listo=0?)
						//log_info(logger_personaje, "El personaje quedó a la espera del recurso", INFO);
				//}

			//}

			//ACCION: ELABORAR NOTIFICACION DE TURNO CONCLUIDO
			//ACCION: SERIALIZAR NOTIFICACION DE TURNO CONCLUIDO

			//send(socket_planificador, msj_notif_turno_concluido, longitud, 0);

			if(consiguio_total_recursos)
			{
				//log_info(logger_personaje, "Nivel finalizado!", "INFO");
				//ACCION: ELABORAR NOTIFICACION NIVEL CONCLUIDO
				//ACCION: SERIALIZAR NOTIFICACION NIVEL CONCLUIDO
				//send(socket_nivel, msj_notif_nivel_concluido, longitud, 0);

				//ACCION: DESCONECTAR DEL NIVEL
				//ACCION: DESCONECTAR DEL HILO PLANIFICADOR DEL NIVEL

				break;
			}

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


