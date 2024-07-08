#include "kernel.h"


int main(int argc, char* argv[]) { 

    //Inicio el logger del kernel
    kernel_logger = iniciar_logger("kernel.log", "LOGGER_KERNEL");  

    //Inicio la configuracion del kernel
    kernel_config = iniciar_config("kernel.config");
    PUERTO_ESCUCHA = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
    log_info(kernel_logger, "PUERTO_ESCUCHA %s", PUERTO_ESCUCHA);
    IP_MEMORIA = config_get_string_value(kernel_config, "IP_MEMORIA");
    log_info(kernel_logger, "IP_MEMORIA %s", IP_MEMORIA);
    PUERTO_MEMORIA = config_get_string_value(kernel_config, "PUERTO_MEMORIA");
    log_info(kernel_logger, "PUERTO_MEMORIA %s", PUERTO_MEMORIA);    
    IP_CPU = config_get_string_value(kernel_config, "IP_CPU");
    log_info(kernel_logger, "IP_CPU %s", IP_CPU);
    PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config, "PUERTO_CPU_DISPATCH");
    log_info(kernel_logger, "PUERTO_CPU_DISPATCH %s", PUERTO_CPU_DISPATCH);
    PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config, "PUERTO_CPU_INTERRUPT");
    log_info(kernel_logger, "PUERTO_CPU_INTERRUPT %s", PUERTO_CPU_INTERRUPT);
    ALGORITMO_PLANIFICACION = config_get_string_value(kernel_config, "ALGORITMO_PLANIFICACION");
    log_info(kernel_logger, "ALGORITMO_PLANIFICACION %s", ALGORITMO_PLANIFICACION);
    QUANTUM = config_get_int_value(kernel_config, "QUANTUM");
    log_info(kernel_logger, "QUANTUM %d", QUANTUM);
    quantum_64 = (int64_t)QUANTUM;
    RECURSOS = config_get_array_value(kernel_config, "RECURSOS");
    log_info(kernel_logger, "RECURSOS: %s|%s|%s",RECURSOS[0],RECURSOS[1],RECURSOS[2]);
    INSTANCIAS_RECURSOS = config_get_array_value(kernel_config, "INSTANCIAS_RECURSOS");
    log_info(kernel_logger, "INSTANCIAS_RECURSOS %s|%s|%s", INSTANCIAS_RECURSOS[0], INSTANCIAS_RECURSOS[1], INSTANCIAS_RECURSOS[2]);    
    GRADO_MULTIPROGRAMACION = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");
    log_info(kernel_logger, "GRADO_MULTIPROGRAMACION %d", GRADO_MULTIPROGRAMACION);   
    tamanio_lista_recursos;
    tamanio_lista_recursos = string_array_size(RECURSOS); 
    
    //Inicio el cliente para cpu dispatch
    conexion_cpu_dispatch = crear_conexion_cliente(IP_CPU, PUERTO_CPU_DISPATCH);
        
    //Inicio el cliente para cpu interrupt
    conexion_cpu_interrupt = crear_conexion_cliente(IP_CPU, PUERTO_CPU_INTERRUPT);
    
    
    //Inicio el cliente para memoria
    conexion_k_memoria = crear_conexion_cliente(IP_MEMORIA,PUERTO_MEMORIA);
    realizar_handshake(HANDSHAKE_KERNEL, conexion_k_memoria);
    log_info(kernel_logger,"Handshake con Memoria realizado");
    //Inicio el servidor
    servidor = iniciar_servidor(PUERTO_ESCUCHA);
    
    //Espero a los clientes
   // cliente_entradasalida = esperar_cliente(servidor); 

    
    //Planificacion
    //Inicio los hilos de planificacion
    inicializar_listas();  

    //Leer consola
    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, (void *)leer_consola, NULL);
    pthread_detach(hilo_consola);


    //Atiendo mensajes de Entrada/Salida
    /*pthread_t hilo_entradasalida;
    int* socket_cliente_entradasalida2_ptr = malloc(sizeof(int));
    *socket_cliente_entradasalida2_ptr = cliente_entradasalida;
    pthread_create(&hilo_entradasalida, NULL,atender_entradasalida2, socket_cliente_entradasalida2_ptr);
    log_info(kernel_logger, "Atendiendo mensajes de Entrada/Salida");
    pthread_join(hilo_entradasalida,NULL);*/

    //Acá espero y manejo a entrada y salida. Por cada uno que acepto creo un hilo.
    sem_init(&mutex_lista_interfaces,0,1);
    while(1){
        pthread_t hilo_entradasalida;
        int *fd_conexion_ptr = malloc(sizeof(int));
        *fd_conexion_ptr = accept(servidor, NULL, NULL);
        log_info(kernel_logger, "Se conecto una nueva interfaz");        
        pthread_create(&hilo_entradasalida,NULL,atender_entradasalida2,fd_conexion_ptr);
        pthread_detach(hilo_entradasalida);
    }
    
    
    



    return EXIT_SUCCESS;
}

