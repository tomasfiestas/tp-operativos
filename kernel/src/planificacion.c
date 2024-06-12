#include "planificacion.h"

t_pcb* pcb_nuevo;
char** recursos;
char** instancias_recursos;
t_temporal* timer;
int64_t tiempo_ejecutado;


//Listas de estados
t_list* plani_new;
t_list* plani_ready;
t_list* total_pcbs;
t_list* plani_exec;
t_list* plani_block;
t_queue** plani_block_recursos;
t_list* plani_exit;

//t_list* lista_pcbs_sockets; //No se que es?


algoritmos  algoritmo_plani;



int multiprogramacion;

sem_t planificacion_largo_plazo_activa;
sem_t planificacion_corto_plazo_activa;
//Semaforos para multiprogramacion
sem_t multiPermiteIngresar;
sem_t lugares_ready_llenos;
sem_t lugares_ready_vacios;
sem_t mutex_multiprogramacion;

// sem_t multiPermiteIngresar;
sem_t hayPCBsEnNew;
sem_t hayPCBsEnReady;
sem_t puedeEntrarAExec;

//Semaforo para respuestas
sem_t respuesta_memoria;
//sem_t se_creo_archivo;
sem_t respuesta_delete_segment;
sem_t proceso_inciado_memoria;

//Semaforo para los estados
sem_t sem_new;
sem_t sem_total_pcbs;
sem_t sem_ready;
sem_t sem_exec;
sem_t sem_block;
sem_t sem_exit;

//semaforo recursos
sem_t mutex_recursos; 
sem_t semaforo_recursos[];

//semaforo para quantum
sem_t sem_volvioContexto;


void crear_pcb(int pid){
	t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    nuevo_pcb->pid = pid;	
	nuevo_pcb->program_counter = 0;	
	//nuevo_pcb->tabla_archivos = list_create(); //Comento porque no se para que sirve
	nuevo_pcb->estado = NEW;
	nuevo_pcb->ejecuto = 0;
	nuevo_pcb->quantum = quantum_64;

	
	inicializar_registros(nuevo_pcb);
	

	// agrego el nuevo proceso a la lista total_pcbs
	sem_wait(&sem_total_pcbs);
	list_add(total_pcbs, nuevo_pcb);
	sem_post(&sem_total_pcbs);
	
	agregar_a_new(nuevo_pcb);
	log_info(kernel_logger, "Nuevo proceso %d en NEW", pid);
	
}

void inicializar_registros(t_pcb* nuevo_pcb){
	uint8_t ax = 0;
	uint32_t eax = 0;	
	memcpy(&(nuevo_pcb->registros.AX), &ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.BX), &ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.CX), &ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.DX), &ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.EAX), &eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.EBX), &eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.ECX), &eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.EDX), &eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.SI), &eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.DI), &eax, sizeof(uint32_t));
}



//Inicio semaforos

void inicializar_semaforos(){
	sem_init(&multiPermiteIngresar, 0,GRADO_MULTIPROGRAMACION );
	//sem_init(&lugares_ready_llenos, 0, 0);
	// sem_init(&lugares_ready_vacios,0, GRADO_MULTIPROGRAMACION); no los usamos nunca ja
	sem_init(&puedeEntrarAExec, 0, 1);
	sem_init(&mutex_multiprogramacion, 0, 1);
	sem_init(&hayPCBsEnReady, 0, 0);
	sem_init(&hayPCBsEnNew, 0, 0);
	sem_init(&sem_new, 0, 1);
	sem_init(&sem_total_pcbs, 0, 1);
	sem_init(&sem_ready, 0, 1);
	sem_init(&sem_exec, 0, 1);
	sem_init(&sem_block, 0, 1);
	sem_init(&sem_exit, 0, 1);
	sem_init (&planificacion_largo_plazo_activa,0,0);
	sem_init (&planificacion_corto_plazo_activa,0,0);
	sem_init(&sem_volvioContexto,0,0);
	sem_init(&mutex_recursos,0,1);
	for (int i = 0; i < string_array_size(RECURSOS); i++) {
		int instancia_recurso = atoi(INSTANCIAS_RECURSOS[i]);
        sem_init(&semaforo_recursos[i], 0, instancia_recurso); 
		//log_info(kernel_logger, "el recurso en posicion %d tiene %d instancias para utilizar", i, instancia_recurso);
    }
	
}


