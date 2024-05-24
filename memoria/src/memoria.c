#include "memoria.h"
#include <string.h>

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
    void* memoria_total = reservar_memoria();
    tabla_paginas* tabla = iniciar_tabla_paginas(memoria_total);

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
    case SOLICITUD_INST:
        log_info(logger, "Solicitud de instrucciones");
        t_buffer *buffer = recibir_buffer(cliente);

        // TODO: En la entrega 3 tendremos que cargar en memoria
        // y luego buscaremos PCBs por PID creo?
        int pid = extraer_int_del_buffer(buffer);

        t_buffer *response_buffer = crear_buffer();
        cargar_instrucciones_a_buffer(response_buffer, instrucciones_a_enviar);
        t_paquete *response = crear_paquete(SOLICITUD_INST_OK, buffer);

        // TODO: ver como enviar paquete al cliente.
        enviar_paquete(response, cliente);

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

void parse_file(const char* filePath, int pid) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        log_error(logger, "No se pudo abrir el archivo de instrucciones.");
        return;
    }

    char linea[256];

    // TODO: Puede ser insignificante, pero capaz se puede cambiar este
    // espacio fijo por uno que arranque en 10 y escale a medida sea necesario.
    t_instruccion instrucciones[200];
    int cantidad_instrucciones = 0;
    while (fgets(linea, sizeof(linea), file) != NULL) {
        t_instruccion instruccion;
        char* token = strtok(linea, " ");
        
        size_t length = strlen(token);
        instruccion.instruccion = malloc(length + 1);
        instruccion.instruccion_longitud = length;
        strncpy(instruccion.instruccion, token, length + 1);

        instruccion.parametros = NULL;
        instruccion.parametros_cantidad = 0;
        while ((token = strtok(NULL, " ")) != NULL) {
            token[strcspn(token, "\n")] = 0;
            t_parametro parametro;

            size_t parametro_length = strlen(token);
            parametro.parametro = malloc(parametro_length + 1);
            parametro.longitud = parametro_length;
            strncpy(parametro.parametro, token, parametro_length + 1);

            instruccion.parametros_cantidad++;
            instruccion.parametros = realloc(instruccion.parametros, instruccion.parametros_cantidad * sizeof(t_parametro));

            instruccion.parametros[instruccion.parametros_cantidad - 1] = parametro;

        }

        log_instruccion(instruccion);

        instrucciones[cantidad_instrucciones] = instruccion;
        cantidad_instrucciones++;

    }

    instrucciones_a_enviar = {
         .pid = pid,
         .instrucciones = instrucciones, 
         .cantidad = cantidad_instrucciones
    };
    
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

tabla_paginas* iniciar_tabla_paginas(void* memoria) {
    int cantidad_paginas = atoi(TAM_MEMORIA) / atoi(TAM_PAGINA);
    tabla_paginas* tabla = (tabla_paginas*)memoria;
    tabla->registros = (registro_tabla_paginas*)((char*)memoria + sizeof(tabla_paginas));
    tabla->size = cantidad_paginas;
    for (int i = 0; i < cantidad_paginas; i++) {
        tabla->registros[i].numeroFrame = i;
        tabla->registros[i].estaPresente = false;
    }
    return tabla;
}

void atender_crear_proceso(t_buffer* buffer){
    int pid = extraer_int_del_buffer(buffer);
    char* filename = extraer_string_del_buffer(buffer);   
    log_info(logger, "PID: %d ,Filename: %s",pid, filename);    
    destruir_buffer(buffer);

    char* path = malloc(strlen(PATH_INSTRUCCIONES) + strlen(filename) + 2);
    sprintf(path, "%s/%s", PATH_INSTRUCCIONES, filename);
    free(filename);

    parse_file(path, pid);
    free(path);
}

void log_instruccion(t_instruccion instruccion) {
    char* log_message = malloc(strlen(instruccion.instruccion) + 20);
    sprintf(log_message, "Instruccion: %s", instruccion.instruccion);

    for (int i = 0; i < instruccion.parametros_cantidad; i++) {
        char* old_log_message = log_message;
        log_message = malloc(strlen(old_log_message) + strlen(instruccion.parametros[i].parametro) + 5);
        sprintf(log_message, "%s %s", old_log_message, instruccion.parametros[i].parametro);
        free(old_log_message);
    }

    log_info(logger, "%s", log_message);
    free(log_message);
}