#include "kernel.h"
int main(int argc, char* argv[]) { 

    //Inicio el logger del kernel
    kernel_logger = iniciar_logger("kernel.log", "LOGGER_KERNEL");  

    //Inicio la configuracion del kernel
    kernel_config = iniciar_config("kernel.config");
    puerto_escucha = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
    ip_memoria = config_get_string_value(kernel_config, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(kernel_config, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(kernel_config, "IP_CPU");
    puerto_cpu = config_get_string_value(kernel_config, "PUERTO_CPU");
    

    log_info(kernel_logger, "PUERTO_ESCUCHA %s", puerto_escucha); 

    //Inicio el cliente para cpu
    int conexion_cpu = crear_conexion_cliente(ip_cpu, puerto_cpu);

    
    //Inicio el cliente para memoria
    int conexion_memoria = crear_conexion_cliente(ip_memoria,puerto_memoria);

    //Inicio el servidor
    int servidor = iniciar_servidor(puerto_escucha,kernel_logger);

    //Espero a los clientes
    int clientes = esperar_cliente(servidor,kernel_logger); 
}


