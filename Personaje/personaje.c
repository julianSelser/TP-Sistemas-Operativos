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
#include <ctype.h>

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
int buscar_paso(int * posicion, int * destino, int * prox_paso);

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


	//-------------INICIO LECTURA ARCHIVO CONFIG--------------//

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

	log_name = malloc(strlen(nombre)+1);
	strcpy(log_name, nombre);
	string_append(&log_name,".log"); // queda : nombre.log

	logger = log_create(log_name, "PERSONAJE", 1, LOG_LEVEL_TRACE);
	free(log_name);//hacer free a log_name???????? seh

	i=0;

	recursos_por_nivel = malloc(cantidad_niveles * sizeof(char)); // conozco e el tamaño de char*?

	while(i<cantidad_niveles)
	{
		char * clave;
		char * temp_recursos;
		char * string_recursos;
		int pos;

		clave = string_from_format("obj[%s]", isspace(plan_de_niveles[i][0])? (plan_de_niveles[i]+1) : plan_de_niveles[i]);

		if(!config_has_property(configuracion, clave)) {log_error(logger, "Archivo de configuracion invalido");exit(EXIT_FAILURE);}

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
		recursos_por_nivel[i] = malloc(strlen(string_recursos)+1);
		strcpy(recursos_por_nivel[i], string_recursos); // queda vector de todos los niveles en los q en cada
		// posicion hay una cadena con todos los recursos de ese nivel .
		free(string_recursos);
		free(clave);
		i++;
	}


	//-------------FIN LECTURA ARCHIVO CONFIG--------------//

	niveles_completados=0;

	while (!(cantidad_niveles==niveles_completados)) //mientras no haya terminado el plan de niveles
	{

		char * nivel_a_pedir;

		t_solicitud_info_nivel * solicitud_info_nivel;
		solicitud_info_nivel=malloc(sizeof(t_solicitud_info_nivel));

		t_info_nivel_planificador * info_nivel_y_planificador;

		t_notificacion_nivel_cumplido * notificacion_nivel_cumplido;
		notificacion_nivel_cumplido=malloc(sizeof(t_notificacion_nivel_cumplido));

		t_datos_delPersonaje_alPlanificador * datos_personaje_planificador;
		datos_personaje_planificador=malloc(sizeof(t_datos_delPersonaje_alPlanificador));

		t_datos_delPersonaje_alNivel *datos_personaje_nivel;
		datos_personaje_nivel=malloc(sizeof(t_datos_delPersonaje_alNivel));

		int socket_nivel;
		int socket_planificador;
		int recursos_obtenidos;
		int posicion[2] = {1,1};
		int destino[2]; 
		int sabe_donde_ir;
		int consiguio_total_recursos;

		nivel_a_pedir = plan_de_niveles[niveles_completados];

		solicitud_info_nivel->solicitor = simbolo;
		solicitud_info_nivel->nivel_solicitado=strdup(( isspace(nivel_a_pedir[0])?(nivel_a_pedir+1) : nivel_a_pedir ));
		log_info(logger, string_from_format("Proximo Nivel:%s",nivel_a_pedir), "INFO");

		socket_orquestador = init_socket_externo(puerto_orquestador, ip_orquestador, logger);
		log_debug(logger, "Conexión con hilo orquestador establecida", "DEBUG");		// 1111111111111111111

		enviar(socket_orquestador,SOLICITUD_INFO_NIVEL,solicitud_info_nivel,logger);

		info_nivel_y_planificador = recibir(socket_orquestador, INFO_NIVEL_Y_PLANIFICADOR);  //22222222222222222

		//comprobacion de si existe no el nivel perdido para romper
		if(!strcmp(info_nivel_y_planificador->ip_nivel, "NIVEL NO ENCONTRADO"))
		{
			log_error(logger, "No existe el nivel pedido al orquestador");
			exit(EXIT_FAILURE);
		}

		log_debug(logger, "Recibida la información del nivel y el planificador", "DEBUG");
		close(socket_orquestador);
		log_debug(logger, "Desconectado del hilo orquestador", "DEBUG");

		socket_nivel = init_socket_externo(info_nivel_y_planificador->puerto_nivel, info_nivel_y_planificador->ip_nivel, logger);
		log_info(logger, "Entrando al nivel...", "INFO");									//333333333333333333

		socket_planificador = init_socket_externo(info_nivel_y_planificador->puerto_planificador, ip_orquestador, logger);
		log_debug(logger, "Conectado al hilo planificador del nivel", "DEBUG");             //3333333333333333333

		free(info_nivel_y_planificador->ip_nivel);
		free(info_nivel_y_planificador);
		//ANUNCIO AL NIVEL Y AL PLANIFICADOR

		datos_personaje_planificador->char_personaje=simbolo;
		datos_personaje_planificador->nombre_personaje=strdup(nombre);
		enviar(socket_planificador,ENVIO_DE_DATOS_AL_PLANIFICADOR,datos_personaje_planificador,logger);

		datos_personaje_nivel->char_personaje=simbolo;
		datos_personaje_nivel->nombre_personaje= strdup(nombre);
		datos_personaje_nivel->necesidades=strdup(recursos_por_nivel[niveles_completados]);
		enviar(socket_nivel,ENVIO_DE_DATOS_PERSONAJE_AL_NIVEL,datos_personaje_nivel,logger);

		//FIN ANUNCIO


		sabe_donde_ir = 0; //booleano que representa si el personaje tiene un destino válido o no.
		//Se pone en Falso al entrar a un nivel
		recursos_obtenidos = 0; //cuantos recursos obtuvo
		consiguio_total_recursos=0;
		game_over=0;

		while(1)
		{


			char mje_a_recibir = 17;
			char  proximo_recurso;

			t_solicitud_movimiento *solicitud_movimiento;
			solicitud_movimiento=malloc(sizeof(t_solicitud_movimiento));
			t_resp_solicitud_movimiento * rspt_solicitud_movimiento;
			// rspt_solicitud_movimiento=malloc(sizeof(t_resp_solicitud_movimiento));
			t_turno_concluido * turno_concluido;
			turno_concluido=malloc(sizeof(t_turno_concluido));

			free(recibir(socket_planificador, NOTIF_MOVIMIENTO_PERMITIDO));  //RECIBIR QUANTUM   //333333333333333333333333333333
			//el propósito de este "recibir" es puramente que el personaje se bloquee
			//no necesita ninguna información. De hecho
			//el mensaje podría ser solamente el header y nada de datos.
			//recibir este mensaje significa que es el turno del personaje

			if(!sabe_donde_ir) //SI NO SABE DONDE IR, PEDIR LA UBICACION DEL PROXIMO RECURSO
			{
				t_solicitud_ubicacion_recurso * sol_ubicacion_recurso;
				sol_ubicacion_recurso=malloc(sizeof(t_solicitud_ubicacion_recurso));
				t_ubicacion_recurso * ubicacion_recursos; //recibir asigna memoria

				//ACCION: AVERIGUAR CUÁL ES EL PROXIMO RECURSO A OBTENER
				proximo_recurso = recursos_por_nivel[niveles_completados][recursos_obtenidos];

				//ACCION: ELABORAR SOLICITUD DE UBICACION DEL PROXIMO RECURSO
				sol_ubicacion_recurso->recurso=proximo_recurso;
				enviar(socket_nivel, SOLICITUD_UBICACION_RECURSO,sol_ubicacion_recurso, logger);
				ubicacion_recursos = recibir(socket_nivel, INFO_UBICACION_RECURSO); //por que esta linea estaba comentada?

				destino[0]=ubicacion_recursos->x; //esto podría ser más prolijo si destino lo hicieramos un struct en vez de un vector
				destino[1]=ubicacion_recursos->y ;//idem
				sabe_donde_ir = 1;

				log_info(logger,string_from_format("el nuevo destino es (%d,%d) y el recurso es %c ", destino[0], destino[1], proximo_recurso), "INFO");

				free(ubicacion_recursos);
			}

			int prox_paso[2];

			if (buscar_paso(posicion, destino, prox_paso)) //se modifican los datos a los que apunta prox_paso, no prox_paso en si
			{ //es decir, si tiene sentido moverse
				solicitud_movimiento->char_personaje=simbolo;
				solicitud_movimiento->x=prox_paso[0];
				solicitud_movimiento->y=prox_paso[1];

				enviar(socket_nivel,SOLICITUD_MOVIMIENTO_XY,solicitud_movimiento, 0);

				rspt_solicitud_movimiento=recibir(socket_nivel,RTA_SOLICITUD_MOVIMIENTO_XY);

				if (!rspt_solicitud_movimiento->aprobado) // esto se deberia cargar con el recibir asi q no hace falta inicializ.
				{
					log_error(logger, "Se intentó realizar un movimiento imposible", "ERROR");
					printf("SOLICITUD DE MOVIMIENTO DENEGADO");
					exit(EXIT_FAILURE); //terminar anormalmente
				}

				free(rspt_solicitud_movimiento);

				memcpy(posicion,prox_paso,sizeof(posicion)); //me muevo a la proxima posicion
			} //CON ESTO DOY EL FASITO, SI ES NECESARIO

			if(llego(posicion, destino)) //EVALUO SI LLEGUE A MI DESTINO
			{
				//llego es una funcion que simplemente compara la posicion con el destino, componente a componente
				//yes, estoy probando si llego dos veces, pero realmente hay que preguntar antes de pedir moverse (caso extremo), y despues de que me pude mover (caso usual)
				t_solcitud_instancia_recurso * solicitud_instancia;
				solicitud_instancia=malloc(sizeof(t_rspta_solicitud_instancia_recurso));
				t_rspta_solicitud_instancia_recurso * rpta_solicitud_instancia_recurso;
				//rpta_solicitud_instancia_recurso=malloc(sizeof(t_rspta_solicitud_instancia_recurso)); no necesita ini.
				//rpta_solicitud_instancia_recurso->concedido=0; lo use para probar
				solicitud_instancia->instancia_recurso = proximo_recurso;
				enviar(socket_nivel,SOLICITUD_INSTANCIA_RECURSO,solicitud_instancia,logger);

				rpta_solicitud_instancia_recurso = recibir(socket_nivel,RTA_SOLICITUD_INSTANCIA_RECURSO);

				if (rpta_solicitud_instancia_recurso->concedido)
				{

					//ACCION: ACTUALIZAR RECURSOS OBTENIDOS. SI CONSIGUIO EL TOTAL, INDICAR consiguio_total_recursos = 1
					recursos_obtenidos++;
					// printf("%d\n",recursos_obtenidos); ..recordar borrar el malloc de rpta_solic. ESTO ES TESTING?

					if(recursos_obtenidos == strlen(recursos_por_nivel[niveles_completados]))
					{

						consiguio_total_recursos = 1; // para que me sirve esta variable

						//	una vez que consiguio total de recursos se deberia desconectar del nivel ?
					}
					sabe_donde_ir = 0;
					log_info(logger, "Se obtuvo el recurso!", "INFO");
					//y despues no necesita hacer mas nada!
					turno_concluido->bloqueado=0;
					enviar(socket_planificador,NOTIF_TURNO_CONCLUIDO,turno_concluido,logger);

				}
				else if (!rpta_solicitud_instancia_recurso->concedido) //denegado
				{
					log_info(logger, "El personaje quedó a la espera del recurso", "INFO");

					turno_concluido->bloqueado=1;
					turno_concluido->recurso_de_bloqueo = proximo_recurso;
					enviar(socket_planificador,NOTIF_TURNO_CONCLUIDO,turno_concluido,logger);
					//INFORMO QUE ME BLOQUEE
					//TENGO QUE INFORMARLO SI O SI ACA, SINO COMO ME VAN A CONDENAR O CONCEDER EL RECURSO? ACA TERMINO EL TURNO

					mje_a_recibir = getnextmsg(socket_planificador);

					if(mje_a_recibir == NOTIF_PERSONAJE_CONDENADO)
					{

						free(recibir(socket_planificador,NOTIF_PERSONAJE_CONDENADO)); //hay que limpiarlo del socket

						log_info(logger, "Este personaje va a morir para solucionar un interbloqueo", "INFO");
						//notificar al nivel que murio el personaje
						//morir(); //morir se encarga de setear game_over si es necesario
						if(contador_vidas>0){
							contador_vidas--;
							recursos_obtenidos=0;
							//enviar mensaje al orquestador reiniciar nivel
						}else{
							contador_vidas=config_get_int_value(configuracion,"vidas");
							game_over=1;
						}
						break; //sale del nivel
					}
					else if (mje_a_recibir == NOTIF_RECURSO_CONCEDIDO)
					{
						free(recibir(socket_planificador,NOTIF_RECURSO_CONCEDIDO));
						recursos_obtenidos++;
						sabe_donde_ir=0;
						log_info(logger, "Finalmente se concedió el recurso! Personaje desbloqueado.", "INFO");
					}
				} //FIN CASO RECURSO DENEGADO
				free(rpta_solicitud_instancia_recurso);
			} //FIN SITUACION DE SOLICITAR RECURSO (LLEGO A DESTINO)


			else if(!llego(posicion, destino)) //si NO llegó a destino (no hacer nada más)
			{
				turno_concluido->bloqueado=0;
				enviar(socket_planificador,NOTIF_TURNO_CONCLUIDO,turno_concluido,logger);				
			}


			if(consiguio_total_recursos)
			{
				log_info(logger, "Nivel finalizado!", "INFO");
				notificacion_nivel_cumplido->char_personaje=simbolo;
				niveles_completados++; //aumentamos los niveles concluidos
				enviar(socket_nivel,NOTIF_NIVEL_CUMPLIDO,notificacion_nivel_cumplido,logger);  // a quien se lo envia realmente
				break;  //por acá se sale del while(1) [personaje sale del nivel]
			}

		}//fin while(1) [personaje en nivel]

		//en este punto es donde definitivamente se sale de un nivel. esto significa desconectarse del hilo planificador y del nivel


		close(socket_nivel);
		close(socket_planificador);
		//después del fin while(1), el personaje pide info del próximo nivel
		//si el personaje murió, entonces no marcó el nivel como concludio, y va a pedir la info del nivel en el que estaba. esto es dudoso, ya que la consigna dice que "notifica su intención de reiniciar el nivel"
		//en fin, el pedido de la info del proximo nivel la hace automaticamente el while plan de niveles

		if(game_over) //a menos que el personaje haya perdido todas sus vidas
		{
			//REINICIAR PLAN DE NIVELES
			niveles_completados=0;
			log_info(logger, "Game over - reiniciando plan de niveles","INFO");
		}


	} //FIN PLAN DE NIVELES


	//el personaje, al terminar su plan de niveles, se conecta al hilo orquestador y se lo notifica

	//ACCION: CONECTAR CON EL HILO ORQUESTADOR
	socket_orquestador=init_socket_externo(puerto_orquestador,ip_orquestador,logger);
	log_debug(logger, "Conexión con hilo orquestador establecida", "DEBUG");

	//ELABORAR NOTIFICACION DE PLAN TERMINADO
	notificacion_plan_terminado->personaje=strdup(nombre);  // revisar q le voy a mandar al orquestador
	enviar(socket_orquestador,NOTIF_PLAN_TERMINADO,notificacion_plan_terminado,logger);

	//	printf("%s\n",recursos_por_nivel[3]); que es esta linea????

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

int buscar_paso(int * posicion, int * destino, int * prox_paso) //pone en prox_paso el proximo paso a tomar. retorna 1 en caso de exito, 0 si ya estoy en mi destino
{
	if (llego(posicion, destino)) return 0;

	if (posicion[0]<destino[0])
	{
		prox_paso[0]=(posicion[0]+1);
		prox_paso[1]=posicion[1];

	}
	else if (posicion[0]>destino[0])
	{
		prox_paso[0]=(posicion[0]-1);
		prox_paso[1]=posicion[1];
	}
	else //if (posicion[0]==destino[0])
	{
		prox_paso[0]=posicion[0];
		if (posicion[1]<destino[1])
		{
			prox_paso[1]=(posicion[1]+1);
		}
		else if (posicion[1]>destino[1])
		{
			prox_paso[1]=(posicion[1]-1);
		}
		//aca no hay else porque significaria pos=dest (ya lo deberia haber atrapado la funcion llego)
	}
	return 1;
}


int llego(int pos[],int dest[]){
	//true si las componentes son iguales
	return pos[0]==dest[0] && pos[1]==dest[1];
}

