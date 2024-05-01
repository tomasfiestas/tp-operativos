#include "memoria.h"

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

    //CHECKPOINT 2: Estas estructuras por ahora no se cargaran.
    void* memoriaTotal = reservar_memoria();
    TablaPaginas* tabla = iniciar_tabla_paginas(memoriaTotal);

    // Inicio servidor Memoria
    int servidor_memoria = iniciar_servidor(PUERTO_ESCUCHA);
    log_info(logger, "Servidor de memoria iniciado ");    
    
     //Espero conexion de CPU
    int cliente_cpu = esperar_cliente(servidor_memoria); 
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

void parse_file(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        log_error(logger, "No se pudo abrir el archivo de instrucciones.");
        return;
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), file) != NULL) {
        linea[strcspn(linea, "\n")] = 0;
        InstruccionSerializada instruccion;
        char* token = strtok(linea, " ");
        strncpy(instruccion.instruction, token, TAM_MAX_INSTRUCCION);
        int i = 0;
        while ((token = strtok(NULL, " ")) != NULL && i < CANT_MAX_PARAMETRO) {
            strncpy(instruccion.parameters[i], token, TAM_MAX_PARAMETRO);
            i++;
        }
        // Enviar al CPU por ahora... pero como?? no somos clientes de CPU.

    }

    fclose(file);
}

void* reservar_memoria() {
    void* totalMemory = malloc(atoi(TAM_MEMORIA));
    if(totalMemory == NULL) {
        log_error(logger, "No se pudo reservar la memoria necesaria.");
        abort();
    }
    return totalMemory;
}

TablaPaginas* iniciar_tabla_paginas(void* memoria) {
    int cantidad_paginas = atoi(TAM_MEMORIA) / atoi(TAM_PAGINA);
    TablaPaginas* tabla = (TablaPaginas*)memoria;
    tabla->registros = (RegistroTablaPaginas*)((char*)memoria + sizeof(TablaPaginas));
    tabla->size = cantidad_paginas;
    for (int i = 0; i < cantidad_paginas; i++) {
        tabla->registros[i].numeroFrame = i;
        tabla->registros[i].estaPresente = false;
    }
    return tabla;
}

void atender_crear_proceso(t_buffer* buffer){
    int pid = extraer_int_del_buffer(buffer);
    char* path = extraer_string_del_buffer(buffer);   
    log_info(logger, "PID: %d ,Path: %s",pid, path);    
    free(path);
    destruir_buffer(buffer);

    parse_file(PATH_INSTRUCCIONES);
}