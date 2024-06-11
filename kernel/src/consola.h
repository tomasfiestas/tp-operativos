#ifndef CONSOLA_H
#define CONSOLA_H

#include <utils/shared.h>
#include <gestor_kernel.h>
#include <commons/string.h>

void leer_consola();
void ejecutar_script(t_buffer* buffer);
void iniciar_proceso(t_buffer* buffer);
void finalizar_proceso(t_buffer* buffer);
int extraigo_string_delbuffer(t_buffer* buffer);
void ejecutar_archivo(const char* filePath);
void procesar_mensaje(t_mensajes_consola mensaje_a_consola, char** argumentos);




#endif // CONSOLA_H