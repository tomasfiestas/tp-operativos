#ifndef ENTRADASALIDA_H
#define ENTRADASALIDA_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include <readline/readline.h>
#include <filesystem.h>

/*
void crear_interfaz(char* nombre, char* tipo){
    t_paquete* paquete = crear_paquete();
    crear_buffer();
    cargar_string_a_buffer(paquete->buffer, nombre);
    cargar_string_a_buffer(paquete->buffer, tipo);

    log_info(logger, "Interfaz Generica creada");
}
*/
typedef enum{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} t_tipo_interfaz;

typedef struct{
    char* nombre;
    t_tipo_interfaz tipo;
    int tiempo_unidad_trabajo;
} t_interfaz;

typedef struct{
    char* nombre;
    t_tipo_interfaz tipo;
    int tiempo_unidad_trabajo;
    int block_size;
    int block_count;
    int retraso_compactacion;
} t_dial_fs;

typedef struct{
    op_code codigo_operacion;
    t_buffer* buffer;
} t_struct_atender_kernel;

extern t_log* io_logger;

extern t_config* entradasalida_config;

extern char* TIPO_INTERFAZ;
extern char* TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern char* BLOCK_SIZE;
extern char* BLOCK_COUNT;
extern char* RETRASO_COMPACTACION;
extern int conexion_kernel;
extern int conexion_kernel2;
extern int conexion_memoria;
extern int tiempo_fs;


void atender_mensajes_memoria(void* socket_cliente_ptr);

void leer_consola();

t_mensajes_consola mensaje_a_consola(char *mensaje_consola);

void crear_interfaz(char* nombre, char* tipo,int unidades_trabajo);

void crear_interfaz_fs(char* nombre, char* tipo, int tiempo_unidad_trabajo,int block_size, int block_count, int retraso_compactacion);

t_interfaz* buscar_interfaz(char* nombre);

void atender_peticiones_de_kernel(t_struct_atender_kernel* struct_atender_kernel);

void atender_kernel(void* socket_cliente_ptr);

void enviar_solicitud_escritura(int pid, int direccion_fisica, int tamanio,char* valor_a_escribir);

t_list* crear_lista_direcciones(t_buffer* buffer);

char* leer_de_consola(int tamanio);

void instruccion_realizada(int socket_kernel, char* nombre_recibido, int pid, char* instruccion_realizada);

void escribir_a_memoria(t_list* lista_paginas, int size,t_pcb* pcb, void* valor);

void* leer_de_memoria(t_list* lista_df,int bytes_a_leer, int pid, int socket_memoria);

t_list* extraer_lista_de_direcciones_de_buffer(t_buffer* buffer);

void inicializar_interfaces(char* path);

t_fcb* crear_fcb(char* nombre_archivo);


#endif