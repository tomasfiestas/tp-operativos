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
sem_t lugares_ready_llenos = 0;
sem_t lugares_ready_vacios = MULTIPROGRAMACION;
sem_t mutex_multiprogramacion;

// sem_t multiPermiteIngresar;
// sem_t hayPCBsEnNew;
// sem_t hayPCBsEnReady;
// sem_t puedeEntrarAExec;

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






#endif 