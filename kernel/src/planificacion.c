#include "planificacion.h"

void crear_pcb(t_list* lista_instrucciones){
	t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    int pid = asignar_pid();
	nuevo_pcb->pid = pid;
	nuevo_pcb->instrucciones = lista_instrucciones;
	nuevo_pcb->program_counter = 0;	
	nuevo_pcb->tabla_archivos = list_create();
	nuevo_pcb->estado = NEW;
	nuevo_pcb->ejecuto = 0;

	inicializar_semaforos();
	inicializar_registros(nuevo_pcb);
	inicializar_listas();

	// agrego el nuevo proceso a la lista total_pcbs
	sem_wait(&sem_total_pcbs);
	list_add(total_pcbs, nuevo_pcb);
	sem_post(&sem_total_pcbs);

	agregar_a_new(nuevo_pcb);
	log_info(kernel_logger, "Nuevo proceso %d en NEW", pid);

	agregar_a_ready(nuevo_pcb);
	log_info(kernel_logger, "Nuevo proceso %d en NEW", pid);
	
}

void inicializar_registros(t_pcb* nuevo_pcb){
	uint8_t ax = 0;
	uint32_t eax = 0;	
	memcpy(&(nuevo_pcb->registros.AX), ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.BX), ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.CX), ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.DX), ax, sizeof(uint8_t));
	memcpy(&(nuevo_pcb->registros.EAX), eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.EBX), eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.ECX), eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.EDX), eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.SI), eax, sizeof(uint32_t));
	memcpy(&(nuevo_pcb->registros.DI), eax, sizeof(uint32_t));
}

void agregar_a_new(t_pcb* nuevo_pcb){
	sem_wait(&sem_new);
		queue_push(plani_new, nuevo_pcb);
	sem_post(&sem_new);
	sem_post(&hayPCBsEnNew);
}

void agregar_a_ready(t_pcb* nuevo_pcb){
	//semaforo tipo productor-consumidor
	//kernel productor va a esperar a q haya espacio para un proceso en ready segun multip
	sem_wait(&lugares_ready_vacios);
	sem_wait(&mutex_multiprogramacion);
		list_add(plani_ready, nuevo_pcb);
	sem_post(&mutex_multiprogramacion);
	sem_post(&lugares_ready_llenos);
}



//Inicio semaforos

void inicializar_semaforos(){
	sem_init(&lugares_ready_llenos, 0, 0);
	sem_init(&lugares_ready_vacios,0, multiprogramacion);
	sem_init(&mutex_multiprogramacion, 0, 1);
	sem_init(&hayPCBsEnNew, 0, 0);
	sem_init(&sem_new, 0, 1);
	sem_init(&sem_total_pcbs, 0, 1);
	sem_init(&sem_ready, 0, 1);
	sem_init(&sem_exec, 0, 1);
	sem_init(&sem_block, 0, 1);
	sem_init(&sem_exit, 0, 1);
	
}


void inicializar_listas(){
	// Inicio listas
	plani_new = queue_create();
	total_pcbs = list_create();
	plani_ready = list_create();
	plani_exec = list_create();
	plani_block = list_create();
    plani_exit = list_create();
} 
/*Creo hilos planificadores
	pthread_t hilo_plani_largo_plazo;
	pthread_create(&hilo_plani_largo_plazo, NULL, (void *)inicio_plani_largo_plazo, NULL);
	pthread_detach(hilo_plani_largo_plazo);
	*/

void iniciar_planificacion(){
	t_pcb* pcb = 1;
	while (1){

		sem_wait(&lugares_ready_llenos);
		sem_wait(&mutex_multiprogramacion);
			pcb = sacar_de_ready();
			cambiar_estado_pcb(pcb, EXEC);
			agregar_a_exec(pcb);
		sem_post(&mutex_multiprogramacion);
		sem_post(&lugares_ready_vacios);
		
	}
}

t_pcb* sacar_de_ready(){
	t_pcb* pcb;
	if(algoritmo_plani == FIFO){
		sem_wait(&sem_ready);
		pcb = list_remove(plani_ready, 0);
		sem_post(&sem_ready);
		return pcb;
	}else
		return NULL;
	
}

void agregar_a_exec(t_pcb* pcb){
	sem_wait(&sem_exec);
		pcb->ejecuto = 1;
		list_add(plani_exec, pcb);
	sem_post(&sem_exec);
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