void inicializar_listas(){
	log_info(kernel_logger, "Algoritmo planificacion: %s", ALGORITMO_PLANIFICACION);
	
	algoritmo_plani = obtener_algoritmo();
	int cant_recursos = string_array_size(RECURSOS);

	// Inicio listas
	plani_new = list_create();
	total_pcbs = list_create();
	plani_ready = list_create();
	plani_exec = list_create();
	plani_block = list_create();
    plani_exit = list_create();
	
	
	
	//inicializo lista de colas para manejar los bloqueados por recurso 
	t_queue ** lista_recursos_bloqueados = malloc(sizeof(t_queue*)*cant_recursos);
	for(int i = 0; i < cant_recursos; i++){
		lista_recursos_bloqueados[i] = queue_create();
		log_info(kernel_logger, "cola de recurso en posicion %d inicializada", i);
	}
	plani_block_recursos = lista_recursos_bloqueados;

	inicializar_semaforos();
	inicializar_hilos();
	//inicializar_colas_bloqueo_de_recusos();
	//crear_lista_recursos();
	
	
} 

void inicializar_hilos(){

    pthread_t hilo_planificador_largo_plazo, hilo_planificador_corto_plazo, hilo_exit;

    pthread_create(&hilo_planificador_largo_plazo, NULL, (void *)inicio_plani_largo_plazo, NULL);
	pthread_detach(hilo_planificador_largo_plazo);

	pthread_create(&hilo_planificador_corto_plazo, NULL, (void *)inicio_plani_corto_plazo, NULL);
	pthread_detach(hilo_planificador_corto_plazo);
    /*pthread_create(&hilo_exit, NULL, (void*)sacar_de_exit, NULL);   
	pthread_detach(hilo_exit);*/ 
    log_info(kernel_logger, "Levanto los hilos");
    
}

algoritmos obtener_algoritmo(){
    
    if(strcmp(ALGORITMO_PLANIFICACION,"RR") == 0){
        return RR;
    }
    if(strcmp(ALGORITMO_PLANIFICACION,"FIFO") == 0){
        return FIFO;
    }else{		
		return VRR;
    }
}


// ---------------------------------------------------
//             PLANIFICADOR LARGO PLAZO
// ---------------------------------------------------

void* inicio_plani_largo_plazo(void* arg){
	sem_wait(&planificacion_largo_plazo_activa);
	int valor = 0;
	sem_getvalue(&planificacion_largo_plazo_activa, &valor);

	while(valor ==1){		
		
		// Espero a que haya procesos en new 			
		sem_wait(&hayPCBsEnNew);
		log_info(kernel_logger, "Planificador Largo PLazo: Hay PCBs en NEW.");
		
		// Espero a que el grado de multiprogramacion me permita agregar un proceso a RAM		
		sem_wait(&multiPermiteIngresar); // estaba aca (por el comentario d arriba) no se pq ni cuando desaparecio
		log_info(kernel_logger, "Planificador Largo PLazo: Multiprogramacion permite ingresar a RAM.");
		
		//Se agrega el pcb a READY
		t_pcb* pcb = sacar_siguiente_de_new();		
		agregar_a_ready(pcb);
		log_info(kernel_logger, "Planificador Largo PLazo: Se agrego un nuevo proceso a READY");
		
		sem_getvalue(&planificacion_largo_plazo_activa, &valor);
	}

	return NULL;
}



