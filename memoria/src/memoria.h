#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include <sys/socket.h>



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



#endif