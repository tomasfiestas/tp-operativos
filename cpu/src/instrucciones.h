#ifndef _INSTRUCCIONES_CPU_H_
#define _INSTRUCCIONES_CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include <commons/log.h>
#include <utils/shared.h>
#include <cpu.h>

extern int socket_memoria;
extern int socket_kernel;
extern int direccion_fisica;
extern int conexion_memoria;
extern t_log* cpu_logger;
extern t_registros regs;

typedef struct {
	int pagina;
	int marco;
} t_tlb_entry;

void ciclo_de_instruccion(t_pcb* pcbb);
t_instruccion fetch(t_pcb* contexto);
int decode(t_instruccion instruccion);
void asignar_valor_a_registro(t_pcb* contexto,char* registro, int valor);
int obtener_valor_de_registro(t_pcb* contexto, char* registro);
void execute(t_instruccion instruccion, t_pcb* contexto);


#endif // !_INSTRUCCIONES_CPU_H_