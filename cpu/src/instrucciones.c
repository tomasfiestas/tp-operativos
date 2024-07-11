#include <instrucciones.h>

int hay_interrupcion;
t_pcb* ctx_global;
int tamanio_pagina = 32; // VER BIEN.

void ciclo_de_instruccion(t_pcb* pcbb){  
   	t_instruccion *instruccion_actual;	
    regs = pcbb->registros;
    hay_interrupcion = 0;
    direccion_fisica = 0;

	while (pcbb != NULL /*&& !hayinterruption */) { //Y no haya interrupciones.
		instruccion_actual = fetch(pcbb);		
		if (decode(instruccion_actual)){                        
            /*Implementar TLB y MMU para traducción de direcciones

            número_página = floor(dirección_lógica / tamanio_página)
            desplazamiento = dirección_lógica - número_página * tamanio_página

            //Pedir tamaño de página a memoria. Cuando se conecta como cliente. 
            */
        }       
        execute(instruccion_actual, &pcbb);				//Agregar interrupciones. 
        //hayinterrupcion = 0;
	}    
}


t_instruccion *solicitar_instruccion_a_memoria(int program_counter, int pid)
{   
    //Solicito instruccion
    t_buffer *buffer = crear_buffer();
    t_paquete *paquete = crear_paquete(SOLICITUD_INST,buffer);

    agregar_a_paquete(paquete, &program_counter, sizeof(int));
    agregar_a_paquete(paquete, &pid, sizeof(int));

    enviar_paquete(paquete, conexion_memoria);

    //codigo_operacion cod_op = recibir_operacion(conexion_memoria);
    t_buffer *buffer = crear_buffer();
    op_code cod_op = recibir_operacion(conexion_memoria);
    if(cod_op == SOLICITUD_INST_OK){
        cod_op = recibir_operacion(conexion_memoria);
    }
    else{
       log_error(cpu_logger, "Ocurrio un error al recibir la instruccion");
    }
    buffer->stream = recibir_buffer(conexion_memoria);

    t_instruccion *instruccion = deserializar_instruccion_solicitada(buffer);
    
    liberar_buffer(buffer);

    return instruccion;
}

t_instruccion *fetch(t_pcb *ctx)
{
    log_info(cpu_logger,"PID: %d - FETCH - Program Counter: %d.", ctx->pid, ctx->program_counter);

    t_instruccion *instruccion = solicitar_instruccion_a_memoria(ctx->program_counter, ctx -> pid);
    
    return instruccion;
} //Program counter, quien lo aumenta, memoria o CPU?

bool decode(t_instruccion *instruccion){ //Muestra si requiere cambio de direccion logica a fisica

    op_code operacion = instruccion->operacion;

    return(operacion == MOV_IN || operacion == MOV_OUT || operacion == COPY_STRING   || operacion == IO_STDIN_READ   || operacion == IO_STDOUT_WRITE  || operacion == IO_FS_WRITE  || operacion == IO_FS_READ);
} //Devuelve True si requiere cambio de dirección.


void asignar_valor_a_registro(t_pcb* contexto,char* registro, int valor)
{

    // Asignar el valor al registro correspondiente
    if (strcmp(registro, "AX") == 0) {
        contexto->registros.AX = valor;
    } else if (strcmp(registro, "BX") == 0) {
        contexto->registros.BX = valor;
    } else if (strcmp(registro, "CX") == 0) {
        contexto->registros.CX = valor;
    } else if (strcmp(registro, "DX") == 0) {
        contexto->registros.DX = valor;
    } else if (strcmp(registro, "EAX") == 0) {
        contexto->registros.EAX = valor;
    } else if (strcmp(registro, "EBX") == 0) {
        contexto->registros.EBX = valor;
    } else if (strcmp(registro, "ECX") == 0) {
        contexto->registros.ECX = valor;
    } else if (strcmp(registro, "EDX") == 0) {
        contexto->registros.EDX = valor;
    } else if (strcmp(registro, "SI") == 0) {
        contexto->registros.SI = valor;
    } else if (strcmp(registro, "DI") == 0) {
        contexto->registros.DI = valor;
    } else if (strcmp(registro, "PC") == 0) {
        contexto->registros.PC = valor;
    } else {
        printf("Registro desconocido: %s\n", registro);
    }
}