// ---------------------------------------------------
//             PLANIFICADOR CORTO PLAZO
// ---------------------------------------------------
void* inicio_plani_corto_plazo(void* arg){
	sem_wait(&planificacion_corto_plazo_activa);
	int valor_corto_plazo = 0;
	sem_getvalue(&planificacion_corto_plazo_activa, &valor_corto_plazo);
	
	while(valor_corto_plazo ==1){
		
		sem_wait(&hayPCBsEnReady);
		  log_info(kernel_logger, "Planificador Corto PLazo: Hay PCBs en READY.");

		sem_wait(&puedeEntrarAExec);
		  log_info(kernel_logger, "Planificador Corto PLazo: PCB puede entrar a EXEC.");

		t_pcb* pcb;
		
		pcb = sacar_de_ready();
		agregar_a_exec(pcb);
		

		//MANDAR CONTEXTO A CPU PARA QUE EJECUTE		
  		mandar_contexto_a_CPU(pcb);

		//Implemento hilo para contar quantum en RR 
		if(algoritmo_plani!=FIFO){
		//pthread_t hilo_quantum;
    	int* socket_cliente_cpu_dispatch_ptr = malloc(sizeof(int));
    	//*socket_cliente_cpu_dispatch_ptr = conexion_cpu_dispatch;
    	pthread_create(&hilo_quantum, NULL, (void *)manejo_quantum, (void*)pcb); 		   	
    	pthread_detach(hilo_quantum);
		}
			

		//IMPLEMENTAR COMO VAMOS A RECIBIR EL CONTEXTO DE EJECUCIÓN
		pthread_t hilo_kernel_dispatch;
    	int* socket_cliente_cpu_dispatch_ptr = malloc(sizeof(int));
    	*socket_cliente_cpu_dispatch_ptr = conexion_cpu_dispatch;
    	pthread_create(&hilo_kernel_dispatch, NULL, atender_cpu_dispatch, socket_cliente_cpu_dispatch_ptr);
    	log_info(kernel_logger, "Atendiendo mensajes de CPU Interrupt");
    	pthread_join(hilo_kernel_dispatch,NULL);//REVISAR
		sem_getvalue(&planificacion_corto_plazo_activa, &valor_corto_plazo);
		
		//pthread_cancel(hilo_quantum);	

	}

}

void iniciar_planificacion(){	
		
		sem_post(&planificacion_largo_plazo_activa);	
		sem_post(&planificacion_largo_plazo_activa);
		sem_post(&planificacion_corto_plazo_activa);
		sem_post(&planificacion_corto_plazo_activa);		
	
}
void detener_planificacion(){
	sem_wait(&planificacion_largo_plazo_activa);
	sem_wait(&planificacion_corto_plazo_activa);
}	

// -------------------------------------------------
//             FUNCIONES GENERALES
// -------------------------------------------------



void agregar_a_new(t_pcb* nuevo_pcb){
	sem_wait(&sem_new);	
		list_add(plani_new, nuevo_pcb);
		log_info(kernel_logger, "Se crea el proceso %d en NEW", nuevo_pcb->pid);
	sem_post(&sem_new);
	sem_post(&hayPCBsEnNew);		
}

void agregar_a_ready(t_pcb* nuevo_pcb){
	
	sem_wait(&mutex_multiprogramacion);
		list_add(plani_ready, nuevo_pcb);
	sem_post(&mutex_multiprogramacion);
	cambiar_estado_pcb(nuevo_pcb, READY);
	mostrar_pids_ready();	
	sem_post(&hayPCBsEnReady);
}
void mostrar_pids_ready() {
	
	char* pids = string_new();
	for (int i = 0; i < list_size(plani_ready); i++) {
		t_pcb* pcb = list_get(plani_ready, i);
		char* pid_str = string_itoa(pcb->pid);
		string_append(&pids, pid_str);
		string_append(&pids, ", "); // Add a comma and space separator
		free(pid_str);
	}
	// Remove the last comma and space separator
	if (strlen(pids) > 0) {
		pids[strlen(pids) - 2] = '\0';
	}
	log_info(kernel_logger, "Cola Ready : [%s ]", pids);	
	free(pids);
}
t_pcb* sacar_siguiente_de_new(){
	sem_wait(&sem_new);
	t_pcb* pcb = list_remove(plani_new,0);
	sem_post(&sem_new);
	return pcb;
}

