#ifndef GESTOR_CPU_H
#define GESTOR_CPU_H

#include<stdio.h>
#include<stdlib.h>
#include <commons/log.h>
#include <utils/shared.h>
#include <cpu.h>

#include <stdio.h>
#include <utils/hello.h> 
#include <utils/logging.h>
#include <pthread.h>
#include <instrucciones.h>
#include <cpu.h>


extern int socket_memoria;
extern int socket_kernel;
extern int direccion_fisica;
extern int conexion_memoria;
extern t_log* cpu_logger;
extern t_config* cpu_config;
extern int tamanio_pagina;
//extern t_registros regs;



extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern int CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;
extern int conexion_memoria;
extern int cliente_kernel_dispatch;
extern int cliente_kernel_interrupt;
extern bool llego_interrupcion ;
extern pthread_t hilo_kernel_dispatch;
extern int direccion_fisica;
extern t_list* tlb;



#endif