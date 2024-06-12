#ifndef MEMORIA_USUARIO_H
#define MEMORIA_USUARIO_H

#include "memoria.h"
#include <commons/bitarray.h>

typedef struct {
    int frame;
    bool presente;
} t_pagina;

char* bitmap;
t_bitarray* bitarray;
void* memoria_total;

t_list* iniciar_tabla_paginas();
void inicializar_memoria();
void* reservar_memoria();
void finalizar_proceso(int pid);
t_proceso* obtener_proceso(int pid);
int resize(int pid, int bytes);
int asignar_memoria(t_proceso *proceso, int cantidad_paginas);
int obtener_numero_marco(int numero_pagina);
bool validar_si_existen_marcos(int cantidad_paginas);
int obtener_cantidad_marcos_disponibles();
int obtener_cantidad_paginas_en_uso(t_proceso *proceso);

#endif