t_pcb* sacar_de_ready(){
	t_pcb* pcb;
	switch(algoritmo_plani){
	case FIFO:
		sem_wait(&sem_ready);
			pcb = list_remove(plani_ready, 0);
		sem_post(&sem_ready);
		return pcb;
	
	case RR: 
		sem_wait(&sem_ready);
    		pcb = list_remove(plani_ready, 0);
        sem_post(&sem_ready);		
		return pcb;

	case VRR:
		if(!list_is_empty(plani_block)){
			sem_wait(&sem_block);
    			pcb = list_remove(plani_block, 0);
       		sem_post(&sem_block);	
		}else{
			sem_wait(&sem_ready);
    			pcb = list_remove(plani_ready, 0);
        	sem_post(&sem_ready);	
		}
		return pcb;
	default:
		return NULL;
	}
}

void agregar_a_exec(t_pcb* pcb){
	sem_wait(&sem_exec);
		pcb->ejecuto = 1;
		list_add(plani_exec, pcb);
	sem_post(&sem_exec);
	cambiar_estado_pcb(pcb, EXEC);
}
void sacar_de_exec(t_pcb* pcb, op_code op_code){
	sem_wait(&sem_exec);
		list_remove_element(plani_exec, pcb);
	sem_post(&sem_exec);
		switch (op_code)
			{
			case IO:
				agregar_a_bloqueado(pcb);
			break;
			case ESPERA_RECURSO:
				agregar_a_bloqueado(pcb);
			break;
			case PROCESO_DESALOJADO:			
				agregar_a_ready(pcb);
			break;
			case FINPROCESO:
				agregar_a_exit(pcb,op_code);
			break;
			case FIN_DE_QUANTUM: //ES SOLO DE PRUEBA!!
				agregar_a_ready(pcb);
			break;
			case SUCCESS:
				agregar_a_exit(pcb,op_code);
			break;
			case INVALID_RESOURCE:
				agregar_a_exit(pcb,op_code);				
			default: // FINPROCESO
				agregar_a_exit(pcb,op_code);
			break;
			}
}

void agregar_a_bloqueado(t_pcb* pcb){
	sem_wait(&sem_block);
		list_add(plani_block, pcb);
	sem_post(&sem_block);
	cambiar_estado_pcb(pcb, BLOCK);
}

void sacar_de_bloqueado(t_pcb* pcb){
	sem_wait(&sem_block);
		list_remove(plani_block, pcb);
	sem_post(&sem_block);
}

void agregar_a_exit(t_pcb* pcb,op_code motivo_a_mostrar){
	sem_wait(&sem_exit);
		list_add(plani_exit, pcb);
	sem_post(&sem_exit);
	cambiar_estado_pcb(pcb, EXIT);
	sem_post(&multiPermiteIngresar);
	char *motivo = mensaje_a_string(motivo_a_mostrar);
	log_info(kernel_logger, "Finaliza el proceso %d - Motivo: %s", pcb->pid, motivo);
}

t_pcb* pcb_de_exec(){
	sem_wait(&sem_exec);
	t_pcb* pcb = list_get(plani_exec, 0);
	sem_post(&sem_exec);
	return pcb;
}

void mandar_contexto_a_CPU(t_pcb* pcb){
	t_buffer* buffer_cpu = crear_buffer();    
    cargar_pcb_a_buffer(buffer_cpu,pcb);    
	log_info(kernel_logger, "Envio contexto de ejecucion a CPU %d", pcb->pid);
	t_paquete* paquete_cpu = crear_paquete(CONTEXTO_EJECUCION, buffer_cpu);
    enviar_paquete(paquete_cpu, conexion_cpu_dispatch);	
	destruir_paquete(paquete_cpu);
}

