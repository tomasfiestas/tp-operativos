#ifndef _INSTRUCCIONES_CPU_H_
#define _INSTRUCCIONES_CPU_H_

/*#include<stdio.h>
#include<stdlib.h>
#include <commons/log.h>
#include <utils/shared.h>
#include <cpu.h>*/
#include <gestor_cpu.h>






typedef struct {
	int pagina;
	int marco;
	int pid;
} t_tlb_entry;

void ciclo_de_instruccion(t_pcb* pcbb);
t_instruccion fetch(t_pcb *ctx);
bool decode(t_instruccion instruccion);
void asignar_valor_a_registro(t_pcb* contexto,char* registro, int valor);
int obtener_valor_de_registro(t_pcb* contexto, char* registro);
void execute(t_instruccion instruccion, t_pcb* contexto);
t_instruccion solicitar_instruccion_a_memoria(t_pcb* t_pcb);
t_instruccion crear_instruccion_nuevamente(t_instruccion_a_enviar instruccion_a_enviar, t_buffer *buffer_recibido);
char* leer_de_memoria(int dir_fisica,int bytes_a_leer, int pid);

void asignar_valor_char_a_registro(t_pcb* contexto,char* registro, char* valor);
#endif // !_INSTRUCCIONES_CPU_H_