void atender_entradasalida2(void* socket_cliente_ptr){
    int cliente_entradasalida2 = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    
    bool control_key = 1;
    while (control_key){
        op_code op_code = recibir_operacion(cliente_entradasalida2);        
        switch (op_code){
            case CREAR_NUEVA_INTERFAZ:
            t_buffer *buffer = recibir_buffer(cliente_entradasalida2);
            char* nombre = extraer_string_del_buffer(buffer);
            char* tipo = extraer_string_del_buffer(buffer);
            t_entrada_salida *nueva_interfaz = malloc(sizeof(t_entrada_salida));
            nueva_interfaz->nombre = nombre;
            nueva_interfaz->tipo = tipo;
            //nueva_interfaz->disponible = 1;
            sem_init(&nueva_interfaz->sem_disponible, 0, 1);
            nueva_interfaz-> pid_usandola = 0;
            nueva_interfaz->fd_interfaz = cliente_entradasalida2;
            log_info(kernel_logger, "Nombre nueva interfaz: %s y tipo %s",
            nueva_interfaz->nombre,nueva_interfaz->tipo);
            nueva_interfaz->cola_procesos_bloqueados = queue_create();
            sem_wait(&mutex_lista_interfaces);
                list_add(lista_interfaces, nueva_interfaz);
            sem_post(&mutex_lista_interfaces);
            log_info(kernel_logger,"Tamaño  de la lista de interfaces: %d", list_size(lista_interfaces));
            break;
            
            case OPERACION_FINALIZADA:
            log_info(kernel_logger, "Operacion finalizada");
            t_buffer *buffer2 = recibir_buffer(cliente_entradasalida2);
            char* nombre2 = extraer_string_del_buffer(buffer2);
            int pid = extraer_int_del_buffer(buffer2);
            t_entrada_salida * interfaz_a_liberar = buscar_interfaz(nombre2);
            //Hay que poner el PCB en la cola de listos
            t_pcb* pcb_a_liberar = buscarPcbBloqueado(pid);
            sacar_de_bloqueado(pcb_a_liberar);  
            //agregar_a_exit(pcb_a_liberar,SUCCESS);          
            if (queue_is_empty(interfaz_a_liberar->cola_procesos_bloqueados)){ // si no tengo a nadie esperando por la interfaz
                interfaz_a_liberar->pid_usandola = 0;
                sem_post(&interfaz_a_liberar->sem_disponible);
            }else{
                t_lista_block* proximo_proceso_bloqueado = queue_pop(interfaz_a_liberar->cola_procesos_bloqueados);
                interfaz_a_liberar->pid_usandola = proximo_proceso_bloqueado->pcb->pid;
                //mandar_interfaz_a_io(interfaz_a_liberar); aca hay q hacer de algo para que en la cola de bloqueados por interfaz no solo me guarde
                // el pcb sino tambien la operacion y los parametros q queria hacer (no tengo idea, capaz lo estamos planteando mal)
                t_buffer* buffer_a_enviar = crear_buffer();
                //VER ACÄ MANDAR BIEN LAS COSAS
                cargar_string_a_buffer(buffer_a_enviar, proximo_proceso_bloqueado->parametros);
                t_paquete* paquete_a_enviar = crear_paquete(IO_GEN_SLEEP, buffer_a_enviar);
                enviar_paquete(paquete_a_enviar,interfaz_a_liberar->fd_interfaz);
            }

            break;
        default:
			log_error(kernel_logger, "Se desconectó la interfaz: %d", op_code);
            sem_wait(&mutex_lista_interfaces);
                list_remove_element(lista_interfaces, nueva_interfaz);
            sem_post(&mutex_lista_interfaces);
            log_info(kernel_logger,"Tamaño  de la lista de interfaces: %d", list_size(lista_interfaces));
			control_key = 0;
			break;
        }
    }
}