void enviar_interrupcion_por_quantum(t_pcb* pcb){
    t_buffer* buffer_cpu2 = crear_buffer();    
    cargar_pcb_a_buffer(buffer_cpu2,pcb);  
	log_info(kernel_logger, "Envio interrupcion por fin de quantum a CPU %d", pcb->pid);  
	t_paquete* paquete_cpu = crear_paquete(FIN_DE_QUANTUM, buffer_cpu2);
    enviar_paquete(paquete_cpu, conexion_cpu_interrupt); 
	destruir_buffer(buffer_cpu2);
}
void cambiar_estado_pcb(t_pcb* pcb, t_estado estadoNuevo){
	char* estadoAnteriorString = string_new();
	char* estadoNuevoString = string_new();

	t_estado estadoAnterior = pcb->estado;
	pcb->estado = estadoNuevo;

	string_append(&estadoAnteriorString, estado_a_string(estadoAnterior));
	string_append(&estadoNuevoString, estado_a_string(estadoNuevo));

	log_info(kernel_logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estadoAnteriorString, estadoNuevoString);

	free(estadoAnteriorString);
	free(estadoNuevoString);
}

void *contar_quantum(){
    usleep(QUANTUM*1000);
   // sem_post(&sem_desalojar);
}

char* estado_a_string(t_estado estado) {
	switch (estado) {
	case NEW:
		return "NEW";
	case READY:
		return "READY";
	case EXEC:
		return "EXEC";
	case BLOCK:
		return "BLOCK";
	case EXIT:
		return "EXIT";
	default:
		return "ESTADO DESCONOCIDO";
	}
}

void atender_cpu_dispatch(void* socket_cliente_ptr) {
    int cliente_kd = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);    	
    op_code op_code = recibir_operacion(cliente_kd);
	log_info(kernel_logger,"Me llegó un op_code %d",op_code);
	sem_post(&puedeEntrarAExec); 
	llego_contexto = true;	
	pthread_cancel(hilo_quantum); 
	log_info(kernel_logger,"Cancelo HILO QUANTUM %d",hilo_quantum);  
	t_buffer* buffer = recibir_buffer(cliente_kd);	
	t_pcb* pcb = extraer_pcb_del_buffer(buffer);	
	if(algoritmo_plani == VRR){		
		temporal_stop(timer);
		log_info(kernel_logger, "Se detiene el timer : %d, Quantum restante antes de restar: %d", timer->elapsed_ms,pcb->quantum);		
			tiempo_ejecutado = temporal_gettime(timer);	
			temporal_destroy(timer);				
				if(op_code != PROCESO_DESALOJADO){					
					if (pcb->quantum - tiempo_ejecutado > 1) {
						pcb->quantum -= tiempo_ejecutado;
						log_info(kernel_logger, "Quantum restante: %d", pcb->quantum);						
					} else if(pcb->quantum - tiempo_ejecutado == 0){
						pthread_cancel(hilo_quantum);
						log_info(kernel_logger, "cancelo el hilo de quantum ya que justo dio cero");
						pcb->quantum = (int64_t)QUANTUM;
						log_info(kernel_logger, "Reiniciando ya que dió cero JUSTO, quantum restante: %d", pcb->quantum);						
					}else if(pcb->quantum - tiempo_ejecutado < 0){
						pthread_cancel(hilo_quantum);
						log_info(kernel_logger, "cancelo el hilo de quantum ya que dio negativo");
						pcb->quantum = (int64_t)QUANTUM;
						log_info(kernel_logger, "Reiniciando ya que dió negativo, quantum restante: %d", pcb->quantum);						
					}

				} 
				
	}	
	//pthread_cancel(hilo_quantum);   
	
	switch(op_code) {
		case PROCESO_DESALOJADO:			
            
			sacar_de_exec(pcb, op_code);     
    		log_info(kernel_logger, "LLEGO EL PROCESO QUE DESALOJé por FIN de Q", pcb->pid); 
			destruir_buffer(buffer);
			
			break;
		case FIN_DE_QUANTUM:
			sacar_de_exec(pcb, op_code);     
    		//log_info(kernel_logger, "PID: %d - Desalojado por fin de Quantum", pcb->pid); 
			destruir_buffer(buffer);
			break;
		case INTERRUPTED_BY_USER:
			log_info(kernel_logger, "llegó fin de proceso");
			//t_buffer* buffer3 = recibir_buffer(cliente_kd);			        
			atender_fin_proceso(buffer,op_code,pcb);			
			break;
		case SUCCESS:
			log_info(kernel_logger, "Se finalizó correctamente el proceso");
			t_buffer* buffer4 = recibir_buffer(cliente_kd);			        
			atender_fin_proceso_success(buffer4,op_code);
			break;
		case SOLICITAR_WAIT:
			//log_info(kernel_logger, "Llegó solicitud de wait");
			//t_buffer* buffer5 = recibir_buffer(cliente_kd);
			//t_pcb* pcb = extraer_de_buffer(buffer5);
			char recurso_wait = extraer_string_del_buffer2(buffer);

			destruir_buffer(buffer);
			wait_recurso(pcb,recurso_wait);
            //free(recurso_wait);

            break;
		case SOLICITAR_SIGNAL:
			log_info(kernel_logger, "Llegó solicitud de signal");
			//t_buffer* buffer6 = recibir_buffer(cliente_kd);
			//t_pcb* pcb2 = extraer_de_buffer(buffer6);
			char recurso_signal = extraer_string_del_buffer2(buffer);

			signal_recurso(pcb,recurso_signal);
            //free(recurso_signal);

            break;
			

			break;
		default:
			log_error(kernel_logger, "No se reconoce el handshake");
			break;
	}
	llego_contexto = false;
	log_info(kernel_logger,"llego contexto en FALSE");
	

}

