#include "cpu.h"

extern t_log* logger;

int main(int argc, char* argv[]) {
    //Inicio el logger de la cpu
    logger = iniciar_logger("cpu.log", "LOGGER_CPU");
    
    //Inicio la configuracion de la cpu
    cpu_config = iniciar_config("cpu.config");

    //Obtengo los valores de la configuracion
    IP_MEMORIA = config_get_string_value(cpu_config, "IP_MEMORIA");
    log_info(logger, "IP_MEMORIA: %s", IP_MEMORIA);
    PUERTO_MEMORIA = config_get_string_value(cpu_config, "PUERTO_MEMORIA");
    log_info(logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(cpu_config, "PUERTO_ESCUCHA_DISPATCH");
    log_info(logger, "PUERTO_ESCUCHA_DISPATCH: %s", PUERTO_ESCUCHA_DISPATCH);
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpu_config, "PUERTO_ESCUCHA_INTERRUPT");
    log_info(logger, "PUERTO_ESCUCHA_INTERRUPT: %s", PUERTO_ESCUCHA_INTERRUPT);
    CANTIDAD_ENTRADAS_TLB = config_get_int_value(cpu_config, "CANTIDAD_ENTRADAS_TLB");
    log_info(logger, "CANTIDAD_ENTRADAS_TLB: %d", CANTIDAD_ENTRADAS_TLB);
    ALGORITMO_TLB = config_get_string_value(cpu_config, "ALGORITMO_TLB");
    log_info(logger, "ALGORITMO_TLB: %s", ALGORITMO_TLB);


    log_info(logger, "________________________________");

    //Inicio el servidor de la cpu
    int servidor_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH);
    int servidor_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT);

    //Inicio la conexion como cliente con la memoria
    conexion_memoria = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(logger, "Conexion con memoria establecida");  
    
    realizar_handshake(HANDSHAKE_CPU,conexion_memoria);
    log_info(logger, "Handshake con Memoria realizado");
    
    //Espero al cliente Kernel - Dispatch
    int cliente_kernel_dispatch = esperar_cliente(servidor_dispatch); 
    //Atender los mensajes de Kernel - Dispatch
    pthread_t hilo_kernel_dispatch;
    int* socket_cliente_kernel_disptach_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_disptach_ptr = cliente_kernel_dispatch;
    pthread_create(&hilo_kernel_dispatch, NULL, atender_kernel_dispatch, socket_cliente_kernel_disptach_ptr);
    pthread_detach(hilo_kernel_dispatch);
    log_info(logger, "Atendiendo mensajes de Kernel Dispatch");       

    //Espero al cliente Kernel - Interrupt
    int cliente_kernel_interrupt = esperar_cliente(servidor_interrupt);    
    

    //Atender los mensajes de Kernel - Interrupt
    pthread_t hilo_kernel_interrupt;
    int* socket_cliente_kernel_interrupt_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_interrupt_ptr = cliente_kernel_interrupt;
    pthread_create(&hilo_kernel_interrupt, NULL, atender_kernel_interrupt, socket_cliente_kernel_interrupt_ptr);
    log_info(logger, "Atendiendo mensajes de Kernel Interrupt");
    pthread_join(hilo_kernel_interrupt);
    


    //realizar_handshake(CPU,conexion_memoria);

    //realizar_handshake(CPU,conexion_memoria);
    //log_info(logger, "Handshake con Memoria realizado");
    

    //Atender los mensajes de Memoria  

    return EXIT_SUCCESS;


}

void atender_kernel_dispatch(void* socket_cliente_ptr) {
    int cliente_kd = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
    while (control_key){
    op_code op_code = recibir_operacion(cliente_kd);    
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

void atender_kernel_interrupt(void* socket_cliente_ptr) {
    int cliente_ki = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
    while (control_key){
    op_code handshake = recibir_operacion(cliente_ki);    
	switch(handshake) {
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


