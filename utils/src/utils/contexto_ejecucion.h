#ifndef CONTEXTO_EJECUCION_H_
#define CONTEXTO_EJECUCION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/collections/list.h>


#include <../shared.h>


//Funciones
void enviar_contexto(t_contexto_ejecucion *ctx, codigo_operacion cod_op, int socket);
t_contexto_ejecucion *recibir_contexto(int socket);
void liberar_contexto(t_contexto_ejecucion* contexto_ejecucion);

#endif