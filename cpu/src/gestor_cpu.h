#ifndef GESTOR_CPU_H
#define GESTOR_CPU_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include <pthread.h>
#include <ciclo_inst_cpu.h>


extern t_log* cpu_logger;
extern t_config* cpu_config;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern char* CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;
extern int conexion_memoria;
extern int cliente_kernel_dispatch;
extern int cliente_kernel_interrupt;

#endif