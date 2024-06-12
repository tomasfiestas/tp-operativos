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
            char ** argumentos = string_split(linea, " ");
            t_mensajes_consola mensaje_consola;
            mensaje_consola = mensaje_a_consola(argumentos[0]); 
            log_info(kernel_logger, "Mensaje de consola: %s", argumentos[1]);                       
            procesar_mensaje(mensaje_consola, argumentos);
                      
        
        free(linea);
        free(argumentos);
        }
    }
    
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
void finalizar_proceso_por_consola(t_buffer* buffer){    
    int pid= extraer_int_del_buffer(buffer); 
    printf("El proceso a finalizar es: %d\n", pid);         
    
    destruir_buffer(buffer);   
    

    t_pcb * pcb_a_finalizar = buscarPcb(pid);
    if(pcb_a_finalizar->estado == EXEC){ 
    //Le aviso a CPU interrupt que quiero terminar un proceso
    t_buffer* buffer_cpu_interrupt = crear_buffer();
    pthread_cancel(hilo_quantum); //Detengo el hilo que cuenta el quantum
    cargar_pcb_a_buffer(buffer_cpu_interrupt, pcb_a_finalizar);
    t_paquete* paquete_cpu = crear_paquete(FINPROCESO, buffer_cpu_interrupt);
    enviar_paquete(paquete_cpu, conexion_cpu_interrupt);

    }
    else{
        sacar_pcb_de_lista(pcb_a_finalizar);
        agregar_a_exit(pcb_a_finalizar,INTERRUPTED_BY_USER);
    }
}

void procesar_mensaje(t_mensajes_consola mensaje_a_consola, char** argumentos){
    switch(mensaje_a_consola){
                case EJECUTAR_SCRIPT:
                    ejecutar_script(argumentos[1]);
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
                    finalizar_proceso_por_consola(buffer_finalizar_proceso);

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
                    t_buffer* buffer_cambiar_multi = crear_buffer();
                    int nuevo_multi = atoi(argumentos[1]);
                    cargar_int_a_buffer(buffer_cambiar_multi, nuevo_multi); 
                    cambiar_grado_multiprogramacion(buffer_cambiar_multi);
                    break;
                case PROCESO_ESTADO:
                    mostrar_pids_y_estados();
                    break;                
                case ERROR:
                    printf("Este comando es invalido\n");
                    break;               

            } 
}

//REVISAR
void ejecutar_script(char* argumentos){ 
    ejecutar_archivo(argumentos);
    printf("El script es: %s\n", argumentos);       
}

void ejecutar_archivo(const char* filePath) {
    // Abrir el archivo y obtener las instrucciones
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo de instrucciones.");
        return;
    }

    char lineas[256];
    while (fgets(lineas, sizeof(lineas), file) != NULL) {
        lineas[strcspn(lineas, "\n")] = '\0'; // Eliminar el salto de línea
        // Aquí puedes utilizar la variable "lineas" sin el salto de línea
    
        // Ejecutar cada instrucción en la consola
        // Aquí puedes llamar a la función que ejecuta una instrucción en la consola
        // Pasando la línea como parámetro
        //mensaje_a_consola(linea);
        char** argumentos_script = string_split(lineas, " ");
        t_mensajes_consola mensaje_consola;
        /*t_buffer* buffer = crear_buffer();
        cargar_string_a_buffer(buffer, argumentos_script[1]);*/
        mensaje_consola = mensaje_a_consola(argumentos_script[0]);                           
        procesar_mensaje(mensaje_consola, argumentos_script);

    
    }
    
    fclose(file);

    // Terminar la función después de leer todas las líneas
    
}