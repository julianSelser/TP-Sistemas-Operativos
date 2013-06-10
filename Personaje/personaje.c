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


#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

t_log * logger;
t_config * configuracion;
int termino_plan_niveles;
int game_over = 0;
int contador_vidas;


int main(int argc, char** argv)
{
	if(argc != 2) //controlar que haya exactamente un parámetro
	{
		puts("Uso: personaje <arch.conf>\n");
		return -1;
	}

	//configuracion = config_create(argv); //qué devuelve en caso de error?
	t_config * configuracion = config_create(argv[1]);


	if(!( config_has_property(configuracion, "nombre") &&
		  config_has_property(configuracion, "simbolo") &&
		  config_has_property(configuracion, "planDeNiveles") &&
		  config_has_property(configuracion, "vidas") &&
		  config_has_property(configuracion, "ip_orquestador")&&
		  config_has_property(configuracion, "puerto_orquestador")
		)) //si no están todos los campos necesarios de la configuración
	{
		puts("Archivo de configuración incompleto o inválido\n");
		return -2;
	}


	  typedef struct {

		  char *nombre;
		  char *simbolo;
		  char ** plan_de_niveles ;
		  int  vidas;
		  char * ip_orquestador ;
		  int puerto_orquestador;

	  } arch_personaje;

	  arch_personaje * personaje = malloc(sizeof(arch_personaje));
	     personaje->simbolo=config_get_string_value(configuracion,"simbolo");
	     personaje->nombre=config_get_string_value(configuracion,"nombre");
	     personaje->ip_orquestador=config_get_string_value(configuracion,"ip_orquestador");
	     personaje->vidas=config_get_int_value(configuracion,"vidas");
	     personaje->puerto_orquestador=config_get_int_value(configuracion,"puerto_orquestador");
	     personaje->plan_de_niveles=config_get_array_value(configuracion,"planDeNiveles");
  //char ** niveles = personaje->plan_de_niveles;

  contador_vidas=personaje->vidas;
	//ACCION: LEER EL ARCHIVO DE CONFIGURACION

	logger = log_create("personaje.log", "PERSONAJE", 0, LOG_LEVEL_TRACE);
	//se crea una instancia del logger
	//se va a crear un logger por personaje. por lo tanto, deberiamos crear un archivo .log por personaje?
	//crear un solo archivo de log o varios segun el nivel de logueo?
	//mas copado seria que despues de leer el archivo de config, podamos crear el archivo de log segun el nombre del personaje


	//ACCION: ESTABLECER HANDLERS DE SEÑALES
	//log_debug(logger_personaje, "Handlers de señales establecidos", "DEBUG");



	termino_plan_niveles = 0;
	//hay que ver cómo determinamos si el personaje terminó o no su plan, y evaluarlo acá, en vez de inicializar esto en 0
	//int i=0;
	while (!termino_plan_niveles)
	{

		// niveles[i];
		// int nivel_a_pedir;

		//int destino[2]; por ahora comentado porque no se usa y tira warning
		int sabe_donde_ir, consiguio_total_recursos;



		//nivel_a_pedir=niveles[i];

		//ACCION: UBICAR EL PROXIMO NIVEL A PEDIR
		//log_info(logger_personaje, strcat("Próximo nivel", nivel_a_pedir), "INFO");

		//ACCION: CONECTAR CON EL HILO ORQUESTADOR
		//log_debug(logger_personaje, "Conexión con hilo orquestador establecida", "DEBUG");

		  int unSocket;
		  struct sockaddr_in el_orquestador;
		  if((unSocket = socket(AF_INET, SOCK_STREAM,0))==-1){
		          	perror("socket");
		          	exit(1);
		          }

		  	  	  el_orquestador.sin_family=AF_INET;
		          el_orquestador.sin_port= htons(personaje->puerto_orquestador);
		          el_orquestador.sin_addr.s_addr=inet_addr(personaje->ip_orquestador); // aca puede ir inet_addr(DIRECCION)
		         // memset(&(el_orquestador.sin_zero),8);



		          if(connect(unSocket,(struct sockaddr *) & el_orquestador,sizeof(struct sockaddr))==-1){

		         	 perror("connect");
		         	 exit(1);
		          }



		//ACCION: ELABORAR SOLICITUD DE DATOS DE NIVEL
		//ACCION: SERIALIZAR SOLICITUD DE DATOS DE NIVEL

		//send (socket_orquestador, msj_solicitud_datos_nivel, longitud_msj, 0);
		//info_nivel_y_planificador = (t_info_nivel_y_planificador *) recibir(socket_orquestador, ID_INFO_NIVEL_Y_PLANIFICADOR); //ID_INFO_NIVEL Y PLANIFICADOR ES EL ID DEL TIPO DE MENSAJE, SE PUEDE DEFINIR EN UN .h, LO ENVIAMOS A LA FUNCION RECIBIR PARA VALIDAR QUE SE RECIBA LO QUE ESPERAMOS
		//recibir es la función mágica que, dado un socket, devuelve como puntero a void la dirección del struct que armó des-serializando lo que había en el socket
		//log_debug(logger, "Recibida la información del nivel y el planificador", "DEBUG");

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
			uint8_t mje_a_recibir = 255;

			//IMPORTANTE, ACÁ PUEDEN PASAR UNA DE DOS COSAS:
			//1. EL PROCESO ESTÁ LISTO Y RECIBE LA NOTIFICACIÓN DE MOVIMIENTO PERMITIDO
			//2. EL PROCESO ESTÁ BLOQUEADO Y RECIBE LA NOTIFICACIÓN DE PERSONAJE CONDENADO
			//EL PJ VA A PENSAR QUE RECIBE ESTA ÚLTIMA POR PARTE DEL PLANIFICADOR, AUNQUE EN REALIDAD LO ESTÉ ENVIANDO EL ORQUESTADOR


			//mje_a_recibir = getnextmsg(socket_planificador) //getnextmsg es una función que informa qué tipo de mensaje es el próximo que hay en el socket (hace un peek del header). es bloqueante

			if(mje_a_recibir == 17) //NOTIF_PERSONAJE_CONDENADO
			{
				//recibir(socket_planificador, NOTIF_PERSONAJE_CONDENADO);
				//log_info(logger_personaje, "Este personaje va a morir para solucionar un interbloqueo", "INFO");
				//morir() //morir se encarga de setear game_over si es necesario
				break; //sale del nivel
			}

			//recibir(socket_planificador, ID_NOTIF_MOVIMIENTO_PERMITIDO)


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
						//log_info(logger, "Se obtuvo el recurso!", "INFO);
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
