#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

// Servidor que recibe múltiples clientes, usar con el cliente de la cátedra.

#define DIRECCION INADDR_ANY   //INADDR_ANY representa la direccion de cualquier
							   //interfaz conectada con la computadora
#define PUERTO 5000
#define BUFF_SIZE 1024

void manejoCliente(void * socketCliente); // funcion que recibe datos desde el socket de un cliente.

int main() {

		int socketEscucha, socketNuevaConexion;
	struct sockaddr_in socketInfo;
	int optval = 1;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = DIRECCION; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(PUERTO);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {
		perror("Error al bindear socket escucha");
		return EXIT_FAILURE;
	}

	// Escuchar nuevas conexiones entrantes.
	if (listen(socketEscucha, 10) != 0)
	{
		perror("Error al poner a escuchar socket");
		return EXIT_FAILURE;
	}

	printf("Escuchando conexiones entrantes.\n");

	while (1)
	 {
       	 // Aceptar una nueva conexion entrante. Se genera un nuevo socket con la nueva conexion.
		if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) < 0) 
		{
		perror("Error al aceptar conexion entrante");
		return EXIT_FAILURE;
		} 
		else {
			pthread_t hiloCliente; // hilo que interactuará con UN cliente
			pthread_create(&hiloCliente,NULL,(void *)manejoCliente,(void *)socketNuevaConexion);
		}		
	}//fin while

	close(socketEscucha);
	return EXIT_SUCCESS;
}

void manejoCliente(void *socketCliente){
	int nbytesRecibidos;
	char buffer[BUFF_SIZE];

		while (1) 
		{
			// Recibir hasta BUFF_SIZE datos y almacenarlos en 'buffer'.
			if ((nbytesRecibidos = recv((int)socketCliente, buffer, BUFF_SIZE, 0))	> 0) 
			{
				printf("Mensaje recibido: ");
				fwrite(buffer, 1, nbytesRecibidos, stdout);
				printf("\n");
				printf("Tamanio del buffer %d bytes!\n", nbytesRecibidos);
				fflush(stdout);

				if (memcmp(buffer, "fin", nbytesRecibidos) == 0) {

					printf("Un cliente se ha desconectado.\n");
					break;

				}
				printf("\n");
			} else {
			perror("Error al recibir datos");
			break;
			}
		}//fin while

		close( (int) socketCliente);
		pthread_exit(NULL);
}
