#ifndef CPU_H
#define CPU_H

#include <gestor_cpu.h>


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

void atender_kernel_dispatch(void* socket_cliente_ptr);
void atender_kernel_interrupt(void* socket_cliente_ptr);
void devolver_contexto(t_pcb *ctx,op_code cod_operacion);




#endif