void atender_crear_pr2(t_pcb* pcb,op_code op_code){
	//t_pcb* pcb = extraer_pcb_del_buffer(buffer);
	
	//REVISAR ACÄ COMO CONTINUAR
	sacar_de_exec(pcb, op_code);
     
    //log_info(kernel_logger, "PID: %d - Desalojado por fin de Quantum", pcb->pid); 

    //destruir_buffer(buffer);
}



void atender_proceso_desalojado(t_buffer* buffer, op_code op_code){
	t_pcb* pcb;
	//t_pcb pcbb = recibir_contexto_ejecucion(buffer); 
	pcb = extraer_de_buffer(buffer);
    t_pcb valor_pcb = *pcb;
    //free(pcb);
   
	//REVISAR ACÄ COMO CONTINUAR
	sacar_de_exec(pcb, op_code);
     
    log_info(kernel_logger, "LLEGO A KERNEL PROCESO DESALOJADO - LO MANDO A READY XQ ESTOY EN RR: %d", valor_pcb.pid); 

    destruir_buffer(buffer);
}

void *manejo_quantum(t_pcb * pcb){
	switch(algoritmo_plani){
		case RR:
			contar_quantum();
			//CHEQUEAR SI SIGUE EL PCB EJECUTANDO o si recibí algo.
			//result = sem_trywait(&sem_volvioContexto);
			if(!llego_contexto){ //este semaforo se levanta cuando 
			//me llega algo de cpu --> si no llego nada mando interrupcion por quantum
			enviar_interrupcion_por_quantum(pcb);
			}
			break;
		case VRR:			
			timer = temporal_create();			
			log_info(kernel_logger, "Es el hilo numero %d", pthread_self());
			identificador_hilo++;
			log_info(kernel_logger,"Quantum restante antes de dormir: %d",pcb->quantum);
			usleep((pcb->quantum)*1000);			
			log_info(kernel_logger,"Se despertó el hilo de quantum");
			log_info(kernel_logger,"LLegó el contexto? %d",llego_contexto);
			if(!llego_contexto){				
				pcb->quantum = (int64_t)QUANTUM;
				log_info(kernel_logger, "Reiniciando quantum ya que mando interrupcion restante: %d", pcb->quantum);	
				enviar_interrupcion_por_quantum(pcb);			
			}
			
			break;	
	}
	log_info(kernel_logger, "Fin de hilo Quantum %d",pthread_self());
	//pthread_cancel(hilo_quantum);			

}


