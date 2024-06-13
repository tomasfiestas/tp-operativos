#include "memoria.h"
#include "memoria_usuario.h"
#include <string.h>
#include <commons/collections/list.h>
#include <bits/pthreadtypes.h>

extern t_log* logger;



int main(int argc, char *argv[])
{

    // Inicio el logger de la memoria
    logger = iniciar_logger("memoria.log", "LOGGER_MEMORIA");

    // Inicio la configuracion de la memoria
    memoria_config = iniciar_config("memoria.config");

    // Obtengo los valores de la configuracion
    PUERTO_ESCUCHA = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
    log_info(logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
    TAM_MEMORIA = config_get_string_value(memoria_config, "TAM_MEMORIA");
    log_info(logger, "TAM_MEMORIA: %s", TAM_MEMORIA);
    TAM_PAGINA = config_get_string_value(memoria_config, "TAM_PAGINA");
    log_info(logger, "TAM_PAGINA: %s", TAM_PAGINA);
    PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
    log_info(logger, "PATH_INSTRUCCIONES: %s", PATH_INSTRUCCIONES);
    RETARDO_RESPUESTA = config_get_string_value(memoria_config, "RETARDO_RESPUESTA");
    log_info(logger, "RETARDO_RESPUESTA: %s", RETARDO_RESPUESTA);

    inicializar_memoria();
    procesos = list_create();

    // Inicio servidor Memoria
    int servidor_memoria = iniciar_servidor(PUERTO_ESCUCHA);
    log_info(logger, "Servidor de memoria iniciado ");    
    
     //Espero conexion de CPU
    cliente_cpu = esperar_cliente(servidor_memoria); 
    //Atiendo mensajes de CPU
    pthread_t hilo_cpu;
    int* socket_cliente_cpu_ptr = malloc(sizeof(int));
    *socket_cliente_cpu_ptr = cliente_cpu;
    pthread_create(&hilo_cpu, NULL, atender_cpu, socket_cliente_cpu_ptr);
    pthread_detach(hilo_cpu);
    log_info(logger, "Atendiendo mensajes de CPU");   
    
    
    //Espero conexion de kernel
    int cliente_kernel = esperar_cliente(servidor_memoria);   

    //Atiendo mensajes de Kernel
    pthread_t hilo_kernel;
    int* socket_cliente_kernel_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_ptr = cliente_kernel;
    pthread_create(&hilo_kernel, NULL, atender_kernel, socket_cliente_kernel_ptr);
    pthread_detach(hilo_kernel);
    log_info(logger, "Atendiendo mensajes de Kernel");  
     
    //Espero conexion de entrada/salida
    int cliente_entradasalida = esperar_cliente(servidor_memoria);   

    //Atiendo mensajes de Entrada/Salida
    pthread_t hilo_entradasalida;
    int* socket_cliente_entradasalida_ptr = malloc(sizeof(int));
    *socket_cliente_entradasalida_ptr = cliente_entradasalida;
    pthread_create(&hilo_entradasalida, NULL,atender_entradasalida, socket_cliente_entradasalida_ptr);
    log_info(logger, "Atendiendo mensajes de Entrada/Salida");
    pthread_join(hilo_entradasalida);
    
    
    
    

   

   

     



}

void atender_cpu(void* socket_cliente_ptr) {
    int cliente = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
   while (control_key){
    op_code op_code = recibir_operacion(cliente);    
	switch(op_code) {
    case SOLICITUD_INST:
        log_info(logger, "Solicitud de instrucciones");
        t_buffer *buffer = recibir_buffer(cliente);

        // // TODO: En la entrega 3 tendremos que cargar en memoria
        // // y luego buscaremos PCBs por PID creo?
        // int pid = extraer_int_del_buffer(buffer);

        // t_buffer *response_buffer = crear_buffer();
        // cargar_instrucciones_a_buffer(response_buffer, instrucciones_a_enviar);
        // t_paquete *response = crear_paquete(SOLICITUD_INST_OK, response_buffer);

        // enviar_paquete(response, cliente);

        // // Con este codigo es posible recibir paquete y extraer del buffer:
        // t_buffer* buffer = recibir_buffer(<socket que envia el paquete>);
        // t_instrucciones* instrucciones_del_buffer = extraer_instrucciones_del_buffer(buffer);
        break;
    case HANDSHAKE_KERNEL:
        log_info(logger, "Se conecto el Kernel");
        break;
    case HANDSHAKE_CPU:
        log_info(logger, "Se conecto el CPU");
        break;
    case HANDSHAKE_MEMORIA:
        log_info(logger, "Se conecto la Memoria");
        break;
    case HANDSHAKE_ES:
        log_info(logger, "Se conecto el IO");
        break;
    default:
        log_error(logger, "No se reconoce el handshake");
        control_key = 0;
        break;
	}   } 
}

void atender_kernel(void* socket_cliente_ptr){
    int cliente_k = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);    
    bool control_key = 1;
    while (control_key){        
        op_code op_code = recibir_operacion(cliente_k);
        switch (op_code){
            case HANDSHAKE_KERNEL:
			    log_info(logger, "Se conecto el Kernel");
			    break;
		    case CREAR_PROCESO_KM:
                t_buffer* buffer = recibir_buffer(cliente_k);
			    log_info(logger, "Creamos procesos");                
                atender_crear_proceso(buffer);
                break;
            case FINALIZAR_PROCESO:
                log_info(logger, "Solicitud de finalizacion de proceso");
                t_buffer* buffer = recibir_buffer(cliente_k);
                atender_eliminar_proceso(buffer);
                break;
		    default:
			    log_error(logger, "No se reconoce el handshake");
			    control_key = 0;
		    	break;			
        }
    }
}

