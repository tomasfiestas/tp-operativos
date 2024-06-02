#include "memoria.h"
#include "memoria_usuario.h"
#include <commons/bitarray.h>

void inicializar_memoria() {
    memoria_total = reservar_memoria();
    bitarray_create_with_mode(bitmap, atoi(TAM_MEMORIA) / atoi(TAM_PAGINA), LSB_FIRST);
}

void crear_proceso(int pid, t_instrucciones* instrucciones) {
    t_tabla_paginas* tabla_paginacion = iniciar_tabla_paginas(pid);
    alocar_memoria(tabla_paginacion, instrucciones);
}

void* reservar_memoria() {
    void* totalMemory = malloc(atoi(TAM_MEMORIA));
    if(totalMemory == NULL) {
        log_error(logger, "No se pudo reservar la memoria necesaria.");
        abort();
    }
    return totalMemory;
}

t_tabla_paginas* iniciar_tabla_paginas(int pid) {
    t_tabla_paginas* tabla = (t_tabla_paginas*) *(memoria_total + sizeof(t_tabla_paginas) * cantidad_procesos);
    tabla->pid = pid;
    tabla->paginas = (t_pagina*) (cantidad_procesos + sizeof(t_tabla_paginas));
    for(int i = 0; i < atoi(TAM_MEMORIA) / atoi(TAM_PAGINA); i++) {
        tabla->paginas[i].presente = false;
    }
    cantidad_procesos++;
    return tabla;
}

void alocar_memoria(t_tabla_paginas* tabla, t_instrucciones* instrucciones) {

    // TODO. Ni idea como se hace esto. La idea es tener una tabla por proceso???
    // los frames seran las particiones de la memoria reservada anteriormente.

}