int obtener_valor_de_registro(t_pcb* contexto, char* registro) {
    if (strcmp(registro, "AX") == 0) {
        return contexto->registros.AX;
    } else if (strcmp(registro, "BX") == 0) {
        return contexto->registros.BX;
    } else if (strcmp(registro, "CX") == 0) {
        return contexto->registros.CX;
    } else if (strcmp(registro, "DX") == 0) {
        return contexto->registros.DX;
    } else if (strcmp(registro, "EAX") == 0) {
        return contexto->registros.EAX;
    } else if (strcmp(registro, "EBX") == 0) {
        return contexto->registros.EBX;
    } else if (strcmp(registro, "ECX") == 0) {
        return contexto->registros.ECX;
    } else if (strcmp(registro, "EDX") == 0) {
        return contexto->registros.EDX;
    } else if (strcmp(registro, "SI") == 0) {
        return contexto->registros.SI;
    } else if (strcmp(registro, "DI") == 0) {
        return contexto->registros.DI;
    } else if (strcmp(registro, "PC") == 0) {
        return contexto->registros.PC;
    } else {
        printf("Registro desconocido: %s\n", registro);
        return -1; // Error: registro desconocido
    }
} //En teoria hacerlo en int no pasa nada, ya que la maquina lo hace en binario y despues lo traduce.

char *obtener_registro(const char *nombre, t_registros *registros)
{
    if(strcmp(nombre, "AX") == 0)
        return &(registros->AX);
    else if(strcmp(nombre,"BX") == 0)
        return &(registros->BX);
    else if(strcmp(nombre,"CX") == 0)
        return &(registros->CX);
    else if(strcmp(nombre,"DX") == 0)
        return &(registros->DX);
    else return NULL;

}

void execute(t_instruccion *instruccion, t_pcb* contexto){ //Ejecuta instrucción 

    switch (instruccion->operacion){
    
    case SET:                                                                    //SET(Registro, Valor)
        char* registro =  (char*) list_get(instruccion->parametros, 0);
        int valor = *((int*) list_get(instruccion->parametros, 1));
        log_info(cpu_logger, "PID : %d - <SET> - <%s %d>", contexto->pid, registro, valor);
        asignar_valor_a_registro(contexto,registro,valor);
        break;
    
    case SUM:                                                                    //SUM(Registro Destino, Registro Origen)
            char* registro_destino = (char*) list_get(instruccion->parametros, 0);
            char* registro_origen = (char*) list_get(instruccion->parametros, 1);
            int valor_origen = obtener_valor_de_registro(contexto, registro_origen);
            int valor_destino = obtener_valor_de_registro(contexto, registro_destino);
            log_info(cpu_logger, "PID: %d - <SUM> - <%s %s>", contexto->pid, registro_destino, registro_origen);
            asignar_valor_a_registro(contexto, registro_destino, valor_destino + valor_origen);
        break;

    case SUB:                                                                    //SUB(Registro Destino, Registro Origen)
             char* registro_destino = (char*) list_get(instruccion->parametros, 0);
            char* registro_origen = (char*) list_get(instruccion->parametros, 1);
            int valor_origen = obtener_valor_de_registro(contexto, registro_origen);
            int valor_destino = obtener_valor_de_registro(contexto, registro_destino);
            log_info(cpu_logger, "PID: %d - <SUB> - <%s %s>", contexto->pid, registro_destino, registro_origen);
            asignar_valor_a_registro(contexto, registro_destino, valor_destino - valor_origen);
        break;

    case JNZ:                                                                     //JNZ(Registro, Instrucción)
            char* registro = (char*) list_get(instruccion->parametros, 0);
            int numero_instruccion = *((int*) list_get(instruccion->parametros, 1));
            int valor_registro = obtener_valor_de_registro(contexto, registro);
            log_info(cpu_logger, "PID: %d - <JNZ> - <%s %d>", contexto->pid, registro, numero_instruccion);
            if (valor_registro != 0) {
                contexto->program_counter = numero_instruccion;
            }
         break;

    //case IO_GEN_SLEEP                                                                     //IO_GEN_SLEEP (Interfaz, Unidades de trabajo)
    //    break;

    default:
        break;
    }
}

