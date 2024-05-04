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

typedef enum{
	NEW,
	READY,
	EXEC,
	BLOCK,
	FIN
} t_estado;

typedef struct{
	uint8_t AX;
	uint8_t BX;
	uint8_t CX;
	uint8_t DX;
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
	uint32_t SI;
	uint32_t DI;
} t_registros;

typedef struct{
	int pid;
	t_list* instrucciones;
	int program_counter;
	t_estado estado;
	t_registros registros;
	t_list* tabla_segmentos;
	double estimado_prox_rafaga;	
	t_list* tabla_archivos;	
	int ejecuto;
} t_pcb;





typedef enum{
	FIFO,
	VRR,
	RR
} algoritmos;

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
void cargar_string_a_buffer(t_buffer* buffer, char* valor);

void* extraer_de_buffer(t_buffer* buffer);
int extraer_int_del_buffer(t_buffer* buffer);
char* extraer_string_del_buffer(t_buffer* buffer);
void* serializar_paquete(t_paquete* paquete);





t_paquete* crear_paquete(op_code cod_op, t_buffer* buffer);
void destruir_paquete(t_paquete* paquete);
void iniciar_proceso(t_buffer* buffer);


#endif