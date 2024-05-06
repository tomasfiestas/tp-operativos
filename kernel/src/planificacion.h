#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "gestor_kernel.h"
#include "servicios_kernel.h"
#include <semaphore.h>

t_pcb* pcb_nuevo;
char** recursos;
char** instancias_recursos;


//Listas de estados
t_queue* plani_new;
t_list* plani_ready;
t_list* total_pcbs;
t_list* plani_exec;
t_list* plani_block;
t_queue** plani_block_recursos;
t_list* plani_exit;

//t_list* lista_pcbs_sockets; //No se que es?

//Algoritmos de planificacion
algoritmos algoritmo_plani;

int multiprogramacion;

//Semaforos para multiprogramacion
sem_t lugares_ready_llenos;
sem_t lugares_ready_vacios;
sem_t mutex_multiprogramacion;

// sem_t multiPermiteIngresar;
 sem_t hayPCBsEnNew;
// sem_t hayPCBsEnReady;
//sem_t puedeEntrarAExec;

//Semaforo para respuestas
sem_t respuesta_memoria;
//sem_t se_creo_archivo;
sem_t respuesta_delete_segment;
sem_t proceso_inciado_memoria;

//Semaforo para los estados
sem_t sem_new;
sem_t sem_total_pcbs;
sem_t sem_ready;
sem_t sem_exec;
sem_t sem_block;
sem_t sem_exit;
sem_t sem_recursos;
sem_t sem_inst_recursos;


void crear_pcb(t_list* lista_instrucciones);
void inicializar_registros(t_pcb* nuevo_pcb);
void agregar_a_new(t_pcb* nuevo_pcb);
void agregar_a_ready(t_pcb* nuevo_pcb);
void inicializar_semaforos();
void inicializar_listas();
void iniciar_planificacion();
t_pcb* sacar_de_ready();
void agregar_a_exec(t_pcb* pcb);
void cambiar_estado_pcb(t_pcb* pcb, t_estado estadoNuevo);
char* estado_a_string(t_estado estado);




#endif 