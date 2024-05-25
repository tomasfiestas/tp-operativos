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
    
        
    //Atender los mensajes de Memoria  
    return EXIT_SUCCESS;
}

//----------------------
//       Kernel
//----------------------


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

    pthread_t conexion_cpu_dispatch;
    pthread_create(&conexion_cpu_dispatch, NULL, (void*)esperar_contextos, &cliente_kernel_dispatch);

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


void esperar_contextos(int *socket_cliente_kernel){
    while(1){

        t_contexto_ejecucion *ctx;
        codigo_operacion cod_op = recibir_operacion(*socket_cliente_kernel);

        switch (cod_op)
        {
        case CONTEXTO_EJECUCION:
            
            ctx = recibir_contexto(*socket_cliente_kernel);
            empezar_ciclo_instruccion(ctx);

            break;
        
        default:
            log_error(cpu, "No se pudo reconocer la operacion");
            break;
        }
    }
}


void devolver_contexto(t_contexto_ejecucion *ctx,op_code cod_operacion)
{
    t_paquete *paquete = crear_paquete(cod_operacion);

    serializar_contexto(paquete,ctx);

    enviar_paquete(paquete, cliente_kernel_dispatch);
}


//To do: completar los demas contextos



//----------------------
//       Memoria
//----------------------

//Separar en archivos

int conexion_memoria;

void atender_memoria(char *ip_memoria, char *puerto_memoria){
    
    conexion_memoria = crear_conexion_cliente(ip_memoria, puerto_memoria);

    realizar_handshake(2, conexion_memoria);

    log_info(cpu, "Se conecto con memoria");

    if(recibir_operacion(conexion_memoria) == OK){
        log_info(cpu, "Memoria envio un OK");
    }else{
        log_error(cpu, "No se recibio el tamanio de la pagina de manera correcta");
    }

    //creo buffer y recibo operacion de memoria

    t_buffer *buffer = crear_buffer();
    buffer->stream = recibir_buffer(&(buffer->size), conexion_memoria);

    log_info(cpu, "Recibio la operacion de memoria");

    memcpy(&tam_pag, buffer->stream, sizeof(int));

    destruir_buffer(buffer);

}


char* leer_de_memoria(int direc_fisica, int pid)
{
    t_buffer *buffer = crear_buffer();
    t_paquete *paquete = crear_paquete(LEER_VALOR, buffer);

    agregar_a_paquete(paquete, &direc_fisica, sizeof(int));

    agregar_a_paquete(paquete, &pid, sizeof(int));

    enviar_paquete(paquete, conexion_memoria);

    op_code codigo_operacion = recibir_operacion(conexion_memoria);

    if(codigo_operacion != WRITE)
        log_error(cpu, "No se pudo realizar MOV_IN");

    buffer->stream = recibir_buffer(conexion_memoria);

    uint32_t valor_leido;
    //copio el valor leido del buffer a valor_leido
    memcpy(&valor_leido, buffer->stream, sizeof(uint32_t));

    char* valor_cadena = (char*)malloc(12 * sizeof(char));

    //uso %u para pasar el uint32_t a char
    snprintf(valor_cadena, 12, "%u", valor_leido);

    destruir_buffer(buffer);
    return valor_cadena;    
}


t_instruccion *pedir_instruccion_a_memoria(int pid, uiunt32_t program_counter)
{
    t_buffer *buffer = crear_buffer();

    t_paquete *paquete = crear_paquete(SOLICITUD_INST);

    agregar_a_paquete(paquete,&pid, sizeof(int));
    agregar_a_paquete(paquete, &program_counter, sizeof(uint32_t));

    enviar_paquete(paquete, conexion_memoria);

    //duda:
    //op_code codigo_operacion = recibir_operacion(conexion_memoria);

    if(codigo_operacion == OK)
        codigo_operacion = recibir_operacion(conexion_memoria);
        //deserializar la inst
    if(codigo_operacion != INSTRUCCION)
        log_error(cpu, "No se recibio la instruccion de memoria");
    

    buffer->stream = recibir_buffer(&(buffer->size), conexion_memoria);

    t_instruccion *instruccion = deserializar_instruccion(buffer); //cambiar funcion de deserializar

    destruir_buffer(buffer);
    return instruccion;

}