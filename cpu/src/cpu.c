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
    int servidor_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH);
    int servidor_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT);
    //Inicio la conexion como cliente con la memoria
    conexion_memoria = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(cpu_logger, "Conexion con memoria establecida");  

    realizar_handshake(HANDSHAKE_CPU,conexion_memoria);
    log_info(cpu_logger, "Handshake con Memoria realizado");

    //Espero al cliente Kernel - Dispatch
    cliente_kernel_dispatch = esperar_cliente(servidor_dispatch); 
    //Atender los mensajes de Kernel - Dispatch
    hilo_kernel_dispatch;
    int* socket_cliente_kernel_disptach_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_disptach_ptr = cliente_kernel_dispatch;
    pthread_create(&hilo_kernel_dispatch, NULL, atender_kernel_dispatch, socket_cliente_kernel_disptach_ptr);
    pthread_detach(hilo_kernel_dispatch);
    log_info(cpu_logger, "Atendiendo mensajes de Kernel Dispatch");       
    //Espero al cliente Kernel - Interrupt
    cliente_kernel_interrupt = esperar_cliente(servidor_interrupt);    
    
    //Atender los mensajes de Kernel - Interrupt
    pthread_t hilo_kernel_interrupt;
    int* socket_cliente_kernel_interrupt_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_interrupt_ptr = cliente_kernel_interrupt;
    pthread_create(&hilo_kernel_interrupt, NULL, atender_kernel_interrupt, socket_cliente_kernel_interrupt_ptr);
    log_info(cpu_logger, "Atendiendo mensajes de Kernel Interrupt");
    pthread_join(hilo_kernel_interrupt,NULL);
    
        
    //Atender los mensajes de Memoria  
    return EXIT_SUCCESS;
}
void atender_kernel_dispatch(void* socket_cliente_ptr) {
    int cliente_kd = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
    while (control_key && !llego_interrupcion){          
        
    op_code op_code = recibir_operacion(cliente_kd);  
    log_info(cpu_logger,"Me llega un opcode %d",op_code);  
	switch(op_code) {
		case CONTEXTO_EJECUCION:
			log_info(cpu_logger, "Me llegó contexto ejecución");
            t_buffer* buffer = recibir_buffer(cliente_kd);
			log_info(cpu_logger, "EJECUTAMOS procesos");                
            atender_crear_pr(buffer);
			break;
		case HANDSHAKE_CPU:
			log_info(cpu_logger, "Se conecto el CPU");
			break;
		case HANDSHAKE_MEMORIA:
			log_info(cpu_logger, "Se conecto la Memoria");
			break;
		case HANDSHAKE_ES:
			log_info(cpu_logger, "Se conecto el IO");
			break;
		default:
			log_error(cpu_logger, "No se reconoce el handshake");
			control_key = 0;
			break;
	}   } 
    log_info(cpu_logger,"Fin de hilo de dispatch");
}
void atender_kernel_interrupt(void* socket_cliente_ptr) {
    int cliente_ki = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
    while (control_key){
    op_code handshake = recibir_operacion(cliente_ki); 
    llego_interrupcion = true;
    t_buffer* buffer = recibir_buffer(cliente_ki);
    //pthread_cancel(hilo_kernel_dispatch);
    //log_info(cpu_cpu_logger,"Cancelo hilo de dispatch porque me llega una interrupción");
	switch(handshake) {
		case FIN_DE_QUANTUM:
			log_info(cpu_logger, "Me llegó FIN DE QUANTUM");            			              
            atender_fin_quantum(buffer);
			break;
		case FINPROCESO:
			log_info(cpu_logger, "me llegó finalizar proceso");            
            t_pcb* pcbb = extraer_pcb_del_buffer(buffer);
            log_info(cpu_logger, "Fin de proceso: %d", pcbb->pid);   
            destruir_buffer(buffer) ;
            t_buffer* buffer_cpu_ki = crear_buffer();    
            cargar_pcb_a_buffer(buffer_cpu_ki,pcbb);             
            pthread_cancel(hilo_kernel_dispatch);
            log_info(cpu_logger, "Enviamos PCB de proceso FINALIZADO - PID %d ", pcbb->pid);   
            t_paquete* paquete_cpu = crear_paquete(INTERRUPTED_BY_USER, buffer_cpu_ki);
            enviar_paquete(paquete_cpu, cliente_kernel_dispatch);    
            free(pcbb);            
            destruir_paquete(paquete_cpu);

			break;
		case HANDSHAKE_MEMORIA:
			log_info(cpu_logger, "Se conecto la Memoria");
			break;
		case HANDSHAKE_ES:
			log_info(cpu_logger, "Se conecto el IO");
			break;
		default:
			log_error(cpu_logger, "No se reconoce el handshake");
			control_key = 0;
			break;
	}   
    llego_interrupcion = false;
    log_info(cpu_logger,"llego interrupcion en false"); 
    } 

}

void atender_crear_pr(t_buffer* buffer){
      
    t_pcb* pcbb = extraer_pcb_del_buffer(buffer);
    log_info(cpu_logger, "Creamos PCB: %d", pcbb->pid); 
    destruir_buffer(buffer);
    t_buffer* buffer_cpu_ki = crear_buffer();   
    pcbb->registros.AX = 5;
    int64_t numero = (int64_t) 5;
    pcbb->quantum = numero;
    cargar_pcb_a_buffer(buffer_cpu_ki,pcbb); 
    //cargar_string_a_buffer(buffer_cpu_ki, "Nueva 1");
    //cargar_string_a_buffer(buffer_cpu_ki, "50");       
    //char * recurso = "RA";
    cargar_string_a_buffer(buffer_cpu_ki,"RA");
    //usleep(5000000);
    //sleep(10);
    t_paquete* paquete_cpu = crear_paquete(SOLICITAR_WAIT, buffer_cpu_ki);
    //log_info(cpu_logger, "Enviamos PCB a Kernel con quantum: %d", pcbb->quantum);
    enviar_paquete(paquete_cpu, cliente_kernel_dispatch);
    destruir_buffer(buffer_cpu_ki);
}

/*void devolver_pcb(t_pcb pcb){
    t_buffer* buffer_cpu_k = crear_buffer();    
    cargar_pcb_a_buffer2(buffer_cpu_k,pcb);    
	t_paquete* paquete_cpu = crear_paquete(FIN_DE_QUANTUM, buffer_cpu_k);
    enviar_paquete(paquete_cpu, cliente_kernel_dispatch);
    destruir_buffer(buffer_cpu_k);

}*/

void atender_fin_quantum(t_buffer* buffer){
      
    t_pcb* pcbb = extraer_pcb_del_buffer(buffer);
    log_info(cpu_logger, "Fin de Quantum: %d", pcbb->pid);   
    destruir_buffer(buffer) ;
    t_buffer* buffer_cpu_ki = crear_buffer();    
    cargar_pcb_a_buffer(buffer_cpu_ki,pcbb); 
    //char * recurso = "RA";
    //cargar_string_a_buffer(buffer_cpu_ki,recurso);
    pthread_cancel(hilo_kernel_dispatch);
    log_info(cpu_logger, "Enviamos PCB de proceso desalojado - PID %d a Kernel Interrupt con Quantum %d", pcbb->pid, pcbb->quantum);   
	t_paquete* paquete_cpu = crear_paquete(PROCESO_DESALOJADO, buffer_cpu_ki);
    enviar_paquete(paquete_cpu, cliente_kernel_dispatch);    
    free(pcbb);
    
}