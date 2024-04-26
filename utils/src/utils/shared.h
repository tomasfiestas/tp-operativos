#ifndef SHARED_H
#define SHARED_H

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<commons/string.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<assert.h>

typedef enum {
	EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    MULTIPROGRAMACION,
    PROCESO_ESTADO,
	ERROR,
	EXIT
} t_mensajes_consola;

typedef enum
{
	MENSAJE,
	PAQUETE,
	HANDSHAKE,

	//KERNEL
	//Kernel le avisa a memoria que tiene que crear un proceso
	CREAR_PROCESO_KM
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

typedef enum{
	KERNEL,
	CPU,
	MEMORIA,
	IO,
	//CREAR_PROCESO_KM
}module_code;

int crear_conexion_cliente(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void paquete(int conexion);
void iterator(char* value);
//void* recibir_buffer(int*, int);
int iniciar_servidor(char* puerto);
int esperar_cliente(int socket_servidor);
t_list* recibir_paquete(int);
int recibir_operacion(int);
void realizar_handshake(module_code module, int server);
void handle_handshake(module_code module);
void* atender_cliente(void* socket_cliente_ptr);
void recibir_mensaje(int socket_cliente);
//t_buffer* crear_buffer();


//Agrego funciones de prueba para enviar y recibir paquetes
t_buffer* crear_buffer();
t_buffer* recibir_buffer(int conexion);
void destruir_buffer(t_buffer* buffer);
void cargar_a_buffer(t_buffer* buffer, void* valor, int tamanio);
void cargar_int_a_buffer(t_buffer* buffer, int valor);
void cargar_string_a_buffer(t_buffer* buffer, char* valor);

void* extraer_de_buffer(t_buffer* buffer);
int extraer_int_del_buffer(t_buffer* buffer);
char* extraer_string_del_buffer(t_buffer* buffer);
void* serializar_paquete(t_paquete* paquete);





t_paquete* crear_paquete(op_code cod_op, t_buffer* buffer);
void destruir_paquete(t_paquete* paquete);
void iniciar_proceso(t_buffer* buffer);


#endif