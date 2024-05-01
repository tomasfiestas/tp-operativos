#ifndef MEMORIA_H
#define MEMORIA_H
#define TAM_MAX_INSTRUCCION 16
#define TAM_MAX_PARAMETRO 32
#define CANT_MAX_PARAMETRO 5

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include <sys/socket.h>

typedef struct {
    unsigned int numeroFrame;
    bool estaPresente;
    // agregar mas info luego
} RegistroTablaPaginas;

typedef struct {
    RegistroTablaPaginas* registros;
    size_t size;
} TablaPaginas;

typedef struct {
    char* instruccion;
    char* parameters[5];
} Instruccion;

typedef struct {
    char instruction[TAM_MAX_INSTRUCCION];
    char parameters[CANT_MAX_PARAMETRO][TAM_MAX_PARAMETRO];
} InstruccionSerializada;


t_log* memoria_logger;
t_config* memoria_config;

char* PUERTO_ESCUCHA;
char* TAM_MEMORIA;
char* TAM_PAGINA;
char* PATH_INSTRUCCIONES;
char* RETARDO_RESPUESTA;
void iterator(char* value);
int cliente_entradasalida;
int cliente_kernel;
int cliente_cpu;

void atender_cpu(void* socket_cliente_ptr);
void atender_entradasalida(void* socket_cliente_ptr);
void atender_kernel(void* socket_cliente_ptr);
void* reservar_memoria();
void parse_file(const char* filePath);
TablaPaginas* iniciar_tabla_paginas(void* memoria);

#endif