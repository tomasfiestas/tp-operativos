#ifndef GLOBAL_CPU_H
#define GLOBAL_CPU_H

#include <utils/shared.h>

extern t_pcb* pcb_ejecutando;
extern t_pcb* pcb_a_finalizar;

extern int hay_interrupcion;
extern int mandaron_finalizar_desde_consola;

#endif