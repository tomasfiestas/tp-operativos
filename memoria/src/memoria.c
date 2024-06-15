#include "memoria.h"
#include "memoria_usuario.h"
#include <string.h>
#include <commons/collections/list.h>
#include <bits/pthreadtypes.h>

t_log* memoria_logger;
t_config* memoria_config;
t_list* procesos;

char* PUERTO_ESCUCHA;
char* TAM_MEMORIA;
char* TAM_PAGINA;
char* PATH_INSTRUCCIONES;
char* RETARDO_RESPUESTA;

int cliente_entradasalida;
int cliente_kernel;
int cliente_cpu;
int cantidad_procesos;

int main(int argc, char *argv[])
{
    // Inicio el logger de la memoria
    memoria_logger = iniciar_logger("memoria.log", "LOGGER_MEMORIA");

    // Inicio la configuracion de la memoria
    memoria_config = iniciar_config("memoria.config");

    // TODO: Charlar tema de comunicacion antes de la entrega.
    // t_list* test_instrucciones = list_create();
    // t_instruccion test_instruccion_1 = { .operacion = RESIZE, .parametros = list_create() };
    // char* parametro_1 = "AX";
    // char* parametro_2 = "BX";
    // list_add(test_instruccion_1.parametros, parametro_1);
    // list_add(test_instruccion_1.parametros, parametro_2);

    // t_instruccion test_instruccion_2 = { .operacion = IO_FS_CREATE, .parametros = list_create() };
    // char* parametro_3 = "CX";
    // char* parametro_4 = "DX";
    // list_add(test_instruccion_2.parametros, parametro_3);
    // list_add(test_instruccion_2.parametros, parametro_4);

    // t_pagina* test_pagina = malloc(sizeof(t_pagina));
    // test_pagina->frame = 0;
    // test_pagina->presente = true;

    // t_pagina* test_pagina_2 = malloc(sizeof(t_pagina));
    // test_pagina->frame = 1;
    // test_pagina->presente = true;

    // t_proceso* test_proceso = malloc(sizeof(t_proceso));
    // test_proceso->pid = 1;
    // test_proceso->pc = 0;
    // test_proceso->instrucciones = list_create();
    // test_proceso->paginas = list_create();

    // list_add(test_proceso->instrucciones, &test_instruccion_1);
    // list_add(test_proceso->instrucciones, &test_instruccion_2);

    // list_add(test_proceso->paginas, test_pagina);
    // list_add(test_proceso->paginas, test_pagina_2);


    // t_instruccion* instruccion = list_get(test_proceso->instrucciones, test_proceso->pc);
    // test_proceso->pc++;

    // t_buffer* response_buffer = crear_buffer();
    // cargar_instruccion_a_buffer(response_buffer, instruccion);
    // t_paquete* response = crear_paquete(SOLICITUD_INST_OK, response_buffer);

    // //TEST:
    // t_instruccion instruccion_buffer = extraer_instruccion_del_buffer(response_buffer);


    

    // Obtengo los valores de la configuracion
    PUERTO_ESCUCHA = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
    log_info(memoria_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA);
    TAM_MEMORIA = config_get_string_value(memoria_config, "TAM_MEMORIA");
    log_info(memoria_logger, "TAM_MEMORIA: %s", TAM_MEMORIA);
    TAM_PAGINA = config_get_string_value(memoria_config, "TAM_PAGINA");
    log_info(memoria_logger, "TAM_PAGINA: %s", TAM_PAGINA);
    PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
    log_info(memoria_logger, "PATH_INSTRUCCIONES: %s", PATH_INSTRUCCIONES);
    RETARDO_RESPUESTA = config_get_string_value(memoria_config, "RETARDO_RESPUESTA");
    log_info(memoria_logger, "RETARDO_RESPUESTA: %s", RETARDO_RESPUESTA);

    inicializar_memoria();
    inicializar_bitmap();
    procesos = list_create();

    // Inicio servidor Memoria
    int servidor_memoria = iniciar_servidor(PUERTO_ESCUCHA);
    log_info(memoria_logger, "Servidor de memoria iniciado ");    
    
     //Espero conexion de CPU
    cliente_cpu = esperar_cliente(servidor_memoria); 
    //Atiendo mensajes de CPU
    pthread_t hilo_cpu;
    int* socket_cliente_cpu_ptr = malloc(sizeof(int));
    *socket_cliente_cpu_ptr = cliente_cpu;
    pthread_create(&hilo_cpu, NULL, atender_cpu, socket_cliente_cpu_ptr);
    pthread_detach(hilo_cpu);
    log_info(memoria_logger, "Atendiendo mensajes de CPU");   
    
    
    //Espero conexion de kernel
    int cliente_kernel = esperar_cliente(servidor_memoria);   

    //Atiendo mensajes de Kernel
    pthread_t hilo_kernel;
    int* socket_cliente_kernel_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_ptr = cliente_kernel;
    pthread_create(&hilo_kernel, NULL, atender_kernel, socket_cliente_kernel_ptr);
    pthread_detach(hilo_kernel);
    log_info(memoria_logger, "Atendiendo mensajes de Kernel");  
     
    //Espero conexion de entrada/salida
    int cliente_entradasalida = esperar_cliente(servidor_memoria);   

    //Atiendo mensajes de Entrada/Salida
    pthread_t hilo_entradasalida;
    int* socket_cliente_entradasalida_ptr = malloc(sizeof(int));
    *socket_cliente_entradasalida_ptr = cliente_entradasalida;
    pthread_create(&hilo_entradasalida, NULL,atender_entradasalida, socket_cliente_entradasalida_ptr);
    log_info(memoria_logger, "Atendiendo mensajes de Entrada/Salida");
    pthread_join(hilo_entradasalida, NULL);
    
    
    
    

   

   

     



}

