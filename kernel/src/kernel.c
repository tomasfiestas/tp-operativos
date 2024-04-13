#include "kernel.h"
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

    //Inicio el cliente para cpu interrupt
    int conexion_cpu_interrupt = crear_conexion_cliente(IP_CPU, PUERTO_CPU_INTERRUPT);
    realizar_handshake(KERNEL, conexion_cpu_interrupt);
    
    //Inicio el cliente para memoria
    int conexion_memoria = crear_conexion_cliente(IP_MEMORIA,PUERTO_MEMORIA);
    realizar_handshake(KERNEL, conexion_memoria);
    
    //Inicio el servidor
    int servidor = iniciar_servidor(PUERTO_ESCUCHA);

    //Espero a los clientes
    int clientes = esperar_cliente(servidor); 


    return EXIT_SUCCESS;
}


