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
int64_t quantum_64;
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
pthread_t hilo_quantum;
int instancias_recurso_1; 
int instancias_recurso_2; 
int instancias_recurso_3; 
int tamanio_lista_recursos;
int result;
bool llego_contexto = false;
int identificador_hilo = 0;

t_list* lista_interfaces;
void atender_entradasalida2(void* socket_cliente_ptr);



#endif