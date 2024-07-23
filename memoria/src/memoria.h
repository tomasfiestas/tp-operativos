#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include <sys/socket.h>
#include <pthread.h>

typedef struct {
    int pid;
    int pc;
    t_list* instrucciones;
    t_list* paginas;
} t_proceso;

typedef struct {
    char* nombre;
    int socket;
} t_io;

extern t_log* memoria_logger;
extern t_config* memoria_config;
extern t_list* procesos;

extern char* PUERTO_ESCUCHA;
extern char* TAM_MEMORIA;
extern char* TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern char* RETARDO_RESPUESTA;
extern int cliente_entradasalida;
extern int cliente_kernel;
extern int cliente_cpu;
extern int cantidad_procesos;


void iterator(char* value);
void* atender_cpu(void* socket_cliente_ptr);
void* atender_entradasalida(void* socket_cliente_ptr);
void* atender_kernel(void* socket_cliente_ptr);
void* reservar_memoria();
t_list* parse_file(const char* filePath);
t_proceso* obtener_proceso(int pid);
void atender_crear_proceso(t_buffer* buffer);
void atender_eliminar_proceso(t_buffer* buffer);
t_io* buscar_interfaz(char* nombre);

#endif