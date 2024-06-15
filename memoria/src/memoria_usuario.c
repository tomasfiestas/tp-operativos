#include "memoria_usuario.h"
#include <commons/bitarray.h>

char* bitmap;
t_bitarray* bitarray;
void* memoria_total;

void inicializar_memoria()
{
    memoria_total = reservar_memoria();
}

void inicializar_bitmap() {
    // un bit por pagina.
    int cantidad_bits = atoi(TAM_MEMORIA) / atoi(TAM_PAGINA);

    int cantidad_bytes = cantidad_bits / 8;


    bitmap = malloc(ceil((double)cantidad_bytes / 8));
    bitarray = bitarray_create_with_mode(bitmap, (atoi(TAM_MEMORIA) / atoi(TAM_PAGINA)) / 8, LSB_FIRST);
    for(int i = 0; i < atoi(TAM_MEMORIA) / atoi(TAM_PAGINA); i++) {
        bitarray_clean_bit(bitarray, i);
    }
}

int iniciar_tabla_paginas(t_proceso *proceso)
{
    t_list *paginas = list_create();
    for (int i = 0; i < atoi(TAM_MEMORIA) / atoi(TAM_PAGINA); i++)
    {
        t_pagina *pagina = malloc(sizeof(t_pagina));
        pagina->presente = false;
        list_add(paginas, pagina);
    }
    proceso->paginas = paginas;
    log_info(memoria_logger, "[Creacion de tabla] PID: %d - Tamaño: %d", proceso->pid, list_size(paginas));
    return 1;
}

void finalizar_proceso(int pid)
{
    t_proceso *proceso = obtener_proceso(pid);
    if (proceso == NULL)
    {
        log_error(memoria_logger, "No se encontro el proceso con PID %d", pid);
        return;
    }

    t_list_iterator *iterator = list_iterator_create(proceso->paginas);
    while (list_iterator_has_next(iterator))
    {
        t_pagina *pagina = list_iterator_next(iterator);
        if (pagina->presente)
        {
            bitarray_clean_bit(bitarray, list_iterator_index(iterator));
        }
    }
    log_info(memoria_logger, "[DESTRUCCION DE TABLA] PID: %d - Tamaño: %d", proceso->pid, list_size(proceso->paginas));
    list_destroy_and_destroy_elements(proceso->paginas, free);
    list_destroy_and_destroy_elements(proceso->instrucciones, free);
    list_remove_and_destroy_element(procesos, obtener_indice_proceso(proceso), free);
}

int quitar_memoria(t_proceso *proceso, int cantidad_paginas)
{
    for (int i = atoi(TAM_MEMORIA) / atoi(TAM_PAGINA); i > 0 && cantidad_paginas > 0; i--)
    {
        t_pagina *pagina = list_get(proceso->paginas, i);
        if (pagina->presente)
        {
            bitarray_clean_bit(bitarray, i);
            pagina->presente = false;
            cantidad_paginas--;
        }
    }
}

int resize(int pid, int bytes)
{
    t_proceso *proceso = obtener_proceso(pid);
    if (proceso == NULL)
    {
        log_error(memoria_logger, "No se encontro el proceso con PID %d", pid);
        return 0;
    }

    int paginas_pedidas = ceil((double)bytes / atoi(TAM_PAGINA));
    int paginas_en_uso_proceso = obtener_cantidad_paginas_en_uso(proceso);
    int marcos_disponibles = obtener_cantidad_marcos_disponibles();

    if (paginas_pedidas == paginas_en_uso_proceso)
    {
        return 0; // No hay que hacer nada
    }

    // Ampliacion
    if (paginas_pedidas > paginas_en_uso_proceso)
    {
        // En un caso de resize de ampliacion, hay que chequear que se disponga de marcos.
        if (paginas_pedidas - paginas_en_uso_proceso > marcos_disponibles)
        {
            return -1; // Out of memory
        }

        log_info(memoria_logger, "PID: %d - Tamaño actual: %d - Tamaño a Ampliar: %d", pid, paginas_en_uso_proceso, paginas_pedidas - paginas_en_uso_proceso);
        return asignar_memoria(proceso, paginas_pedidas - paginas_en_uso_proceso);
    }
    else
    {
        // Reduccion
        log_info(memoria_logger, "PID: %d - Tamaño actual: %d - Tamaño a Reducir: %d", pid, paginas_en_uso_proceso, paginas_en_uso_proceso - paginas_pedidas);
        return quitar_memoria(proceso, paginas_en_uso_proceso - paginas_pedidas);
    }
}

