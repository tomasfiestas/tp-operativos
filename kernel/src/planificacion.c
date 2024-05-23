#include "planificacion.h"

t_pcb* pcb_nuevo;
char** recursos;
char** instancias_recursos;


//Listas de estados
t_queue* plani_new;
t_list* plani_ready;
t_list* total_pcbs;
t_list* plani_exec;
t_list* plani_block;
t_queue** plani_block_recursos;
t_list* plani_exit;

//t_list* lista_pcbs_sockets; //No se que es?


algoritmos  algoritmo_plani;



int multiprogramacion;

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
sem_t sem_recursos;
sem_t sem_inst_recursos;


void crear_pcb(int pid){
	t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    nuevo_pcb->pid = pid;	
	nuevo_pcb->program_counter = 0;	
	//nuevo_pcb->tabla_archivos = list_create(); //Comento porque no se para que sirve
	nuevo_pcb->estado = NEW;
	nuevo_pcb->ejecuto = 0;
	nuevo_pcb->quantum = QUANTUM;

	
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
	sem_init(&lugares_ready_llenos, 0, 0);
	sem_init(&lugares_ready_vacios,0, GRADO_MULTIPROGRAMACION);
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
	
}


void inicializar_listas(){
	log_info(kernel_logger, "Algoritmo planificacion: %s", ALGORITMO_PLANIFICACION);
	
	
	algoritmo_plani = obtener_algoritmo();
	


	
	// Inicio listas
	plani_new = queue_create();
	total_pcbs = list_create();
	plani_ready = list_create();
	plani_exec = list_create();
	plani_block = list_create();
    plani_exit = list_create();


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
	while(1){
		// Espero a que haya procesos en new 			
		sem_wait(&hayPCBsEnNew);
		log_info(kernel_logger, "Planificador Largo PLazo: Hay PCBs en NEW.");
		
		// Espero a que el grado de multiprogramacion me permita agregar un proceso a RAM
		sem_wait(&lugares_ready_vacios);
		log_info(kernel_logger, "Planificador Largo PLazo: Multiprogramacion permite ingresar a RAM.");
		
		//Se agrega el pcb a READY
		t_pcb* pcb = sacar_siguiente_de_new();
		// cambiar_estado_pcb(pcb, READY); pongo esto en la funcion agregar_a_ready asi no lo hacemos cada vez
		agregar_a_ready(pcb);
		log_info(kernel_logger, "Planificador Largo PLazo: Se agrego un nuevo proceso a READY");
		
	}

	return NULL;
}



// ---------------------------------------------------
//             PLANIFICADOR CORTO PLAZO
// ---------------------------------------------------
void* inicio_plani_corto_plazo(void* arg){
	//int pcb_nuevo = 1;

	while(1){
		sem_wait(&hayPCBsEnReady);
		  log_info(kernel_logger, "Planificador Corto PLazo: Hay PCBs en READY.");

		sem_wait(&puedeEntrarAExec);
		  log_info(kernel_logger, "Planificador Corto PLazo: PCB puede entrar a EXEC.");

		t_pcb* pcb;
		
		pcb = sacar_de_ready();
			agregar_a_exec(pcb);
		

		//MANDAR CONTEXTO A CPU PARA QUE EJECUTE		
  		mandar_contexto_a_CPU(pcb);

		//ESPERAR RESPUESTA DE CPU PARA SACAR PCB DE EXEC O LO QUE SEA QUE SE HAGA
		//cod_op operacion_recibida = recibir_operacion(socket_cpu_plani);

//aca podriamos correr un hilo para q me empiece a contar el quantum
         if(algoritmo_plani == RR){
                pthread_t hilo_quantum;
                pthread_create(&hilo_quantum, NULL, (void *)contar_quantum, NULL);
				pthread_detach(hilo_quantum);
        	}

		//IMPLEMENTAR COMO VAMOS A RECIBIR EL CONTEXTO DE EJECUCIón
		// lo tengo q recibir junto con un motivo de desalojo, idea:

		op_code op_code = recibir_operacion(conexion_cpu_interrupt);
		t_buffer* buffer = recibir_buffer(conexion_cpu_interrupt);
		t_pcb pcbDesalojado = recibir_contexto_ejecucion(buffer); 
		log_info(kernel_logger, "pid: %d", pcbDesalojado.pid);
		
		sacar_de_exec(pcb, op_code);
			

	}

}

void iniciar_planificacion(){
	t_pcb* pcb;
	while (1){

		sem_wait(&lugares_ready_llenos);
		sem_wait(&mutex_multiprogramacion);
			pcb = sacar_de_ready();
			cambiar_estado_pcb(pcb, EXEC);
			agregar_a_exec(pcb);
			// mandar a CPU contexto de ejecucion
			mandar_contexto_a_CPU(pcb);
		sem_post(&mutex_multiprogramacion);
		sem_post(&lugares_ready_vacios);
		
	}
}

// -------------------------------------------------
//             FUNCIONES GENERALES
// -------------------------------------------------



void agregar_a_new(t_pcb* nuevo_pcb){
	sem_wait(&sem_new);
		queue_push(plani_new, nuevo_pcb);
		log_info(kernel_logger, "Se crea el proceso %d en NEW", nuevo_pcb->pid);
	sem_post(&sem_new);
	sem_post(&hayPCBsEnNew);	
	log_info(kernel_logger, "le mandé el post a PCB");	
}

void agregar_a_ready(t_pcb* nuevo_pcb){
	//semaforo tipo productor-consumidor
	//kernel productor va a esperar a q haya espacio para un proceso en ready segun multip
	// sem_wait(&lugares_ready_vacios); --> lo pongo en plani largo plazo

	sem_wait(&mutex_multiprogramacion);
		list_add(plani_ready, nuevo_pcb);
	sem_post(&mutex_multiprogramacion);
	cambiar_estado_pcb(nuevo_pcb, READY);
	sem_post(&hayPCBsEnReady);
}

t_pcb* sacar_siguiente_de_new(){
	sem_wait(&sem_new);
	t_pcb* pcb = queue_pop(plani_new);
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
	//ESTO es RR en realidad pero le pongo VRR para probar con lo que está configurado.
	case VRR: 
		sem_wait(&sem_ready);
        pcb = list_remove(plani_ready, 0);
        sem_post(&sem_ready);
		return pcb;

	//TODO IMPLEMENTAR MAGIAS de VRR

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
		list_remove(plani_exec, pcb);
	sem_post(&sem_exec);
		switch (op_code)
			{
			case IO:
				agregar_a_bloqueado(pcb);
			break;
			case FINDEQUANTUM:
				agregar_a_ready(pcb);
			break;
			default: // FINPROCESO
				agregar_a_exit(pcb);
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

void agregar_a_exit(t_pcb* pcb){
	sem_wait(&sem_exit);
		list_add(plani_exit, pcb);
	sem_post(&sem_exit);
	cambiar_estado_pcb(pcb, EXIT);
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
	t_paquete* paquete_cpu = crear_paquete(CONTEXTO_EJECUCION, buffer_cpu);
    enviar_paquete(paquete_cpu, conexion_cpu_dispatch);
}

void enviar_interrupcion_por_quantum(t_pcb* pcb){
    t_buffer* buffer_cpu = crear_buffer();    
    cargar_pcb_a_buffer(buffer_cpu,pcb);    
	t_paquete* paquete_cpu = crear_paquete(FIN_DE_QUANTUM, buffer_cpu);
    enviar_paquete(paquete_cpu, conexion_cpu_interrupt);
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
	case FIN:
		return "EXIT";
	default:
		return "ESTADO DESCONOCIDO";
	}
}