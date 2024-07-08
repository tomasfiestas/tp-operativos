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
t_queue * cola_prioritaria_vrr; // cola auxiliar prioritaria de VRR
t_queue** plani_block_recursos;
t_list* plani_exit;
bool running = false;

//t_list* lista_pcbs_sockets; //No se que es?


algoritmos  algoritmo_plani;



int multiprogramacion;
int gradosPendientes = 0;

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
sem_t sem_aux; //para la cola auxiliar prioritaria
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
	sem_init(&sem_aux, 0, 1);
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
	lista_interfaces = list_create();
	plani_new = list_create();
	total_pcbs = list_create();
	plani_ready = list_create();
	plani_exec = list_create();
	plani_block = list_create();
	cola_prioritaria_vrr = queue_create();
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
	//sem_wait(&planificacion_largo_plazo_activa);
	//int valor = 0;
	//sem_getvalue(&planificacion_largo_plazo_activa, &valor);

	while(1){		
		sem_wait(&planificacion_largo_plazo_activa);
		while(running){
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
		
		//sem_getvalue(&planificacion_largo_plazo_activa, &valor);
		//sem_post(&planificacion_largo_plazo_activa);
		}
	}

	return NULL;
}



// ---------------------------------------------------
//             PLANIFICADOR CORTO PLAZO
// ---------------------------------------------------
void* inicio_plani_corto_plazo(void* arg){
	//sem_wait(&planificacion_corto_plazo_activa);
	//int valor_corto_plazo = 0;
	//sem_getvalue(&planificacion_corto_plazo_activa, &valor_corto_plazo);
	
	while(1){
		sem_wait(&planificacion_corto_plazo_activa);
		while(running){
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
		//sem_getvalue(&planificacion_corto_plazo_activa, &valor_corto_plazo);
		
		//pthread_cancel(hilo_quantum);	

		//sem_post(&planificacion_corto_plazo_activa);
		}
	}

}

void iniciar_planificacion(){	
		
		running = true;
		sem_post(&planificacion_largo_plazo_activa);	
		//sem_post(&planificacion_largo_plazo_activa);
		//sem_post(&planificacion_corto_plazo_activa);
		sem_post(&planificacion_corto_plazo_activa);		
	
}
void detener_planificacion(){
	running = false;
	//sem_wait(&planificacion_corto_plazo_activa);
	//printf("detengo corto plazo");
	//sem_wait(&planificacion_largo_plazo_activa);
	
}	