char *leer_memoria(int pid, int direccion_fisica)
{
    t_proceso *proceso = obtener_proceso(pid);
    if (proceso == NULL)
    {
        log_error(memoria_logger, "No se encontro el proceso con PID %d", pid);
        return NULL;
    }
    log_info(memoria_logger, "PID: %d - Accion: LEER - Direccion fisica: %d - Tamaño: %zu", pid, direccion_fisica, strlen(&memoria_total[direccion_fisica]));
    return &memoria_total[direccion_fisica];
}

int escribir_memoria(int pid, int direccion_fisica, char *bytes)
{
    memcpy(&memoria_total[direccion_fisica], bytes, strlen(bytes));
    log_info(memoria_logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: %d - Tamaño: %zu", pid, direccion_fisica, strlen(&memoria_total[direccion_fisica]));
    return 1;
}

int asignar_memoria(t_proceso *proceso, int cantidad_paginas)
{
    for (int i = 0; i < atoi(TAM_MEMORIA) / atoi(TAM_PAGINA) && cantidad_paginas > 0; i++)
    {
        if (!bitarray_test_bit(bitarray, i))
        {
            t_pagina *pagina = list_get(proceso->paginas, i);
            pagina->presente = true;
            pagina->frame = i;
            bitarray_set_bit(bitarray, i);
            cantidad_paginas--;
        }
    }

    return 1;
}

/**
 * Obtiene un numero de marco a partir de un numero de pagina.
 * Una pagina solo puede corresponderse a un unico marco.
 *
 * Retorna -1 si la pagina no esta poblada para ningun proceso.
 */
int obtener_numero_marco(int numero_pagina)
{
    t_list_iterator *iterator = list_iterator_create(procesos);
    while (list_iterator_has_next(iterator))
    {
        t_proceso *proceso = list_iterator_next(iterator);
        t_pagina *pagina = list_get(proceso->paginas, numero_pagina);
        if (pagina->presente)
        {
            log_info(memoria_logger, "PID: %d - Pagina: %d - Marco: %d", proceso->pid, numero_pagina, pagina->frame);
            list_iterator_destroy(iterator);
            return pagina->frame;
        }
    }
    list_iterator_destroy(iterator);
    return -1;
}

bool validar_si_existen_marcos(int cantidad_paginas)
{
    int marcos_disponibles = 0;
    for (int i = 0; i < atoi(TAM_MEMORIA) / atoi(TAM_PAGINA); i++)
    {
        if (!bitarray_test_bit(bitarray, i))
        {
            marcos_disponibles++;
        }
    }
    return marcos_disponibles >= cantidad_paginas;
}

void *reservar_memoria()
{
    void *totalMemory = malloc(atoi(TAM_MEMORIA));
    if (totalMemory == NULL)
    {
        log_error(memoria_logger, "No se pudo reservar la memoria necesaria.");
        abort();
    }
    return totalMemory;
}

int obtener_cantidad_marcos_disponibles()
{
    int marcos_disponibles = 0;
    for (int i = 0; i < atoi(TAM_MEMORIA) / atoi(TAM_PAGINA); i++)
    {
        if (!bitarray_test_bit(bitarray, i))
        {
            marcos_disponibles++;
        }
    }
    return marcos_disponibles;
}

int obtener_cantidad_paginas_en_uso(t_proceso *proceso)
{
    int paginas_en_uso = 0;
    t_list_iterator *iterator = list_iterator_create(proceso->paginas);
    while (list_iterator_has_next(iterator))
    {
        t_pagina *pagina = list_iterator_next(iterator);
        if (pagina->presente)
        {
            paginas_en_uso++;
        }
    }
    list_iterator_destroy(iterator);
    return paginas_en_uso;
}

int obtener_indice_proceso(t_proceso* proceso) {
    int index = 0;
    t_list_iterator *iterator = list_iterator_create(procesos);
    while (list_iterator_has_next(iterator))
    {
        t_proceso *proceso_iterado = list_iterator_next(iterator);
        if (proceso_iterado->pid == proceso->pid)
        {
            list_iterator_destroy(iterator);
            return index;
        }
        index++;
    }
    list_iterator_destroy(iterator);
    return -1;
}