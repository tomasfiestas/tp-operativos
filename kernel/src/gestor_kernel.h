#ifndef GESTOR_KERNEL_H
#define GESTOR_KERNEL_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/shared.h>
#include <consola.h>
#include<readline/readline.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <planificacion.h>
#include <servicios_kernel.h>
#include <commons/temporal.h>  


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
extern int QUANTUM;
extern int64_t quantum_64;
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;
extern int GRADO_MULTIPROGRAMACION;
extern int servidor;
extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern void iterator(char* value);
extern int recibir_cosas(int cliente_fd);
extern int cliente_entradasalida;
extern int instancias_recurso_1; 
extern int instancias_recurso_2; 
extern int instancias_recurso_3; 
extern t_list *recursos_del_sistema;
extern t_list *lista_recursos_bloqueados;
extern  int tamanio_lista_recursos;

extern int result;
extern bool llego_contexto; 
extern pthread_mutex_t mutex_pid;
extern int identificador_PID;
extern int contador_pcbs;
extern t_temporal* timer;
extern int64_t tiempo_ejecutado;
extern int identificador_hilo;
extern t_list* lista_interfaces;


extern pthread_t hilo_quantum;
typedef struct{
    //char* identificador;
    t_queue* cola_bloqueados_recurso;//puede ser una cola de un struct que tiene pcb y motivo de bloqueo
    //char* tipo; //cola de archivo o recurso
}t_cola_block;



#endif 