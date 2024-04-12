#ifndef SHARED_H
#define SHARED_H

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<assert.h>


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

// CLIENTE
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion_cliente(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void paquete(int conexion);

//SERVIDOR
extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto, t_log* logger);
int esperar_cliente(int socket_servidor, t_log* logger,char* mensaje);
t_list* recibir_paquete(int);

int recibir_operacion(int);

int esperar_cliente_memoria_entradasalida(int socket_servidor, t_log *logger, char* mensaje); //borrar


void realizar_handshake(int numero, int server);


void* atender_cliente(int socket_cliente_ptr,t_log* logger);


#endif