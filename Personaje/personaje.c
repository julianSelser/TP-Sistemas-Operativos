/*
 * personaje_trucho.c
 *
 *  Created on: 29/06/2013
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

int llego();


int main(int argc, char **argv) {

	iniciar_serializadora();
	t_config * configuracion;
	t_notificacion_plan_terminado * notificacion_plan_terminado;
	notificacion_plan_terminado=malloc(sizeof(t_notificacion_plan_terminado));

	char * log_name;
    int socket_orquestador;

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
		return -1;  // esto me saca inmediatamente del main ?
	}

	configuracion = config_create(argv[1]);

	if (!conf_es_valida(configuracion)) //ver que el archivo de config tenga todito
	{
	puts("Archivo de configuración incompleto o inválido.\n");
	return -2;
	}

	temp_ip_puerto_orq = config_get_string_value(configuracion, "orquestador");
	ip_puerto_orquestador = malloc(strlen(temp_ip_puerto_orq)+1);
	strcpy(ip_puerto_orquestador, temp_ip_puerto_orq); //guardo el orquestador aca
	ip_puerto_separados = string_split(ip_puerto_orquestador, ":");
	free(ip_puerto_orquestador); // ya no los uso
	free(temp_ip_puerto_orq);

	ip_orquestador = malloc(strlen(ip_puerto_separados[0]+1)); // aca el +1 tiene que estar afuera
	strcpy(ip_orquestador, ip_puerto_separados[0]);
	puerto_orquestador = atoi(ip_puerto_separados[1]);
	free(ip_puerto_separados);

	temp_nombre = config_get_string_value(configuracion, "nombre");
	nombre = malloc(strlen(temp_nombre)+1);
	strcpy(nombre,temp_nombre);
	free(temp_nombre);

	contador_vidas = max_vidas = config_get_int_value(configuracion, "vidas");
	simbolo = (config_get_string_value(configuracion,"simbolo"))[0];

	temp_plan_niveles = config_get_string_value(configuracion, "planDeNiveles");
	cantidad_niveles = string_count(temp_plan_niveles, ',') + 1;
	plan_de_niveles = string_split(temp_plan_niveles, ",");
	free(temp_plan_niveles);

	i=0;

	recursos_por_nivel = malloc(cantidad_niveles * sizeof(char *)); // conozco e el tamaño de char*?

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
				string_append(&string_recursos, rec); // queda una cadena con todos los recursos de cada nivel
			}
			pos++;
		}
		free(temp_recursos);
		recursos_por_nivel[i] = malloc(strlen(string_recursos)+1);
		strcpy(recursos_por_nivel[i], string_recursos); // queda vector de todos los niveles en los q en cada
		// posicion hay una cadena con todos los recursos de ese nivel .
		free(string_recursos);
		i++;
	}


	log_name = malloc(strlen(nombre)+1);
	strcpy(log_name, nombre);
	string_append(&log_name,".log"); // queda : nombre.log

	logger = log_create(log_name, "PERSONAJE", 0, LOG_LEVEL_TRACE);
    //hacer free a log_name????????
	niveles_completados=0;

	while (!(cantidad_niveles==niveles_completados)){

	char * nivel_a_pedir;

	t_solicitud_info_nivel *solicitud_info_nivel;
	solicitud_info_nivel=malloc(sizeof(t_solicitud_info_nivel));
	t_info_nivel_planificador * info_nivel_y_planificador;
	info_nivel_y_planificador=malloc(sizeof(t_info_nivel_planificador));
	t_notificacion_nivel_cumplido * notificacion_nivel_cumplido;
	notificacion_nivel_cumplido=malloc(sizeof(t_notificacion_nivel_cumplido));
	t_datos_delPersonaje_alPlanificador * datos_personaje_planificador;
	datos_personaje_planificador=malloc(sizeof(t_datos_delPersonaje_alPlanificador));
	t_datos_delPersonaje_alNivel *datos_personaje_nivel;
	datos_personaje_nivel=malloc(sizeof(t_datos_delPersonaje_alNivel));

    int socket_nivel;
	int socket_planificador;
	int recursos_obtenidos;
    int posicion[2] = {0,0};
	int destino[2];  //por ahora comentado porque no se usa y tira warning
	int sabe_donde_ir;

	nivel_a_pedir = strdup(plan_de_niveles[niveles_completados]);

	solicitud_info_nivel->nivel_solicitado=(uint8_t *)strdup(nivel_a_pedir);
	log_info(logger, string_from_format("Proximo Nivel:%s",nivel_a_pedir), "INFO");

	socket_orquestador = init_socket_externo(socket_orquestador, ip_orquestador, logger);
	log_debug(logger, "Conexión con hilo orquestador establecida", "DEBUG");		// 1111111111111111111

	enviar(socket_orquestador,SOLICITUD_INFO_NIVEL,solicitud_info_nivel,logger);

	info_nivel_y_planificador = recibir(socket_orquestador, INFO_NIVEL_Y_PLANIFICADOR);  //22222222222222222
//              info_nivel_y_planificador->ip_nivel=(uint8_t*)"127.0.0.1";
//              info_nivel_y_planificador->ip_planificador=(uint8_t*)"127.0.0.1";
//              info_nivel_y_planificador->puerto_nivel=5400;
//              info_nivel_y_planificador->puerto_planificador=5600;
    log_debug(logger, "Recibida la información del nivel y el planificador", "DEBUG");
    close(socket_orquestador);
	log_debug(logger, "Desconectado del hilo orquestador", "DEBUG");

	socket_nivel = init_socket_externo(info_nivel_y_planificador->puerto_nivel, (char *)info_nivel_y_planificador->ip_nivel, logger);
	log_info(logger, "Entrando al nivel...", "INFO");									//333333333333333333

	socket_planificador = init_socket_externo(info_nivel_y_planificador->puerto_planificador,(char *) info_nivel_y_planificador->ip_planificador, logger);
	log_debug(logger, "Conectado al hilo planificador del nivel", "DEBUG");             //3333333333333333333

			datos_personaje_planificador->char_personaje=simbolo;
	        datos_personaje_planificador->nombre_personaje=(uint8_t*)strdup(nombre);

	       enviar(socket_planificador,ENVIO_DE_DATOS_AL_PLANIFICADOR,datos_personaje_planificador,logger);
	       	   	   	datos_personaje_nivel->char_personaje=simbolo;
	       	        datos_personaje_nivel->nombre_personaje=(uint8_t*) strdup(nombre);
	       	        datos_personaje_nivel->necesidades=(uint8_t*)strdup(recursos_por_nivel[niveles_completados]);
	       enviar(socket_nivel,ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL,datos_personaje_nivel,logger);


     sabe_donde_ir = 0; //booleano que representa si el personaje tiene un destino válido o no.
     	 	 	 	 	//Se pone en Falso al entrar a un nivel
	 recursos_obtenidos = 0;

	 while(1){

		int  consiguio_total_recursos=0;
		uint8_t mje_a_recibir = 17;
        uint8_t  proximo_recurso;

        t_solicitud_movimiento *solicitud_movimiento;
        solicitud_movimiento=malloc(sizeof(t_solicitud_movimiento));
        t_resp_solicitud_movimiento * rspt_solicitud_movimiento;
       // rspt_solicitud_movimiento=malloc(sizeof(t_resp_solicitud_movimiento));
        t_turno_concluido * turno_concluido;
        turno_concluido=malloc(sizeof(t_turno_concluido));

	recibir(socket_planificador, NOTIF_MOVIMIENTO_PERMITIDO);     //333333333333333333333333333333
		//el propósito de este "recibir" es puramente que el personaje se bloquee
		//no necesita ninguna información. De hecho
		//el mensaje podría ser solamente el header y nada de datos.
		//recibir este mensaje significa que es el turno del personaje

		if(!sabe_donde_ir)
		{
			t_solicitud_ubicacion_recurso * sol_ubicacion_recurso;
			sol_ubicacion_recurso=malloc(sizeof(t_solicitud_ubicacion_recurso));
			t_ubicacion_recurso * ubicacion_recursos;
			ubicacion_recursos=malloc(sizeof(t_ubicacion_recurso));

			//ACCION: AVERIGUAR CUÁL ES EL PROXIMO RECURSO A OBTENER
           proximo_recurso = recursos_por_nivel[niveles_completados][recursos_obtenidos];

           //ACCION: ELABORAR SOLICITUD DE UBICACION DEL PROXIMO RECURSO
          sol_ubicacion_recurso->recurso=proximo_recurso;
          enviar(socket_nivel, SOLICITUD_UBICACION_RECURSO,sol_ubicacion_recurso, logger);
	//ubicacion_recursos = recibir(socket_nivel, INFO_UBICACION_RECURSO);

			destino[0]=ubicacion_recursos->x; //esto podría ser más prolijo si destino lo hicieramos un struct en vez de un vector
			destino[1]=ubicacion_recursos->y ;//idem
			sabe_donde_ir = 1;

		 	//ELABORAR MENSAJE PARA LOG: INFORMAR CUAL ES EL NUEVO DESTINO Y
			//QUE RECURSO ESTAMOS YENDO A BUSCAR


	        log_info(logger,string_from_format("el nuevo destino es (%d,%d) y el recurso es %c ",proximo_recurso), "INFO");
		}

		//CASO EXTREMO A CONSIDERAR: SI EL PERSONAJE, APENAS LLEGA AL NIVEL, YA ESTÁ EN SU PRIMER DESTINO??
		//ACCION: ELABORAR SOLICITUD DE MOVIMIENTO XY
         solicitud_movimiento->char_personaje=simbolo;
         solicitud_movimiento->x=destino[0];
         solicitud_movimiento->y=destino[1];

		enviar(socket_nivel,SOLICITUD_MOVIMIENTO_XY,solicitud_movimiento, 0);

			rspt_solicitud_movimiento=recibir(socket_nivel,RTA_SOLICITUD_MOVIMIENTO_XY);

			if (!rspt_solicitud_movimiento->aprobado) // esto se deberia cargar con el recibir asi q no hace falta inicializ.
			{
				log_error(logger, "Se intentó realizar un movimiento imposible", "ERROR");

			//terminar anormalmente
				printf("SOLICITUD DE MOVIMIENTO DENEGADO");
						}
            memcpy(posicion,destino,sizeof(posicion));

			if(llego(posicion, destino)){ //llego es una funcion que simplemente compara la posicion con el destino, componente a componente

			     t_solcitud_instancia_recurso * solicitud_instancia;
			     solicitud_instancia=malloc(sizeof(t_rspta_solicitud_instancia_recurso));
			     t_rspta_solicitud_instancia_recurso * rpta_solicitud_instancia_recurso;
//			     rpta_solicitud_instancia_recurso=malloc(sizeof(t_rspta_solicitud_instancia_recurso)); no necesita ini.
			     //   rpta_solicitud_instancia_recurso->concedido=0; lo use para probar

			     //ACCION: ELABORAR SOLICITUD INSTANCIA DE RECURSO
			     //ACCION: SERIALIZAR SOLICITUD INSTANCIA DE RECURSO
			solicitud_instancia->instancia_recurso = proximo_recurso;
			enviar(socket_nivel,SOLICITUD_INSTANCIA_RECURSO,solicitud_instancia,logger);

			rpta_solicitud_instancia_recurso = recibir(socket_nivel,RTA_SOLICITUD_INSTANCIA_RECURSO);

			if (rpta_solicitud_instancia_recurso->concedido){

				//ACCION: ACTUALIZAR RECURSOS OBTENIDOS. SI CONSIGUIO EL TOTAL, INDICAR consiguio_total_recursos = 1
			    recursos_obtenidos++;
			   // printf("%d\n",recursos_obtenidos); ..recordar borrar el malloc de rpta_solic.

			    if(recursos_obtenidos == strlen(recursos_por_nivel[niveles_completados])){

			    	consiguio_total_recursos = 1; // para que me sirve esta variable

			    //	una vez que consiguio total de recursos se deberia desconectar del nivel ?
			    		}
				sabe_donde_ir = 0;
				log_info(logger, "Se obtuvo el recurso!", "INFO");

				} else if (!rpta_solicitud_instancia_recurso->concedido) //denegado

				{
					log_info(logger, "El personaje quedó a la espera del recurso", "INFO");
					mje_a_recibir = getnextmsg(socket_planificador);  //este msje me termina abruptamente

					if(mje_a_recibir == NOTIF_PERSONAJE_CONDENADO){

			//recibir(socket_orquestador,NOTIF_PERSONAJE_CONDENADO); // me devuelve el struct t_not_perso_conde
			//pero es lo mismo que la  NOTIF_PERSONAJE_CONDENADO
						log_info(logger, "Este personaje va a morir para solucionar un interbloqueo", "INFO");
						//notificar al nivel que murio el personaje
						//morir(); //morir se encarga de setear game_over si es necesario
						if(contador_vidas>0){
							contador_vidas--;
							recursos_obtenidos=0;
						 //enviar mensaje al orquestador reiniciar nivel
						}else if(contador_vidas==0){
							contador_vidas=config_get_int_value(configuracion,"vidas");
							game_over=1;

						}
						break; //sale del nivel
					}
					//fin else if mje == NOTIF_RECURSO_CONCEDIDO then recursos_obtenidos++ y sabe_donde_ir=0(como arriba)
				}
			}

			//ACCION: ELABORAR NOTIFICACION DE TURNO CONCLUIDO
	turno_concluido->bloqueado=1;
	//falta completar el recurso de bloqueo
	//ACCION: SERIALIZAR NOTIFICACION DE TURNO CONCLUIDO
	enviar(socket_planificador,NOTIF_TURNO_CONCLUIDO,turno_concluido,logger);
			if(recursos_obtenidos == strlen(recursos_por_nivel[niveles_completados]))
			{
				log_info(logger, "Nivel finalizado!", "INFO");
				//ACCION: ELABORAR NOTIFICACION NIVEL CONCLUIDO
				notificacion_nivel_cumplido->char_personaje=simbolo;
                niveles_completados++; //aumentamos los niveles concluidos
				//ACCION: SERIALIZAR NOTIFICACION NIVEL CONCLUIDO
				//ACCION: MARCAR NIVEL COMO CONCLUIDO
 enviar(socket_nivel,NOTIF_NIVEL_CUMPLIDO,notificacion_nivel_cumplido,logger);  // a quien se lo envia realmente
				break;  //por acá se sale del while(1) [personaje sale del nivel]
			}

	 }//fin while(1) [personaje en nivel]

		//en este punto es donde definitivamente se sale de un nivel. esto significa desconectarse del hilo planificador y del nivel

	 //ACCION: DESCONECTAR DEL NIVEL
	 //ACCION: DESCONECTAR DEL HILO PLANIFICADOR DEL NIVEL
		close(socket_nivel);
		close(socket_planificador);
	 //después del fin while(1), el personaje pide info del próximo nivel
	 //si el personaje murió, entonces no marcó el nivel como concludio, y va a pedir la info del nivel en el que estaba. esto es dudoso, ya que la consigna dice que "notifica su intención de reiniciar el nivel"

		if(game_over) //a menos que el personaje haya perdido todas sus vidas
	 	{
				//REINICIAR PLAN DE NIVELES
		         niveles_completados=0;
		         log_info(logger, "Game over - reiniciando plan de niveles","INFO");
	 	}


	} //fin plan_de_niveles==niveles_completados


	//el personaje, al terminar su plan de niveles, se conecta al hilo orquestador y se lo notifica

	//ACCION: CONECTAR CON EL HILO ORQUESTADOR
   socket_orquestador=init_socket_externo(puerto_orquestador,ip_puerto_orquestador,logger);
   log_debug(logger, "Conexión con hilo orquestador establecida", "DEBUG");

	//ELABORAR NOTIFICACION DE PLAN TERMINADO
    notificacion_plan_terminado->personaje=(uint8_t *)strdup(nombre);  // revisar q le voy a mandar al orquestador
	enviar(socket_orquestador,NOTIF_PLAN_TERMINADO,notificacion_plan_terminado,logger);

//	while(1); //y queda a la espera indefinidamente? no debería terminar el proceso cuando termina el plan de niveles, así que supongo que hay que dejarlo ahí
//  para evitar el warning, igual no se si tienen que ser tipo int o si pueden ser tipo void nuestros main

	printf("%s\n",recursos_por_nivel[3]);

return 0;

}

int conf_es_valida(t_config * configuracion)
{
	return( config_has_property(configuracion, "nombre") &&
		  config_has_property(configuracion, "simbolo") &&
		  config_has_property(configuracion, "planDeNiveles") &&
		  config_has_property(configuracion, "vidas") &&
		  config_has_property(configuracion, "orquestador"));
}


int llego(int pos[],int dest[]){
	//true si las componentes son iguales
    return pos[0]==dest[0] && pos[1]==dest[1];
}

