#ifndef GESTOR_KERNEL_H
#define GESTOR_KERNEL_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include <consola.h>
#include<readline/readline.h>

extern int conexion_k_memoria;
extern void iterator(char* value);
extern t_log* kernel_logger;
extern t_config* kernel_config;
extern char* PUERTO_ESCUCHA;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;
extern char* ALGORITMO_PLANIFICACION;
extern char* QUANTUM;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;
extern char* GRADO_MULTIPROGRAMACION;
extern int servidor;
extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern void iterator(char* value);
extern int recibir_cosas(int cliente_fd);
extern int cliente_entradasalida;

 
extern pthread_mutex_t mutex_pid;
extern int identificador_PID;
extern int contador_pcbs;

#endif 