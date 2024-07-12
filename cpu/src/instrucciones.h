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
} t_tlb_entry;

void ciclo_de_instruccion(t_pcb* pcbb);
t_instruccion fetch(t_pcb *ctx);
bool decode(t_instruccion instruccion);
void asignar_valor_a_registro(t_pcb* contexto,char* registro, int valor);
int obtener_valor_de_registro(t_pcb* contexto, char* registro);
void execute(t_instruccion instruccion, t_pcb* contexto);
t_instruccion solicitar_instruccion_a_memoria(t_pcb* t_pcb);

#endif // !_INSTRUCCIONES_CPU_H_