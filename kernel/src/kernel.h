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
char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* ALGORITMO_PLANIFICACION;
char* QUANTUM;
char** RECURSOS;
char** INSTANCIAS_RECURSOS;
char* GRADO_MULTIPROGRAMACION;
int servidor;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
void iterator(char* value);
int recibir_cosas(int cliente_fd);





#endif