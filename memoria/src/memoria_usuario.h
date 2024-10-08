#ifndef MEMORIA_USUARIO_H
#define MEMORIA_USUARIO_H

#include "memoria.h"
#include <commons/bitarray.h>
#include <math.h>

typedef struct {
    int frame;
    bool presente;
} t_pagina;

extern char* bitmap;
extern t_bitarray* bitarray;
extern void* memoria_total;

int iniciar_tabla_paginas(t_proceso* proceso);
void* leer_memoria(int pid, int direccion_fisica, int size);
int escribir_memoria(int pid, int direccion_fisica, void* bytes, int size);
void inicializar_memoria();
void inicializar_bitmap();
void* reservar_memoria();
void finalizar_proceso(int pid);
int resize(int pid, int bytes);
int asignar_memoria(t_proceso *proceso, int cantidad_paginas);
int obtener_numero_marco(int pid,int numero_pagina);
bool validar_si_existen_marcos(int cantidad_paginas);
int obtener_cantidad_marcos_disponibles();
int obtener_cantidad_paginas_en_uso(t_proceso *proceso);
int obtener_indice_proceso(t_proceso* proceso);

#endif