void* atender_cpu(void* socket_cliente_ptr) {
    int cliente = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
   while (control_key){
    op_code op_code = recibir_operacion(cliente);    
	switch(op_code) {
    case SOLICITUD_INST:
        log_info(memoria_logger, "Solicitud de instrucciones");
        t_buffer *buffer = recibir_buffer(cliente);

        int pid = extraer_int_del_buffer(buffer);

        destruir_buffer(buffer);

        t_proceso* proceso = obtener_proceso(pid);
        if (proceso == NULL) {
            log_error(memoria_logger, "No se encontro el proceso con PID: %d", pid);
            break;
        }
        t_instruccion* instruccion = list_get(proceso->instrucciones, proceso->pc);
        proceso->pc++;

        t_buffer* response_buffer = crear_buffer();
        cargar_instruccion_a_buffer(response_buffer, instruccion);
        t_paquete* response = crear_paquete(SOLICITUD_INST_OK, response_buffer);

        break;
    case HANDSHAKE_KERNEL:
        log_info(memoria_logger, "Se conecto el Kernel");
        break;
    case HANDSHAKE_CPU:
        log_info(memoria_logger, "Se conecto el CPU");
        break;
    case HANDSHAKE_MEMORIA:
        log_info(memoria_logger, "Se conecto la Memoria");
        break;
    case HANDSHAKE_ES:
        log_info(memoria_logger, "Se conecto el IO");
        break;
    default:
        log_error(memoria_logger, "No se reconoce el handshake");
        control_key = 0;
        break;
	}   } 
    return NULL;
}

void* atender_kernel(void* socket_cliente_ptr){
    int cliente_k = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);    
    bool control_key = 1;
    while (control_key){        
        op_code op_code = recibir_operacion(cliente_k);
        t_buffer* buffer;
        switch (op_code){
            case HANDSHAKE_KERNEL:
			    log_info(memoria_logger, "Se conecto el Kernel");
			    break;
		    case CREAR_PROCESO_KM:
                buffer = recibir_buffer(cliente_k);
			    log_info(memoria_logger, "Creamos procesos");                
                atender_crear_proceso(buffer);
                break;
            case FINALIZAR_PROCESO_KM:
                log_info(memoria_logger, "Solicitud de finalizacion de proceso");
                buffer = recibir_buffer(cliente_k);
                atender_eliminar_proceso(buffer);
                break;
		    default:
			    log_error(memoria_logger, "No se reconoce el handshake");
			    control_key = 0;
		    	break;			
        }
    }
    return NULL;
}

void* atender_entradasalida(void* socket_cliente_ptr){
    int cliente_es = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);    
    bool control_key = 1;
    while (control_key){
        op_code op_code = recibir_operacion(cliente_es);
        switch (op_code){
            case HANDSHAKE_KERNEL:
			log_info(memoria_logger, "Se conecto el Kernel");
			break;
		case HANDSHAKE_CPU:
			log_info(memoria_logger, "Se conecto el CPU");
            break;
		case HANDSHAKE_MEMORIA:
			log_info(memoria_logger, "Se conecto la Memoria");
			break;
		case HANDSHAKE_ES:
			log_info(memoria_logger, "Se conecto el IO");
			break;
		default:
			log_error(memoria_logger, "No se reconoce el handshake");
			control_key = 0;
			break;
        }
    }
    return NULL;
}

t_list* parse_file(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        log_error(memoria_logger, "No se pudo abrir el archivo de instrucciones.");
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
    log_info(memoria_logger, "PID: %d ,Filename: %s",pid, filename);

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

    // TEST:
    // finalizar_proceso(pid);
    // resize(pid, 128);
    // resize(pid, 32);
}

void atender_eliminar_proceso(t_buffer* buffer){
    int pid = extraer_int_del_buffer(buffer);
    log_info(memoria_logger, "Destruyendo proceso PID: %d", pid);
    destruir_buffer(buffer);
    finalizar_proceso(pid);
}

t_proceso *obtener_proceso(int pid)
{
    t_list_iterator *iterator = list_iterator_create(procesos);
    while (list_iterator_has_next(iterator))
    {
        t_proceso *proceso = list_iterator_next(iterator);
        if (proceso->pid == pid)
        {
            list_iterator_destroy(iterator);
            return proceso;
        }
    }
    list_iterator_destroy(iterator);
    return NULL;
}