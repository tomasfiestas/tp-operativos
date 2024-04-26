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

    log_info(logger, "________________");

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
    module_code handshake = recibir_operacion(cliente);    
	switch(handshake) {
		case KERNEL:
			log_info(logger, "Se conecto el Kernel");
			break;
		case CPU:
			log_info(logger, "Se conecto el CPU");
			break;
		case MEMORIA:
			log_info(logger, "Se conecto la Memoria");
			break;
		case IO:
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
    t_buffer* buffer;
    bool control_key = 1;
    while (control_key){
        //module_code handshake = recibir_operacion(cliente_k);
        op_code handshake = recibir_operacion(cliente_k);
        switch (handshake){
            case KERNEL:
			log_info(logger, "Se conecto el Kdsddernel");
			break;
		case CREAR_PROCESO_KM:
			log_info(logger, "Creamos procesos");
            break;
        case MEMORIA:
            log_info(logger, "Se conecto la Memoria");
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
        module_code handshake = recibir_operacion(cliente_es);
        switch (handshake){
            case KERNEL:
			log_info(logger, "Se conecto el Kernel");
			break;
		case CPU:
			log_info(logger, "Se conecto el CPU");
            break;
		case MEMORIA:
			log_info(logger, "Se conecto la Memoria");
			break;
		case IO:
			log_info(logger, "Se conecto el IO");
			break;
		default:
			log_error(logger, "No se reconoce el handshake");
			control_key = 0;
			break;
        }
    }
}


void atender_crear_proceso(t_buffer* buffer){
    int pid = extraer_int_del_buffer(buffer);
    char* path = extraer_string_del_buffer(buffer);   
    log_info(logger, "PID: %d ,Path: %s",pid, path);
    printf("%s,%d",pid,path);
    free(pid);
    free(path);
    
    destruir_buffer(buffer);
}