void atender_entradasalida(void* socket_cliente_ptr){
    int cliente_es = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);    
    bool control_key = 1;
    while (control_key){
        op_code op_code = recibir_operacion(cliente_es);
        switch (op_code){
            case HANDSHAKE_KERNEL:
			log_info(logger, "Se conecto el Kernel");
			break;
		case HANDSHAKE_CPU:
			log_info(logger, "Se conecto el CPU");
            break;
		case HANDSHAKE_MEMORIA:
			log_info(logger, "Se conecto la Memoria");
			break;
		case HANDSHAKE_ES:
			log_info(logger, "Se conecto el IO");
			break;
		default:
			log_error(logger, "No se reconoce el handshake");
			control_key = 0;
			break;
        }
    }
}

t_list* parse_file(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        log_error(logger, "No se pudo abrir el archivo de instrucciones.");
        return;
    }

    char linea[256];

    t_list* instrucciones = list_create();
    int cantidad_instrucciones = 0;
    while (fgets(linea, sizeof(linea), file) != NULL) {
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        char* token = strtok(linea, " ");
        
        instruccion->operacion = (op_code) token;

        instruccion->parametros = list_create();
        while ((token = strtok(NULL, " ")) != NULL) {
            token[strcspn(token, "\n")] = 0;
            char* parametro = strdup(token); 
            list_add(instruccion->parametros, parametro);
        }

        list_add(instrucciones, instruccion);
        cantidad_instrucciones++;
    }
    
    fclose(file);

    return instrucciones;
}

void atender_crear_proceso(t_buffer* buffer){
    int pid = extraer_int_del_buffer(buffer);
    char* filename = extraer_string_del_buffer(buffer);   
    log_info(logger, "PID: %d ,Filename: %s",pid, filename);

    destruir_buffer(buffer);

    char* path = malloc(strlen(PATH_INSTRUCCIONES) + strlen(filename) + 2);
    sprintf(path, "%s/%s", PATH_INSTRUCCIONES, filename);
    free(filename);

    t_list* instrucciones = parse_file(path);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->pc = 0;
    proceso->instrucciones = instrucciones;
    
    iniciar_tabla_paginas(proceso);

    list_add(procesos, proceso);

    free(path);
}

void atender_eliminar_proceso(t_buffer* buffer){
    int pid = extraer_int_del_buffer(buffer);
    log_info(logger, "Destruyendo proceso PID: %d", pid);
    destruir_buffer(buffer);
    finalizar_proceso(pid);
}