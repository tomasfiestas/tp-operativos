#include "kernel.h"

extern t_log* logger;
int main(int argc, char* argv[]) { 

    //Inicio el logger del kernel
    logger = iniciar_logger("kernel.log", "LOGGER_KERNEL");  

    //Inicio la configuracion del kernel
    kernel_config = iniciar_config("kernel.config");
    PUERTO_ESCUCHA = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
    log_info(logger, "PUERTO_ESCUCHA %s", PUERTO_ESCUCHA);
    IP_MEMORIA = config_get_string_value(kernel_config, "IP_MEMORIA");
    log_info(logger, "IP_MEMORIA %s", IP_MEMORIA);
    PUERTO_MEMORIA = config_get_string_value(kernel_config, "PUERTO_MEMORIA");
    log_info(logger, "PUERTO_MEMORIA %s", PUERTO_MEMORIA);    
    IP_CPU = config_get_string_value(kernel_config, "IP_CPU");
    log_info(logger, "IP_CPU %s", IP_CPU);
    PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config, "PUERTO_CPU_DISPATCH");
    log_info(logger, "PUERTO_CPU_DISPATCH %s", PUERTO_CPU_DISPATCH);
    PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config, "PUERTO_CPU_INTERRUPT");
    log_info(logger, "PUERTO_CPU_INTERRUPT %s", PUERTO_CPU_INTERRUPT);
    ALGORITMO_PLANIFICACION = config_get_string_value(kernel_config, "ALGORITMO_PLANIFICACION");
    log_info(logger, "ALGORITMO_PLANIFICACION %s", ALGORITMO_PLANIFICACION);
    QUANTUM = config_get_string_value(kernel_config, "QUANTUM");
    log_info(logger, "QUANTUM %s", QUANTUM);
    RECURSOS = config_get_array_value(kernel_config, "RECURSOS");
    log_info(logger, "RECURSOS: %s|%s|%s",RECURSOS[0],RECURSOS[1],RECURSOS[2]);
    INSTANCIAS_RECURSOS = config_get_array_value(kernel_config, "INSTANCIAS_RECURSOS");
    log_info(logger, "INSTANCIAS_RECURSOS %s|%s|%s", INSTANCIAS_RECURSOS[0], INSTANCIAS_RECURSOS[1], INSTANCIAS_RECURSOS[2]);
    GRADO_MULTIPROGRAMACION = config_get_string_value(kernel_config, "GRADO_MULTIPROGRAMACION");
    log_info(logger, "GRADO_MULTIPROGRAMACION %s", GRADO_MULTIPROGRAMACION);
    
    
   
    //Inicio el cliente para cpu dispatch
    int conexion_cpu_dispatch = crear_conexion_cliente(IP_CPU, PUERTO_CPU_DISPATCH);
    
    realizar_handshake(KERNEL, conexion_cpu_dispatch);
    log_info(logger,"Handshake con CPU realizado");
    
    //Inicio el cliente para cpu interrupt
    int conexion_cpu_interrupt = crear_conexion_cliente(IP_CPU, PUERTO_CPU_INTERRUPT);
    realizar_handshake(KERNEL, conexion_cpu_interrupt);
    log_info(logger,"Handshake con CPU realizado");
    
    //Inicio el cliente para memoria
    int conexion_k_memoria = crear_conexion_cliente(IP_MEMORIA,PUERTO_MEMORIA);
    realizar_handshake(KERNEL, conexion_k_memoria);
    log_info(logger,"Handshake con Memoria realizado");
    //Inicio el servidor
    int servidor = iniciar_servidor(PUERTO_ESCUCHA);
    
    //Espero a los clientes
    int cliente_entradasalida = esperar_cliente(servidor); 

    
    

    //Atiendo mensajes de Entrada/Salida
    pthread_t hilo_entradasalida;
    int* socket_cliente_entradasalida2_ptr = malloc(sizeof(int));
    *socket_cliente_entradasalida2_ptr = cliente_entradasalida;
    pthread_create(&hilo_entradasalida, NULL,atender_entradasalida2, socket_cliente_entradasalida2_ptr);
    log_info(logger, "Atendiendo mensajes de Entrada/Salida");
    
    //Leer consola
    leer_consola();

    pthread_join(hilo_entradasalida);
    
    
    



    return EXIT_SUCCESS;
}

void atender_entradasalida2(void* socket_cliente_ptr){
    int cliente_entradasalida2 = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
    while (control_key){
        module_code handshake = recibir_operacion(cliente_entradasalida2);
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