void resetear_semaforos_multi(int vieja_multi){

	int i;
	if ( GRADO_MULTIPROGRAMACION > vieja_multi){
		for(i=0; i < GRADO_MULTIPROGRAMACION - vieja_multi; i++){
			sem_post(&multiPermiteIngresar);
		}
	}else{
		for(i=0; i < vieja_multi - GRADO_MULTIPROGRAMACION; i++){
			if(sem_trywait(&multiPermiteIngresar) == -1){
				log_info(kernel_logger, "No se puede reducir mas el grado de multiprogramacion ya que se encuentra completo, se veran los cambios una vez que finalice un proceso");
				gradosPendientes ++;
			}
		}
	}
	log_info(kernel_logger, "Semaforos de multiprogramacion reseteados");

}
void finalizar_proceso(t_buffer* buffer){    
    int pid= extraer_int_del_buffer(buffer); 
    printf("El proceso a finalizar es: %d\n", pid);         
    
    destruir_buffer(buffer);   
    

    t_pcb * pcb_a_finalizar = buscarPcb(pid);
    printf("Cuyo estado es: %d\n", pcb_a_finalizar->estado ); 

    if(pcb_a_finalizar->estado == EXEC){ 
    //Le aviso a CPU interrupt que quiero terminar un proceso
    t_buffer* buffer_cpu_interrupt = crear_buffer();
    pthread_cancel(hilo_quantum); //Detengo el hilo que cuenta el quantum
    cargar_pcb_a_buffer(buffer_cpu_interrupt, pcb_a_finalizar);
    t_paquete* paquete_cpu = crear_paquete(FINPROCESO, buffer_cpu_interrupt);
    enviar_paquete(paquete_cpu, conexion_cpu_interrupt);

    }

    else{
        sacar_pcb_de_lista(pcb_a_finalizar);
        agregar_a_exit(pcb_a_finalizar,INTERRUPTED_BY_USER);
    }
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

void agregar_a_cola_prioritaria(t_pcb * pcb){
	sem_wait(&sem_aux);
		queue_push(cola_prioritaria_vrr, pcb);
		log_info(kernel_logger, "El proceso %d ingreso a la cola prioritaria");
	sem_post(&sem_aux);	
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
		if(queue_is_empty(cola_prioritaria_vrr)){
			sem_wait(&sem_ready);
    			pcb = list_remove(plani_ready, 0);
        	sem_post(&sem_ready);	
		}else{ // si es VRR va a priorizar sacar de la cola auxiliar
			sem_wait(&sem_aux);
    			pcb = queue_pop(cola_prioritaria_vrr);
        	sem_post(&sem_aux);	
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
	agregar_a_ready(pcb);	
}

void agregar_a_exit(t_pcb* pcb,op_code motivo_a_mostrar){
	sem_wait(&sem_exit);
		list_add(plani_exit, pcb);
	sem_post(&sem_exit);
	cambiar_estado_pcb(pcb, EXIT);
	if (gradosPendientes == 0){
		sem_post(&multiPermiteIngresar);
	}else{
		gradosPendientes --;
	}
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

	t_pcb * pcb_enLista = buscarPcb(pcb->pid);
	log_info(kernel_logger, "hola");
	pcb_enLista->estado = estadoNuevo;

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
	//pthread_cancel(hilo_quantum); //TODO REVISAR PORQUE EN FIFO NO HAY QUANTUM
	log_info(kernel_logger,"Cancelo HILO QUANTUM %d",hilo_quantum);  //ACTUALIZAR LOS DATOS DE LA LSITA TOTAL DE PCBS
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
			//t_buffer* buffer4 = recibir_buffer(cliente_kd);			        
			atender_fin_proceso_success(buffer,op_code);
			break;

		// RECUSOS -----------------------------------------------------------
		case SOLICITAR_WAIT:
			char * recurso_wait = extraer_string_del_buffer2(buffer);

			destruir_buffer(buffer);
			wait_recurso(pcb,recurso_wait);
            //free(recurso_wait);

            break;
		case SOLICITAR_SIGNAL:
			log_info(kernel_logger, "Llegó solicitud de signal");
			char * recurso_signal = extraer_string_del_buffer2(buffer);

			signal_recurso(pcb,recurso_signal);          

            break;
		// ENTRADA SALIDA -----------------------------------------------------------
		case IO_GEN_SLEEP:
			log_info(kernel_logger,"LLegó un IO_GEN_SLEEP");
			sacar_de_exec(pcb,IO);
			char * nombre_interfaz_solicitada1 = extraer_string_del_buffer(buffer);
			char* unidades_trabajo1 = extraer_string_del_buffer(buffer);
			log_info(kernel_logger, "nombre de la interfaz %s", nombre_interfaz_solicitada1);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz1 = buscar_interfaz(nombre_interfaz_solicitada1);
			sem_post(&mutex_lista_interfaces);	
			
			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada			if (! validar_interfaz_e_instruccion(pcb, interfaz1, op_code))
				break;	
					
			if(sem_trywait(&interfaz1->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz1->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz, nombre_interfaz_solicitada1);
				cargar_string_a_buffer(buffer_interfaz, unidades_trabajo1);
				cargar_int_a_buffer(buffer_interfaz, pcb->pid);
				t_paquete* paquete_interfaz = crear_paquete(IO_GEN_SLEEP, buffer_interfaz);
				enviar_paquete(paquete_interfaz, interfaz1->fd_interfaz);
				destruir_buffer(buffer_interfaz);		
			}
			else{
				t_lista_block* lista_bloqueados1 = malloc(sizeof(t_lista_block));
				lista_bloqueados1->pcb = pcb;
				lista_bloqueados1->operacion = op_code;
				list_add(lista_bloqueados1->parametros, unidades_trabajo1);
				queue_push(interfaz1->cola_procesos_bloqueados,lista_bloqueados1);
			}
			
			break;
		case IO_STDIN_READ:
			log_info(kernel_logger,"LLegó un IO_STDIN_READ");
			sacar_de_exec(pcb,IO);

			char* nombre_interfaz_solicitada2 = extraer_string_del_buffer(buffer);
			char* registro_direccion2 = extraer_string_del_buffer(buffer);
			char* registro_tamanio2 = extraer_string_del_buffer(buffer);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz2 = buscar_interfaz(nombre_interfaz_solicitada2);
			sem_post(&mutex_lista_interfaces);

			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada
			if (! validar_interfaz_e_instruccion(pcb, interfaz2, op_code))
			break;	

			if(sem_trywait(&interfaz2->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz2->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz2 = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz2, nombre_interfaz_solicitada2);
				cargar_string_a_buffer(buffer_interfaz2, registro_direccion2);
				cargar_string_a_buffer(buffer_interfaz2, registro_tamanio2);
				cargar_int_a_buffer(buffer_interfaz2, pcb->pid);
				t_paquete* paquete_interfaz2 = crear_paquete(IO_STDIN_READ, buffer_interfaz2);
				enviar_paquete(paquete_interfaz2, interfaz2->fd_interfaz);
				destruir_buffer(buffer_interfaz2);			
			}
			else{
				t_lista_block* lista_bloqueados2 = malloc(sizeof(t_lista_block));
				lista_bloqueados2->pcb = pcb;
				lista_bloqueados2->operacion = op_code;
				list_add(lista_bloqueados2->parametros, registro_direccion2);
				list_add(lista_bloqueados2->parametros, registro_tamanio2);
				queue_push(interfaz2->cola_procesos_bloqueados,lista_bloqueados2);
			}
		break;
		case IO_STDOUT_WRITE:
			log_info(kernel_logger,"LLegó un IO_STDOUT_WRITE");
			sacar_de_exec(pcb,IO);

			char* nombre_interfaz_solicitada3 = extraer_string_del_buffer(buffer);
			char* registro_direccion3 = extraer_string_del_buffer(buffer);
			char* registro_tamanio3 = extraer_string_del_buffer(buffer);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz3 = buscar_interfaz(nombre_interfaz_solicitada3);
			sem_post(&mutex_lista_interfaces);

			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada
			if (! validar_interfaz_e_instruccion(pcb, interfaz3, op_code))
			break;	

			if(sem_trywait(&interfaz3->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz3->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz3 = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz3, nombre_interfaz_solicitada3);
				cargar_string_a_buffer(buffer_interfaz3, registro_direccion3);
				cargar_string_a_buffer(buffer_interfaz3, registro_tamanio3);
				cargar_int_a_buffer(buffer_interfaz3, pcb->pid);
				t_paquete* paquete_interfaz3 = crear_paquete(IO_STDOUT_WRITE, buffer_interfaz3);
				enviar_paquete(paquete_interfaz3, interfaz3->fd_interfaz);
				destruir_buffer(buffer_interfaz3);			
			}
			else{
				t_lista_block* lista_bloqueados3 = malloc(sizeof(t_lista_block));
				lista_bloqueados3->pcb = pcb;
				lista_bloqueados3->operacion = op_code;
				list_add(lista_bloqueados3->parametros, registro_direccion3);
				list_add(lista_bloqueados3->parametros, registro_tamanio3);
				queue_push(interfaz3->cola_procesos_bloqueados,lista_bloqueados3);
			}
		break;
		case IO_FS_CREATE :
			log_info(kernel_logger,"LLegó un IO_FS_CREATE");
			sacar_de_exec(pcb,IO);

			char* nombre_interfaz_solicitada4 = extraer_string_del_buffer(buffer);
			char* nombre_archivo4 = extraer_string_del_buffer(buffer);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz4 = buscar_interfaz(nombre_interfaz_solicitada4);
			sem_post(&mutex_lista_interfaces);

			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada
			if (! validar_interfaz_e_instruccion(pcb, interfaz4, op_code))
			break;	

			if(sem_trywait(&interfaz4->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz4->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz4 = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz4, nombre_interfaz_solicitada4);
				cargar_string_a_buffer(buffer_interfaz4, nombre_archivo4);
				cargar_int_a_buffer(buffer_interfaz4, pcb->pid);
				t_paquete* paquete_interfaz4 = crear_paquete(IO_FS_CREATE , buffer_interfaz4);
				enviar_paquete(paquete_interfaz4, interfaz4->fd_interfaz);
				destruir_buffer(buffer_interfaz4);			
			}
			else{
				t_lista_block* lista_bloqueados4 = malloc(sizeof(t_lista_block));
				lista_bloqueados4->pcb = pcb;
				lista_bloqueados4->operacion = op_code;
				list_add(lista_bloqueados4->parametros, nombre_archivo4);
				queue_push(interfaz4->cola_procesos_bloqueados,lista_bloqueados4);
			}
		break;
		case IO_FS_DELETE:
			log_info(kernel_logger,"LLegó un IO_FS_DELETE");
			sacar_de_exec(pcb,IO);

			char* nombre_interfaz_solicitada5 = extraer_string_del_buffer(buffer);
			char* nombre_archivo5 = extraer_string_del_buffer(buffer);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz5 = buscar_interfaz(nombre_interfaz_solicitada5);
			sem_post(&mutex_lista_interfaces);

			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada
			if (! validar_interfaz_e_instruccion(pcb, interfaz5, op_code))
			break;	

			if(sem_trywait(&interfaz5->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz5->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz5 = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz5, nombre_interfaz_solicitada5);
				cargar_string_a_buffer(buffer_interfaz5, nombre_archivo5);
				cargar_int_a_buffer(buffer_interfaz5, pcb->pid);
				t_paquete* paquete_interfaz5 = crear_paquete(IO_FS_DELETE , buffer_interfaz5);
				enviar_paquete(paquete_interfaz5, interfaz5->fd_interfaz);
				destruir_buffer(buffer_interfaz5);			
			}
			else{
				t_lista_block* lista_bloqueados5 = malloc(sizeof(t_lista_block));
				lista_bloqueados5->pcb = pcb;
				lista_bloqueados5->operacion = op_code;
				list_add(lista_bloqueados5->parametros, nombre_archivo5);
				queue_push(interfaz5->cola_procesos_bloqueados,lista_bloqueados5);
			}
		break;
		case IO_FS_TRUNCATE:
			log_info(kernel_logger,"LLegó un IO_FS_TRUNCATE");
			sacar_de_exec(pcb,IO);

			char* nombre_interfaz_solicitada6 = extraer_string_del_buffer(buffer);
			char* nombre_archivo6 = extraer_string_del_buffer(buffer);
			char* registro_tamanio6 = extraer_string_del_buffer(buffer);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz6 = buscar_interfaz(nombre_interfaz_solicitada6);
			sem_post(&mutex_lista_interfaces);


			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada
			if (! validar_interfaz_e_instruccion(pcb, interfaz6, op_code))
			break;	

			if(sem_trywait(&interfaz6->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz6->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz6 = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz6, nombre_interfaz_solicitada6);
				cargar_string_a_buffer(buffer_interfaz6, nombre_archivo6);
				cargar_string_a_buffer(buffer_interfaz6, registro_tamanio6);
				cargar_int_a_buffer(buffer_interfaz6, pcb->pid);
				t_paquete* paquete_interfaz6 = crear_paquete(IO_FS_TRUNCATE , buffer_interfaz6);
				enviar_paquete(paquete_interfaz6, interfaz6->fd_interfaz);
				destruir_buffer(buffer_interfaz6);			
			}
			else{
				t_lista_block* lista_bloqueados6 = malloc(sizeof(t_lista_block));
				lista_bloqueados6->pcb = pcb;
				lista_bloqueados6->operacion = op_code;
				list_add(lista_bloqueados6->parametros, nombre_archivo6);
				list_add(lista_bloqueados6->parametros, registro_tamanio6);
				queue_push(interfaz6->cola_procesos_bloqueados,lista_bloqueados6);
			}
		break;
		case IO_FS_WRITE:
			log_info(kernel_logger,"LLegó un IO_FS_WRITE");
			sacar_de_exec(pcb,IO);

			char* nombre_interfaz_solicitada7 = extraer_string_del_buffer(buffer);
			char* nombre_archivo7 = extraer_string_del_buffer(buffer);
			char* registro_direccion7 = extraer_string_del_buffer(buffer);
			char* registro_tamanio7 = extraer_string_del_buffer(buffer);
			char* registro_puntero_archivo7 = extraer_string_del_buffer(buffer);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz7 = buscar_interfaz(nombre_interfaz_solicitada7);
			sem_post(&mutex_lista_interfaces);

			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada
			if (! validar_interfaz_e_instruccion(pcb, interfaz7, op_code))
			break;	

			if(sem_trywait(&interfaz7->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz7->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz7 = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz7, nombre_interfaz_solicitada7);
				cargar_string_a_buffer(buffer_interfaz7, nombre_archivo7);
				cargar_string_a_buffer(buffer_interfaz7, registro_direccion7);
				cargar_string_a_buffer(buffer_interfaz7, registro_tamanio7);
				cargar_string_a_buffer(buffer_interfaz7, registro_puntero_archivo7);
				cargar_int_a_buffer(buffer_interfaz7, pcb->pid);
				t_paquete* paquete_interfaz7 = crear_paquete(IO_FS_WRITE  , buffer_interfaz7);
				enviar_paquete(paquete_interfaz7, interfaz7->fd_interfaz);
				destruir_buffer(buffer_interfaz7);			
			}
			else{
				t_lista_block* lista_bloqueados7 = malloc(sizeof(t_lista_block));
				lista_bloqueados7->pcb = pcb;
				lista_bloqueados7->operacion = op_code;
				list_add(lista_bloqueados7->parametros, nombre_archivo7);
				list_add(lista_bloqueados7->parametros, registro_direccion7);
				list_add(lista_bloqueados7->parametros, registro_tamanio7);
				list_add(lista_bloqueados7->parametros, registro_puntero_archivo7);
				queue_push(interfaz7->cola_procesos_bloqueados,lista_bloqueados7);
			}
		break;
		case IO_FS_READ:
			log_info(kernel_logger,"LLegó un IO_FS_READ");
			sacar_de_exec(pcb,IO);

			char* nombre_interfaz_solicitada8 = extraer_string_del_buffer(buffer);
			char* nombre_archivo8 = extraer_string_del_buffer(buffer);
			char* registro_direccion8 = extraer_string_del_buffer(buffer);
			char* registro_tamanio8 = extraer_string_del_buffer(buffer);
			char* registro_puntero_archivo8 = extraer_string_del_buffer(buffer);

			sem_wait(&mutex_lista_interfaces);
				t_entrada_salida* interfaz8 = buscar_interfaz(nombre_interfaz_solicitada8);
			sem_post(&mutex_lista_interfaces);

			//valido si la interfaz existe/esta conectada y si soporta la instruccion solicitada
			if (! validar_interfaz_e_instruccion(pcb, interfaz8, op_code))
			break;	

			if(sem_trywait(&interfaz8->sem_disponible) ==0 ){
				//MANDAR A TOMI.
				interfaz8->pid_usandola = pcb->pid;
				t_buffer* buffer_interfaz8 = crear_buffer();
				cargar_string_a_buffer(buffer_interfaz8, nombre_interfaz_solicitada8);
				cargar_string_a_buffer(buffer_interfaz8, nombre_archivo8);
				cargar_string_a_buffer(buffer_interfaz8, registro_direccion8);
				cargar_string_a_buffer(buffer_interfaz8, registro_tamanio8);
				cargar_string_a_buffer(buffer_interfaz8, registro_puntero_archivo8);
				cargar_int_a_buffer(buffer_interfaz8, pcb->pid);
				t_paquete* paquete_interfaz8 = crear_paquete(IO_FS_READ  , buffer_interfaz8);
				enviar_paquete(paquete_interfaz8, interfaz8->fd_interfaz);
				destruir_buffer(buffer_interfaz8);			
			}
			else{
				t_lista_block* lista_bloqueados8 = malloc(sizeof(t_lista_block));
				lista_bloqueados8->pcb = pcb;
				lista_bloqueados8->operacion = op_code;
				list_add(lista_bloqueados8->parametros, nombre_archivo8);
				list_add(lista_bloqueados8->parametros, registro_direccion8);
				list_add(lista_bloqueados8->parametros, registro_tamanio8);
				list_add(lista_bloqueados8->parametros, registro_puntero_archivo8);
				queue_push(interfaz8->cola_procesos_bloqueados,lista_bloqueados8);
			}
		break;

		default:
			log_error(kernel_logger, "No se reconoce el handshake");
			break;
	}
	llego_contexto = false;
	log_info(kernel_logger,"llego contexto en FALSE");
	

}

int validar_interfaz_e_instruccion(t_pcb * pcb, t_entrada_salida * interfaz, op_code op_code){
	
	//log_info(kernel_logger, "nombre de la interfaz:  %d",interfaz->nombre);
	if(interfaz == NULL){
		log_error(kernel_logger, "No se encontró la interfaz solicitada, mando proceso a exit");
		agregar_a_exit(pcb, INVALID_INTERFACE);
		return 0;
	}
	int instruccion_valida = validar_instruccion_interfaz(interfaz,op_code);
	if(instruccion_valida == 0){
		log_error(kernel_logger, "Instrucción no soportada, mando proceso a exit");
		agregar_a_exit(pcb, INVALID_INTERFACE);
		return 0;
	}
	return 1;
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
t_pcb *buscarPcbBloqueado(int pid_a_buscar)
{
t_pcb *pcb;
   
        for (int i = 0; i < (list_size(plani_block)); i++) {
        pcb = list_get(plani_block,i);
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
            sem_wait(&hayPCBsEnNew);
            break;
        case BLOCK:
			sacar_de_lista(plani_block, pcb->pid);
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
	    return "SUCCESS";
    /*case INVALID_WRITE:
        return "INVALID_INTERFACE";*/
    case INVALID_INTERFACE:
        return "INVALID_INTERFACE";/*
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

	
	if (sem_trywait(&semaforo_recursos[posicion_recurso]) >= 0) { // si lo esta --> lo decrementa y va a ready
		agregar_a_ready(pcb);
	}else { // si el recurso no esta disponible --> se agrega a la cola de bloqueados por ese recurso y a la lista bloqueados (estado == block)
		sem_wait(&mutex_recursos);
			queue_push(plani_block_recursos[posicion_recurso], pcb);
		sem_post(&mutex_recursos);
		agregar_a_bloqueado(pcb);
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

	// si existe, le subo una instancia al recurso
	sem_post(&semaforo_recursos[posicion_recurso]);

	// si hay pcbs en cola para el recurso --> saco el siguiente, lo pongo en ready, y bajo una instancia
	if(!queue_is_empty(plani_block_recursos[posicion_recurso])){
		sem_wait(&mutex_recursos);
			t_pcb * pcb_bloqueado = queue_pop(plani_block_recursos[posicion_recurso]);
		sem_post(&mutex_recursos);

		sacar_de_bloqueado(pcb_bloqueado);
		if (algoritmo_plani == VRR){
			agregar_a_cola_prioritaria(pcb_bloqueado);
		}else{
			agregar_a_ready(pcb_bloqueado);
		} 
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

t_entrada_salida* buscar_interfaz(char* nombre) {
	for (int i = 0; i < list_size(lista_interfaces); i++) {
		t_entrada_salida* entrada_salida = list_get(lista_interfaces, i);
		if (strcmp(entrada_salida->nombre, nombre) == 0) {
			return entrada_salida;			
		}		
	}
	return NULL;
}

int validar_instruccion_interfaz(t_entrada_salida* t_entrada_salida,op_code op_code){
	switch(op_code){
		case IO_GEN_SLEEP:
			if(strcmp(t_entrada_salida->tipo, "GENERICA")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		case IO_STDIN_READ:
			if(strcmp(t_entrada_salida->tipo, "STDIN")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		case IO_STDOUT_WRITE:
			if(strcmp(t_entrada_salida->tipo, "STDOUT")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		case IO_FS_CREATE:
			if(strcmp(t_entrada_salida->tipo, "DIALFS")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		case IO_FS_DELETE:
			if(strcmp(t_entrada_salida->tipo, "DIALFS")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		case IO_FS_TRUNCATE:
			if(strcmp(t_entrada_salida->tipo, "DIALFS")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		case IO_FS_WRITE:
			if(strcmp(t_entrada_salida->tipo, "DIALFS")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		case IO_FS_READ:
			if(strcmp(t_entrada_salida->tipo, "DIALFS")==0){
				return 1;
			}else{
				return 0;
			}
			break;
		default:
			return 0;
			break;
	}
	
}

