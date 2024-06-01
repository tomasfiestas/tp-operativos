#include "servicios_kernel.h"

t_list *recursos_del_sistema;
t_list *lista_recursos_bloqueados;

int asignar_pid(){
    int valor_pid;

    pthread_mutex_lock(&mutex_pid);
    valor_pid = identificador_PID;
    identificador_PID++;
    pthread_mutex_unlock(&mutex_pid);
    return valor_pid;
}

void inicializar_colas_bloqueo_de_recusos(){
    
    //char** recursos = recursos_config();
    
    //creamos inicialmente una cola por cada recurso que hay en el archivo de config
    int cant_recursos = string_array_size(RECURSOS);
    
    /*for(int i = 0; i < cant_recursos; i++){
        t_cola_block *recurso_existente = malloc(sizeof(t_cola_block));
        //recurso_existente->identificador = strdup(RECURSOS[i]);
        recurso_existente->cola_bloqueados_recurso = queue_create();
        //recurso_existente->tipo = "recurso";
        list_add(lista_recursos_bloqueados, recurso_existente);
        log_info(kernel_logger, "Se creo la cola de bloqueo para el recurso %s", recurso_existente->identificador);
    }*/  
    for(int i = 0; i < cant_recursos; i++){
        t_queue *cola_bloqueados_recurso = queue_create();
        list_add(lista_recursos_bloqueados, cola_bloqueados_recurso);
        log_info(kernel_logger, "Se creo la cola de bloqueo para el recurso %s", RECURSOS[i]);
    }  
    
}

void crear_lista_recursos(){
    //char **recursos_aux = recursos_config();
    
    //char **instancias_aux = instancias_recursos_config();
    
    recursos_del_sistema = list_create();
    

    for(int i = 0; i < (string_array_size(RECURSOS)); i++){
        t_recurso *nuevo_recurso = malloc(sizeof(t_recurso));    

        nuevo_recurso->nombre = strdup(RECURSOS[i]);        
        list_add(recursos_del_sistema, nuevo_recurso);
        log_info(kernel_logger, "Se creo el recurso %s con %d instancias"
        , nuevo_recurso->nombre, nuevo_recurso->cantidad);

    }
    
}