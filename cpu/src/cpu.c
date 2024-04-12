#include "cpu.h"
int main(int argc, char* argv[]) {

    
    


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



    //Inicio el servidor de la cpu
    int servidor__dispatch_cpu = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH,cpu_logger);
    int servidor_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT,cpu_logger);


    //Inicio la conexion como cliente con la memoria
    conexion_memoria = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(cpu_logger, "Conexion con memoria establecida");
    realizar_handshake(2, conexion_memoria);

    //Espero al cliente Kernel - Dispatch
    int cliente_kernel_dispatch = esperar_cliente(servidor__dispatch_cpu,cpu_logger,"Kernel - Dispatch");

    //Atender los mensajes de Kernel - Dispatch

    //Espero al cliente Kernel - Interrupt
    int cliente_kernel_interrupt = esperar_cliente(servidor_interrupt,cpu_logger,"Kernel - Interrupt");

    //Atender los mensajes de Kernel - Interrupt

    

    //Atender los mensajes de Memoria  

    

    



    

    return EXIT_SUCCESS;


}


