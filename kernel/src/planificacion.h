#ifndef PLANIFICACION_H
#define PLANIFICACION_H


#include <utils/shared.h>
#include <gestor_kernel.h>
#include <commons/string.h>
#include <semaphore.h>




void crear_pcb(int pid);
void inicializar_registros(t_pcb* nuevo_pcb);
//Manejo de estados
void agregar_a_new(t_pcb* nuevo_pcb);
t_pcb* sacar_siguiente_de_new();

void agregar_a_ready(t_pcb* nuevo_pcb);
void agregar_a_exec(t_pcb* pcb);
t_pcb* sacar_de_ready();
t_pcb* pcb_de_exec();
void cambiar_estado_pcb(t_pcb* pcb, t_estado estadoNuevo);
char* estado_a_string(t_estado estado);



void inicializar_semaforos();
void inicializar_listas();
void iniciar_planificacion();

algoritmos obtener_algoritmo();


void mandar_contexto_a_CPU(t_pcb* pcb);

//Planificadores
void inicializar_hilos();

void* inicio_plani_largo_plazo(void* arg);
void* inicio_plani_corto_plazo(void* arg);
void* contar_quantum();


void atender_crear_pr2(t_buffer* buffer,op_code op_code);
void atender_cpu_dispatch(void* socket_cliente_ptr);
void *round_robin(void * pcb);
void atender_proceso_desalojado(t_buffer* buffer, op_code op_code);

void *manejo_quantum(void * pcb);

#endif 