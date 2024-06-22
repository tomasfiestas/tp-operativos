#ifndef PLANIFICACION_H
#define PLANIFICACION_H


#include <utils/shared.h>
#include <gestor_kernel.h>
#include <commons/string.h>
#include <semaphore.h>
#include <commons/temporal.h>  






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
//pthread_t hilo_quantum;

void mostrar_pids_ready();

void inicializar_semaforos();
void inicializar_listas();
void iniciar_planificacion();
void cambiar_grado_multiprogramacion(t_buffer *buffer);
void resetear_semaforos_multi(int vieja_multi);
void NewFunction(int nueva_multi);
void detener_planificacion();

algoritmos obtener_algoritmo();

void mandar_contexto_a_CPU(t_pcb *pcb);

//Planificadores
void inicializar_hilos();

void* inicio_plani_largo_plazo(void* arg);
void* inicio_plani_corto_plazo(void* arg);
void* contar_quantum();


void atender_crear_pr2(t_pcb* pcb,op_code op_code);
//void atender_crear_pr2(t_buffer* buffer,op_code op_code);
void atender_cpu_dispatch(void* socket_cliente_ptr);
void *round_robin(void * pcb);
void atender_proceso_desalojado(t_buffer* buffer, op_code op_code);
void atender_fin_proceso_success(t_buffer* buffer,op_code op_code);
void atender_fin_proceso(t_buffer* buffer,op_code op_code,t_pcb* pcb);
void wait_recurso(t_pcb *pcb, char *recurso_recibido);
bool existe_recurso(char *recurso_recibido);
//void disminuir_cantidad_recurso(t_pcb  *pcb, t_recurso *recurso);
int encontrar_posicion_recurso(char* target_char);

void *manejo_quantum(t_pcb * pcb);

void finalizarProceso(int pid);
t_pcb *buscarPcb(int pid_a_buscar);
t_pcb *buscarPcbBloqueado(int pid_a_buscar);
void sacar_de_lista(t_list * lista, int pid);
void sacar_pcb_de_lista(t_pcb* pcb);
void agregar_a_exit(t_pcb* pcb,op_code motivo_a_mostrar);
char *mensaje_a_string(op_code motivo);
void mostrar_pids_y_estados();
void sacar_de_bloqueado(t_pcb* pcb);

//Manejo de IO
t_entrada_salida* buscar_interfaz(char* nombre);
int validar_instruccion_interfaz(t_entrada_salida* t_entrada_salida,op_code op_code);


#endif 