void check_interrupt(t_pcb *ctx)
{
    op_code interrupcion_motivo;
    int PID;
    
    recv(cliente_kernel_interrupt, &interrupcion_motivo, sizeof(op_code),MSG_WAITALL);//Recibo el motivo de interrupcion
    recv(cliente_kernel_interrupt, &PID, sizeof(int),MSG_WAITALL);
    
    hay_interrupcion = 1;
    //sem_wait(&recibir_instruccion);
    //sem_wait(&mutex_ctx);
    if(ctx_global != NULL && ctx_global->pid == PID) //Ver bien lo de contexto global.
    {
        t_buffer *buffer = crear_buffer();
        cargar_pcb_a_buffer(buffer,ctx_global);
        t_paquete* paquete = crear_paquete(interrupcion_motivo,buffer);
        enviar_paquete(paquete,cliente_kernel_dispatch);

        ctx_global = NULL;
       
    }
    //sem_post(&mutex_ctx);
}

//MMU
int traducir_direccion_mmu(int dir_logica, t_pcb *ctx)
{
    //CONSULTAR TAMANIO PAGINA A MEMORIA.
    t_list* tlb;

    int desplazamiento = dir_logica - numero_pagina(dir_logica) * tamanio_pagina;

    int resultado_tlb = consultar_tlb(tlb, numero_pagina(dir_logica));
	if (resultado_tlb != -1) return resultado_tlb * tamanio_pagina + desplazamiento;

    
   
    int num_marco = solicitar_numero_de_marco(numero_pagina(dir_logica), ctx->pid);
    if(num_marco == -1)
    {
        return -1;
    }
    
    int dir_fisica = (num_marco * tamanio_pagina) + desplazamiento;
    ctx->program_counter++;

    agregar_entry_tlb(tlb, numero_pagina, num_marco);
    
    return dir_fisica;

}

int numero_pagina(int dir_logica)
{
    //Solicitar TAMAÑO PAGINA.
    return floor(dir_logica/ tamanio_pagina);
}


int solicitar_numero_de_marco(int num_pagina, int pid)
{
    //t_paquete *paquete = crear_paquete(SOLICITUD_MARCO);

    agregar_a_paquete(paquete, &num_pagina, sizeof(int));
    agregar_a_paquete(paquete, &pid, sizeof(int));

    enviar_paquete(paquete,conexion_memoria);

    int num_marco;

    if(recv(conexion_memoria, &num_marco, sizeof(int), MSG_WAITALL) <= 0)
    {
        log_error(cpu_logger, "Ocurrio un error al recibir el numero de marco");
        return -1;
    }

   
    return num_marco;
}

//TLB
int consultar_tlb(t_list* tlb, int pagina) {
	// Devuelve el marco correspondiente a la pagina, si es que se encuentra.

	t_tlb_entry* entry = NULL;

	for (int i = 0; i < list_size(tlb); i++){
		entry = list_get(tlb, i);
		if (entry->pagina == pagina) {

			// Si el algoritmo de remplazo es LRU remover elemento y agregarlo al final para marcarlo como el ultimo utilizado.
			if (strcmp(ALGORITMO_TLB, "LRU") == 0) {
				list_remove(tlb, i);
				list_add(tlb, entry);
			}
			log_info(cpu_logger, "TLB: HIT PAGINA %d | MARCO %d", pagina, entry->marco);

			return entry->marco;
		}
	}
	log_info(cpu_logger, "TLB: MISS de PAGINA %d", pagina);
	return -1;
}

void agregar_entry_tlb(t_cpu* cpu, int pagina, int marco) {
	t_tlb_entry* entry = malloc(sizeof(t_tlb_entry));
	entry->pagina = pagina;
	entry->marco = marco;

	t_list* tlb = cpu->tlb;

	// Checkeamos si agregar un elemento haria que nos pasemos del maximo de entradas permitidas, y en ese caso eliminamos el primero.
	if (list_size(tlb) >= cpu->cpu_config->ENTRADAS_TLB) {
		t_tlb_entry* first_entry = list_remove(tlb, 0);
        log_info(cpu->cpu_log, "TLB: entry de pagina %d agregado (reemplaza a pagina %d)", pagina, first_entry->pagina);
		free(first_entry);
	} else {
        log_info(cpu->cpu_log, "TLB: entry de pagina %d agregado", pagina);
    }

	list_add(tlb, entry);
}

void limpiar_tlb(t_list* tlb) {
	int size = list_size(tlb);

	for (int i = 0; i < size; i++){
		t_tlb_entry* first_entry = list_remove(tlb, 0);
		free(first_entry);
	}
}

