#ifndef KERNEL_H
#define KERNEL_H

#include <gestor_kernel.h>

void iterator(char* value);
t_log* kernel_logger;
t_config* kernel_config;
int conexion_k_memoria;
char* PUERTO_ESCUCHA;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* ALGORITMO_PLANIFICACION;
int QUANTUM;
char** RECURSOS;
char** INSTANCIAS_RECURSOS;
int GRADO_MULTIPROGRAMACION;
int servidor;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
void iterator(char* value);
int recibir_cosas(int cliente_fd);
int cliente_entradasalida;

pthread_mutex_t mutex_pid;
int identificador_PID = 1;
int contador_pcbs;


void atender_entradasalida2(void* socket_cliente_ptr);


#endif