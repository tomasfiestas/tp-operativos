#ifndef _INSTRUCCIONES_CPU_H_
#define _INSTRUCCIONES_CPU_H_

/*#include<stdio.h>
#include<stdlib.h>
#include <commons/log.h>
#include <utils/shared.h>
#include <cpu.h>*/
#include <gestor_cpu.h>
#include <stdint.h>
#include <inttypes.h>




typedef struct {
	int pagina;
	int marco;
	int pid;
} t_tlb_entry;

void ciclo_de_instruccion(t_pcb* pcbb);
t_instruccion fetch(t_pcb *ctx);
bool decode(t_instruccion instruccion);
void asignar_valor_a_registro(t_pcb* contexto,char* registro, int valor);
void* obtener_puntero_al_registro(t_pcb* contexto, char* registro);
void execute(t_instruccion instruccion, t_pcb* contexto);
t_instruccion solicitar_instruccion_a_memoria(t_pcb* t_pcb);
t_instruccion crear_instruccion_nuevamente(t_instruccion_a_enviar instruccion_a_enviar, t_buffer *buffer_recibido);
void* leer_de_memoria(t_list* lista_df,int bytes_a_leer, int pid);
//char* leer_de_memoria(t_list* lista_df,int bytes_a_leer, int pid);
void escribir_a_memoria(t_list* lista_paginas, int size,t_pcb* pcb, void* valor);

void asignar_valor_char_a_registro(t_pcb* contexto,char* registro, uint32_t valor);
void escribir_a_memoria_string(int dir_fisica, int size,int pid, char* valor);
t_list* traducir_direccion_mmu(int dir_logica, t_pcb *ctx,int tamanio_solicitad);
#endif // !_INSTRUCCIONES_CPU_H_