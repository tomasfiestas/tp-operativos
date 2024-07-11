#ifndef CPU_H
#define CPU_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h> 
#include <utils/logging.h>
#include <utils/shared.h>
#include <pthread.h>
#include <instrucciones.h>



t_log* cpu_logger;
t_config* cpu_config;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
char* CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;
int conexion_memoria;
int cliente_kernel_dispatch;
int cliente_kernel_interrupt;
bool llego_interrupcion = false;
pthread_t hilo_kernel_dispatch;

void atender_kernel_dispatch(void* socket_cliente_ptr);
void atender_kernel_interrupt(void* socket_cliente_ptr);
void atender_crear_pr(t_buffer* buffer);
void atender_fin_quantum(t_buffer* buffer);



#endif