void eliminar_marco_tlb(t_cpu* cpu, int marco) {
    t_list* tlb = cpu->tlb;

    for (int i = 0; i < list_size(tlb); i++){
        t_tlb_entry* entry = list_get(tlb, i);
        if (entry->marco == marco) {
            list_remove(tlb, i);
            free(entry);
            break;
        }
    }
}




/*
int execute(t_instruccion *instruccion, t_pcb *ctx)
{   
    int dir_fisica;
    uint32_t *registro;
    switch (instruccion->operacion)
    {
    case SET://SET(Registro,Valor)
        
        log_info(logger_cpu, "PID: %d - Ejecutando: SET - %s %s." , ctx->pid, instruccion->parametros[0], instruccion->parametros[1]);
        asignar_a_registro(instruccion->parametros[0], instruccion->parametros[1], ctx->registros); 
        return NADA;

    case SUM://SUM(Registro Destino,Registro Origen)
    
        log_info(logger_cpu, "PID: %d - Ejecutando: SUM - %s %s." , ctx->pid, instruccion->parametros[0],instruccion->parametros[1]);
        operar_registros(instruccion->operacion,instruccion->parametros[0], instruccion->parametros[1], ctx->registros);
        return NADA;
        
    case SUB:

        log_info(logger_cpu, "PID: %d - Ejecutando: SUB - %s %s." , ctx->pid, instruccion->parametros[0],instruccion->parametros[1]);
        operar_registros(instruccion->operacion,instruccion->parametros[0], instruccion->parametros[1], ctx->registros);
        return NADA;

    case JNZ://JNZ(Registro,Instruccion)

        log_info(logger_cpu, "PID: %d - Ejecutando: JNZ - %s %s." , ctx->pid, instruccion->parametros[0],instruccion->parametros[1]);
        registro = obtener_registro(instruccion->parametros[0], ctx->registros);
        if(*registro != 0 && es_numero_valido(instruccion->parametros[1]))
        {   
            ctx->program_counter = atoi(instruccion->parametros[1]);
        }
        return NADA;//CREO 
    
    case SIGNAL:

        log_info(logger_cpu, "PID: %d - Ejecutando: SIGNAL - %s." , ctx->pid, instruccion->parametros[0]);   
        return SIGNAL;
    
    case IO_GEN_SLEEP:

        log_info(logger_cpu, "PID: %d - Ejecutando: SLEEP - %s." , ctx->pid, instruccion->parametros[0]);
        return IO_GEN_SLEEP;

    case WAIT:

        log_info(logger_cpu, "PID: %d - Ejecutando: WAIT - %s." , ctx->pid, instruccion->parametros[0]);
        return WAIT;

    case EXIT:

        log_info(logger_cpu, "PID: %d - Ejecutando: EXIT." , ctx->pid);
        return EXIT;

    case MOV_IN:// MOV_IN (Registro, Dirección Lógica)

        log_info(logger_cpu, "PID: %d - Ejecutando: MOV_IN - %s %s." , ctx->pid, instruccion->parametros[0],instruccion->parametros[1]);
        dir_fisica = traducir_direccion_mmu(atoi(instruccion->parametros[1]), ctx);
        if(dir_fisica == -1)
        {
            return MOV_IN;
        }
        char *valor_leido = leer_de_memoria(dir_fisica,ctx->pid);
        asignar_a_registro(instruccion->parametros[0], valor_leido, ctx->registros);
        log_info(logger_cpu, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %s", ctx->pid, dir_fisica, valor_leido);
        free(valor_leido);
        return NADA;
    
    case MOV_OUT://MOV_OUT (Dirección Lógica, Registro)
        log_info(logger_cpu, "PID: %d - Ejecutando: MOV_OUT - %s %s." , ctx->pid, instruccion->parametros[0],instruccion->parametros[1]);
        dir_fisica = traducir_direccion_mmu(atoi(instruccion->parametros[0]), ctx);
        if(dir_fisica == -1)
        {   
            log_info(cpu, "HUBO PAGE FAULT");
            return MOV_OUT;
        }
        
        registro =  obtener_registro(instruccion->parametros[1], ctx->registros);
        int num_pagina = numero_pagina(atoi(instruccion->parametros[0]));
        escribir_en_memoria(dir_fisica,ctx->pid,*registro, num_pagina);
        log_info(logger_cpu, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %d", ctx->pid, dir_fisica, *registro);
        return NADA;

}
*/