void atender_fin_proceso(t_buffer* buffer,op_code op_code,t_pcb* pcb){
	//t_pcb* pcb;	
	//pcb = extraer_de_buffer(buffer);
    t_pcb valor_pcb = *pcb;
    
	sacar_de_exec(pcb, op_code);     
    log_info(kernel_logger, "Llegó el fin de proceso: %d", valor_pcb.pid); 
	finalizarProceso(valor_pcb.pid);
    destruir_buffer(buffer);
	free(pcb);	
}

void finalizarProceso(int pid){
	//Le aviso a la memoria que voy a finalizar un proceso [int pid] 
    t_buffer* buffer_memoria = crear_buffer();
    cargar_int_a_buffer(buffer_memoria, pid);
    //LIBERAR RECURSOS Y ARCHIVOS
    
    t_paquete* paquete_memoria = crear_paquete(FINPROCESO, buffer_memoria);
    enviar_paquete(paquete_memoria, conexion_k_memoria);
}

void atender_fin_proceso_success(t_buffer* buffer,op_code op_code){
	t_pcb* pcb;	
	pcb = extraer_de_buffer(buffer);
    t_pcb valor_pcb = *pcb;
    //free(pcb);	
	sacar_de_exec(pcb, op_code);     
    log_info(kernel_logger, "Llegó el proceso finalizado: %d", valor_pcb.pid); 
	finalizarProceso(valor_pcb.pid);
    destruir_buffer(buffer);
}

t_pcb *buscarPcb(int pid_a_buscar)
{
t_pcb *pcb;
   
        for (int i = 0; i < (list_size(total_pcbs)); i++) {
        pcb = list_get(total_pcbs,i);
        if (pcb->pid == pid_a_buscar) {
            return pcb;
        }
    }
    return NULL;
}


void sacar_pcb_de_lista(t_pcb* pcb){
    int cant_colas_bloqueadas;
    int cant_proc_cola_bloqueada;

    switch(pcb->estado){
        case READY:
            sacar_de_lista(plani_ready, pcb->pid);
            sem_wait(&hayPCBsEnReady);
            break;
        case NEW:
            sacar_de_lista(plani_new, pcb->pid);
            //sem_wait(&hayPCBsEnNew);
            break;
        case BLOCK:
            /*cant_colas_bloqueadas = list_size(lista_recursos_bloqueados);
            for(int i = 0; i < cant_colas_bloqueadas ; i++){
                t_cola_block *cola_a_analizar = list_get(lista_recursos_bloqueados,i);
                cant_proc_cola_bloqueada = queue_size(cola_a_analizar->cola_bloqueados);
                for(int j = 0; j < cant_proc_cola_bloqueada; j++){
                    int* pid_bloqueado = (int *)queue_pop(cola_a_analizar->cola_bloqueados);
                    if(pcb->contexto_ejecucion->pid == *pid_bloqueado){
                        i=cant_colas_bloqueadas;
                        break;
                    }
                    queue_push(cola_a_analizar->cola_bloqueados, pid_bloqueado);
                }
            }*/
            break;
        default:
			log_info(kernel_logger, "No se encontro el proceso en ninguna cola");
            exit(EXIT_FAILURE);
            break;
    }    
}
void sacar_de_lista(t_list * lista, int pid){
    int elementos_lista = list_size(lista);
    t_pcb* pcb;
    for(int i=0; i<elementos_lista; i++){
        pcb = list_get(lista, i);
        if(pcb->pid == pid){
            list_remove(lista, i);
			int elementos_lista2 = list_size(lista);
			printf("Elementos en lista: %d\n", elementos_lista2);
            return;
			
        }
    }
	
    //exit(EXIT_FAILURE);
}

