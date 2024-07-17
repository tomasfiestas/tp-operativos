#ifndef ENTRADASALIDA_H
#define ENTRADASALIDA_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include<readline/readline.h>

/*
void crear_interfaz(char* nombre, char* tipo){
    t_paquete* paquete = crear_paquete();
    crear_buffer();
    cargar_string_a_buffer(paquete->buffer, nombre);
    cargar_string_a_buffer(paquete->buffer, tipo);

    log_info(logger, "Interfaz Generica creada");
}
*/

int conexion_kernel;
int conexion_memoria;


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
    int retraso_compactacion
} t_dial_fs;

typedef struct{
    op_code codigo_operacion;
    t_buffer* buffer;
} t_struct_atender_kernel;




extern t_log* io_logger;

t_config* entradasalida_config;

char* TIPO_INTERFAZ;
char* TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
char* BLOCK_SIZE;
char* BLOCK_COUNT;
int conexion_kernel;
int conexion_kernel2;


void atender_mensajes_memoria(void* socket_cliente_ptr);
void leer_consola();
t_mensajes_consola mensaje_a_consola(char *mensaje_consola);
void crear_interfaz(char* nombre, char* tipo,int unidades_trabajo);
void crear_interfaz_fs(char* nombre, char* tipo, int tiempo_unidad_trabajo,int block_size, int block_count, int retraso_compactacion);
t_interfaz* buscar_interfaz(char* nombre);
void atender_peticiones_de_kernel(t_struct_atender_kernel* struct_atender_kernel);
void atender_kernel(void* socket_cliente_ptr);


//extern t_fcb *fcb;

#endif