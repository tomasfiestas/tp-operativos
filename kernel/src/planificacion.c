#include "planificacion.h"

void crear_pcb(t_list* lista_instrucciones, int socket_consola){
	t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    int pid = asignar_pid();
	nuevo_pcb->pid = pid;
	nuevo_pcb->instrucciones = lista_instrucciones;
	nuevo_pcb->program_counter = 0;	
	nuevo_pcb->tabla_archivos = list_create();
	nuevo_pcb->estado = NEW;
	nuevo_pcb->ejecuto = 0;
	inicializar_registros(nuevo_pcb);

	//pcb_nuevo = nuevo_pcb;

	

	//nuevo_pcb = pcb_nuevo;

	log_info(kernel_logger, "Se crea el proceso %d en NEW", pid);

	
	sem_wait(&sem_total_pcbs);
	list_add(total_pcbs, nuevo_pcb);
	sem_post(&sem_total_pcbs);

	agregar_a_new(nuevo_pcb);

	
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

//Inicio semaforos
	inicio_semaforos();

	// Inicio listas
	plani_new = queue_create();
	total_pcbs = list_create();
	plani_ready = list_create();
	plani_exec = list_create();
	plani_block = list_create();
    plani_exit = list_create();
    
//Creo hilos planificadores
	pthread_t hilo_plani_largo_plazo;
	pthread_create(&hilo_plani_largo_plazo, NULL, (void *)inicio_plani_largo_plazo, NULL);
	pthread_detach(hilo_plani_largo_plazo);

	pthread_t hilo_plani_corto_plazo;
	pthread_create(&hilo_plani_corto_plazo, NULL, (void *)inicio_plani_corto_plazo, NULL);
	pthread_detach(hilo_plani_corto_plazo);