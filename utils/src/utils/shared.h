#ifndef SHARED_H
#define SHARED_H
#define TAM_MAX_INSTRUCCION 16
#define TAM_MAX_PARAMETRO 32
#define CANT_MAX_PARAMETRO 5

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
	HANDSHAKE_KERNEL,
	HANDSHAKE_CPU,
	HANDSHAKE_MEMORIA,
	HANDSHAKE_ES,

	//KERNEL
	//Kernel le avisa a memoria que tiene que crear un proceso
	CREAR_PROCESO_KM
}op_code;

typedef struct {
    char instruction[TAM_MAX_INSTRUCCION];
    char parameters[CANT_MAX_PARAMETRO][TAM_MAX_PARAMETRO];
} instruccion_serializada;

typedef struct {
	instruccion_serializada* instrucciones;
	int cantidad;
} t_instrucciones;

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

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void paquete(int conexion);
void iterator(char* value);
int iniciar_servidor(char* puerto);
int esperar_cliente(int socket_servidor);
t_list* recibir_paquete(int);
int recibir_operacion(int);
void realizar_handshake(op_code module, int server);
void recibir_mensaje(int socket_cliente);

//Agrego funciones de prueba para enviar y recibir paquetes
t_buffer* crear_buffer();
t_buffer* recibir_buffer(int conexion);
void destruir_buffer(t_buffer* buffer);
void cargar_a_buffer(t_buffer* buffer, void* valor, int tamanio);
void cargar_int_a_buffer(t_buffer* buffer, int valor);
void cargar_instrucciones_a_buffer(t_buffer* buffer, t_instrucciones instrucciones);
void cargar_string_a_buffer(t_buffer* buffer, char* valor);

void* extraer_de_buffer(t_buffer* buffer);
int extraer_int_del_buffer(t_buffer* buffer);
char* extraer_string_del_buffer(t_buffer* buffer);
t_instrucciones* extraer_instrucciones_del_buffer(t_buffer* buffer);
void* serializar_paquete(t_paquete* paquete);





t_paquete* crear_paquete(op_code cod_op, t_buffer* buffer);
void destruir_paquete(t_paquete* paquete);
void iniciar_proceso(t_buffer* buffer);


#endif