char *mensaje_a_string(op_code motivo){
	switch (motivo){
    case SUCCESS:    
	    return "SUCCESS";/*
    case INVALID_WRITE:
        return "INVALID_WRITE";
    case PAGE_FAULT:
        return "PAGE_FAULT";
    case INVALID_RESOURCE:
        return "INVALID_RESOURCE";
    case FIN_QUANTUM:
        return "FIN_QUANTUM";
    case NUEVA_PRIORIDAD:
        return "NUEVA_PRIORIDAD";
    case DEADLOCK:
        return "DEADLOCK";*/
    case INTERRUPTED_BY_USER:
        return "INTERRUPTED_BY_USER";
    /*case ABRE_ARCHIVO_W:
        return "ABRE_ARCHIVO_W";
    case ABRE_ARCHIVO_R:
        return "ABRE_ARCHIVO_R";*/
    default:
        return "ERROR";
	}
}

void wait_recurso(t_pcb *pcb, char *recurso_recibido){
    
	
	// si el recurso que me pide no existe lo mando a exit
	int posicion_recurso = encontrar_posicion_recurso(recurso_recibido);
	if(posicion_recurso == -1){
		log_error(kernel_logger, "No se encontro el recurso %s", recurso_recibido);
		sacar_de_exec(pcb, INVALID_RESOURCE);
		return;
	}

	// si el recurso no esta disponible --> se agrega a la cola de bloqueados por ese recurso
	if (sem_trywait(&semaforo_recursos[posicion_recurso]) >= 0) { // si lo esta --> lo decrementa y va a ready
		//sem_wait(&semaforo_recursos[posicion_recurso]);
		agregar_a_ready(pcb);
	}else {
		sem_wait(&mutex_recursos);
			queue_push(plani_block_recursos[posicion_recurso], pcb);
		sem_post(&mutex_recursos);
		//lo manejo
		sacar_de_exec(pcb, ESPERA_RECURSO);// Bloqueado hasta q otro haga un signal del recurso que quiere	y lo mando a ready
	} 
    
}
void signal_recurso(t_pcb *pcb, char *recurso_recibido){

	// si el recurso que me pide no existe lo mando a exit
	int posicion_recurso = encontrar_posicion_recurso(recurso_recibido);
	if(posicion_recurso == -1){
		log_error(kernel_logger, "No se encontro el recurso %s", recurso_recibido);
		sacar_de_exec(pcb, INVALID_RESOURCE);
		return;
	}

	// le subo una instancia al recurso
	sem_post(&semaforo_recursos[posicion_recurso]);


	// si hay pcbs en cola para el recurso --> saco el siguiente, lo pongo en ready, y bajo una instancia
	if(!queue_is_empty(plani_block_recursos[posicion_recurso])){
		sem_wait(&mutex_recursos);
			t_pcb * pcb_bloqueado = queue_pop(plani_block_recursos[posicion_recurso]);
		sem_post(&mutex_recursos);

		sacar_de_bloqueado(pcb_bloqueado);
		agregar_a_ready(pcb_bloqueado);

		sem_wait(&semaforo_recursos[posicion_recurso]);

	}
}



int encontrar_posicion_recurso(char* target_char) {
		int position = -1;
		int cant_recursos =  string_array_size(RECURSOS);

		for (int i = 0; cant_recursos; i++) {
			if (strcmp(RECURSOS[i], target_char) == 0) {
				position = i;
				break;
			}
		}
		return position;
}

void mostrar_pids_y_estados() {
	char* pids_estados = string_new();
	char* estados[] = {"NEW", "READY", "EXEC", "BLOCKED", "EXIT"};
	for (int i = 0; i < 5; i++) {
		string_append(&pids_estados, estados[i]);
		string_append(&pids_estados, ":\n");
		for (int j = 0; j < list_size(total_pcbs); j++) {
			t_pcb* pcb = list_get(total_pcbs, j);
			if (pcb->estado == i) {
				char* pid_str = string_itoa(pcb->pid);
				string_append(&pids_estados, "PID: ");
				string_append(&pids_estados, pid_str);
				string_append(&pids_estados, "\n");
				free(pid_str);
			}			
		}
		string_append(&pids_estados, "\n");
	}
	log_info(kernel_logger, "Lista total_pcbs:\n%s", pids_estados);
	free(pids_estados);
}
