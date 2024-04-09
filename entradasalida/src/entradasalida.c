#include "entradasalida.h"
int main(int argc, char* argv[]) { 
    //Inicio el logger de entradasalida 
    entradasalida_logger = iniciar_logger("entradasalida.log", "LOGGER_ENTRADASALIDA");  
    //Inicio la configuracion de entradasalida
    entradasalida_config = iniciar_config("entradasalida.config");
    TIPO_INTERFAZ = config_get_string_value(entradasalida_config, "TIPO_INTERFAZ");
    log_info(entradasalida_logger, "TIPO_ INTERFAZ: %s", TIPO_INTERFAZ); 
    TIEMPO_UNIDAD_TRABAJO = config_get_string_value(entradasalida_config, "TIEMPO_UNIDAD_TRABAJO");
    log_info(entradasalida_logger, "TIEMPO_UNIDAD_TRABAJO: %s", TIEMPO_UNIDAD_TRABAJO);
    IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
    log_info(entradasalida_logger, "IP_KERNEL: %s", IP_KERNEL);
    PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
    log_info(entradasalida_logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);
    IP_MEMORIA = config_get_string_value(entradasalida_config, "IP_MEMORIA");
    log_info(entradasalida_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    PUERTO_MEMORIA = config_get_string_value(entradasalida_config, "PUERTO_MEMORIA");
    log_info(entradasalida_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    PATH_BASE_DIALFS = config_get_string_value(entradasalida_config, "PATH_BASE_DIALFS");
    log_info(entradasalida_logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);
    BLOCK_SIZE = config_get_string_value(entradasalida_config, "BLOCK_SIZE");
    log_info(entradasalida_logger, "LOCK_SIZE: %s", BLOCK_SIZE);
    BLOCK_COUNT = config_get_string_value(entradasalida_config, "BLOCK_COUNT");
    log_info(entradasalida_logger, "BLOCK_COUNT: %s", BLOCK_COUNT);


    //Creo conexion como cliente hacia Memoria
    int conexion_memoria = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(entradasalida_logger, "Conexion con Memoria establecida");

    //Creo conexion como cliente hacia Kernel
    int conexion_kernel = crear_conexion_cliente(IP_KERNEL, PUERTO_KERNEL);
    log_info(entradasalida_logger, "Conexion con Kernel establecida");

    

    
    
	
    return EXIT_SUCCESS;
}
