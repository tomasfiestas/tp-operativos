#ifndef MEMORIA_USUARIO_H
#define MEMORIA_USUARIO_H

#include "memoria.h"
#include <commons/bitarray.h>

t_bitarray bitmap;

t_tabla_paginas* iniciar_tabla_paginas(int pid);
void inicializar_memoria();
void* reservar_memoria();
void crear_proceso(int pid, t_instrucciones* instrucciones);