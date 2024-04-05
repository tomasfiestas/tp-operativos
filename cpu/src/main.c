#include "main.h"
int main(int argc, char* argv[]) {

    int conexion;
    


    //Inicio el logger de la cpu
    cpu_logger = iniciar_logger("cpu.log", "LOGGER_CPU");
    
    //Inicio la configuracion de la cpu
    cpu_config = iniciar_config("cpu.config");

    //Obtengo los valores de la configuracion
    IP_MEMORIA = config_get_string_value(cpu_config, "IP_MEMORIA");
    log_info(cpu_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    PUERTO_MEMORIA = config_get_string_value(cpu_config, "PUERTO_MEMORIA");
    log_info(cpu_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(cpu_config, "PUERTO_ESCUCHA_DISPATCH");
    log_info(cpu_logger, "PUERTO_ESCUCHA_DISPATCH: %s", PUERTO_ESCUCHA_DISPATCH);
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpu_config, "PUERTO_ESCUCHA_INTERRUPT");
    log_info(cpu_logger, "PUERTO_ESCUCHA_INTERRUPT: %s", PUERTO_ESCUCHA_INTERRUPT);
    CANTIDAD_ENTRADAS_TLB = config_get_int_value(cpu_config, "CANTIDAD_ENTRADAS_TLB");
    log_info(cpu_logger, "CANTIDAD_ENTRADAS_TLB: %d", CANTIDAD_ENTRADAS_TLB);
    ALGORITMO_TLB = config_get_string_value(cpu_config, "ALGORITMO_TLB");
    log_info(cpu_logger, "ALGORITMO_TLB: %s", ALGORITMO_TLB);


    log_info(cpu_logger, "________________________________");


    //Inicio la conexion con la memoria
    conexion = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);

    /* enviar_mensaje("HOLA", conexion); */

       

    liberar_conexion(conexion);


}
