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

typedef struct{
    char* nombre;
    char* tipo;
} t_interfaz;


extern t_log* logger;
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

typedef enum{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} t_tipo_interfaz;


void atender_mensajes_memoria(void* socket_cliente_ptr);
void leer_consola();
t_mensajes_consola mensaje_a_consola(char *mensaje_consola);

#endif