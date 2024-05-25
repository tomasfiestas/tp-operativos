#include "contexto_ejecucion.h"

void enviar_contexto(t_contexto_ejecucion *ctx, op_code cod_op, int socket)
{
    t_paquete *paquete = crear_paquete(cod_op);

    serializar_contexto(paquete, ctx);

    enviar_paquete(paquete, socket);
}

t_contexto_ejecucion *recibir_contexto(int socket)
{
    t_buffer *buffer = crear_buffer();*

    buffer -> stream = recibir_buffer(socket);

    t_contexto_ejecucion *ctx = deserializar_contexto(buffer);

    liberar_buffer(buffer);

    return ctx;
}

void liberar_contexto(t_contexto_ejecucion *contexto_ejecucion){

    list_destroy_and_destroy_elements(contexto_ejecucion->instrucciones, (void*) liberar_instrucciones);
    free(contexto_ejecucion);
}