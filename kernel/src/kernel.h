#ifndef KERNEL_H
#define KERNEL_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
/* #include <utils/conexion_servidor.h>
#include <utils/conexion_cliente.h> */
#include <utils/shared.h>

void iterator(char* value);
t_log* kernel_logger;
t_config* kernel_config;
char* puerto_escucha;
char* puerto_memoria;
char* ip_memoria;
char* ip_cpu;
char* puerto_cpu;
int servidor;
void iterator(char* value);
int recibir_cosas(int cliente_fd);





#endif