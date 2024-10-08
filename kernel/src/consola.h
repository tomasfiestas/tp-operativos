#ifndef CONSOLA_H
#define CONSOLA_H

#include <utils/shared.h>
#include <gestor_kernel.h>
#include <commons/string.h>

void leer_consola();

void iniciar_proceso(t_buffer* buffer);
void finalizar_proceso_por_consola(t_buffer* buffer);
void mandar_fin_proceso_a_cpu(t_pcb* pcb_a_finalizar);
int extraigo_string_delbuffer(t_buffer* buffer);
//void ejecutar_archivo(const char* filePath);
//void procesar_mensaje(t_mensajes_consola mensaje_a_consola, char** argumentos);
void procesar_mensaje(t_mensajes_consola mensaje_a_consola, char** argumentos);
void ejecutar_script(char* argumentos);
void ejecutar_archivo(const char* filePath);






#endif // CONSOLA_H