#ifndef CONSOLA_H
#define CONSOLA_H

#include <utils/shared.h>
#include <gestor_kernel.h>
#include <commons/string.h>

void leer_consola();
void ejecutar_script(t_buffer* buffer);
void iniciar_proceso(t_buffer* buffer);
int extraigo_string_delbuffer(t_buffer* buffer);


#endif // CONSOLA_H