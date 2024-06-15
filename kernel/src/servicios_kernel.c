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
