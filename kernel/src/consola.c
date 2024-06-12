#include "consola.h"
t_mensajes_consola mensaje_a_consola(char *mensaje_consola){
    
    if(strcmp(mensaje_consola,"EJECUTAR_SCRIPT") == 0){
        return EJECUTAR_SCRIPT;
    }
    if(strncmp(mensaje_consola,"INICIAR_PROCESO",14) == 0){
        return INICIAR_PROCESO;
    }
    if(strcmp(mensaje_consola,"FINALIZAR_PROCESO") == 0){
        return FINALIZAR_PROCESO;
    }
    if(strcmp(mensaje_consola,"INICIAR_PLANIFICACION") == 0){
        return INICIAR_PLANIFICACION;
    }
    if(strcmp(mensaje_consola,"DETENER_PLANIFICACION") == 0){
        return DETENER_PLANIFICACION;
    }
    if(strcmp(mensaje_consola,"MULTIPROGRAMACION") == 0){
        return MULTIPROGRAMACION;
    }
    if(strcmp(mensaje_consola,"PROCESO_ESTADO") == 0){
        return PROCESO_ESTADO;
    }
    else
        return ERROR;
}

void leer_consola()
{
	char *linea;
    
    
    while (1) {
        linea = readline(">");
        
        if (!linea) {
            break;
        }
        if (linea) {
            add_history(linea);
            char** argumentos = string_split(linea, " ");
            t_mensajes_consola mensaje_consola;
            mensaje_consola = mensaje_a_consola(argumentos[0]);                            

            switch(mensaje_consola){
                case EJECUTAR_SCRIPT:
                    printf("EJECUTAR_SCRIPT\n");
                    break;
                case INICIAR_PROCESO:
                    //TODO: habría que verificar que siempre pasen el PATH. 
                    t_buffer* buffer = crear_buffer();
                    cargar_string_a_buffer(buffer, argumentos[1]); //PATH     
                    iniciar_proceso(buffer);

                    break;
                case FINALIZAR_PROCESO: 
                    printf("FINALIZAR_PROCESO\n");
                    t_buffer* buffer_finalizar_proceso = crear_buffer();
                    int pids = atoi(argumentos[1]);
                    cargar_int_a_buffer(buffer_finalizar_proceso, pids);                    
                    finalizar_proceso(buffer_finalizar_proceso);

                    break;
                case INICIAR_PLANIFICACION:
                    log_info(kernel_logger, "INICIAR_PLANIFICACION\n");
	                iniciar_planificacion();           

                    break;
                case DETENER_PLANIFICACION:
                    detener_planificacion();
                    log_info(kernel_logger, "DETENER_PLANIFICACION\n");
                    
                    break; 
                case MULTIPROGRAMACION:
                    printf("MULTIPROGRAMACION\n");
                    t_buffer* buffer_cambiar_multi = crear_buffer();
                    int nuevo_multi = atoi(argumentos[1]);
                    cargar_int_a_buffer(buffer_cambiar_multi, nuevo_multi); 
                    cambiar_grado_multiprogramacion(buffer_cambiar_multi);

                    
                    break;
                case PROCESO_ESTADO:
                    printf("PROCESO_ESTADO\n");
                    break;                
                case ERROR:
                    printf("Este comando es invalido\n");
                    break;               

            }           
        
        free(linea);
    }
    
}

}


//REVISAR
void ejecutar_script(t_buffer* buffer){ 
    char* script = extraer_string_del_buffer(buffer);
    printf("El script es: %s\n", script);
    free(script);
}


void cambiar_grado_multiprogramacion(t_buffer* buffer){
    
    int nueva_multi= extraer_int_del_buffer(buffer); 
    int vieja_multi = GRADO_MULTIPROGRAMACION;
    printf("viejo grado: %d\n", GRADO_MULTIPROGRAMACION);         

    destruir_buffer(buffer);   

    GRADO_MULTIPROGRAMACION = nueva_multi;
    
    printf("nuevo grado: %d\n", GRADO_MULTIPROGRAMACION);  

    resetear_semaforos_multi(vieja_multi);
}


void iniciar_proceso(t_buffer* buffer){    
    char* path = extraer_string_del_buffer(buffer); 
    printf("El path del proceso a iniciar es: %s\n", path);     

           
    destruir_buffer(buffer);

    int pid = asignar_pid();
    crear_pcb(pid);

    //Le aviso a la memoria que voy a iniciar un proceso [int pid] [char* path] [int size]
    t_buffer* buffer_memoria = crear_buffer();
    cargar_int_a_buffer(buffer_memoria, pid);
    cargar_string_a_buffer(buffer_memoria, path);
    
    t_paquete* paquete_memoria = crear_paquete(CREAR_PROCESO_KM, buffer_memoria);
    enviar_paquete(paquete_memoria, conexion_k_memoria);
}
