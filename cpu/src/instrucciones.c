#include <instrucciones.h>


t_pcb* ctx_global;
int tamanio_pagina; // VER BIEN.
int socket_memoria;
int socket_kernel;
int direccion_fisica;


t_log* cpu_logger;
t_config* cpu_config;
t_list* tlb;



void ciclo_de_instruccion(t_pcb* pcbb){  
   	t_instruccion instruccion_actual;	
    ctx_global = pcbb;
    //regs = pcbb->registros;
    hay_interrupcion = 0;
    direccion_fisica = 0;

	while (ctx_global != NULL && !hay_interrupcion ) { //Y no haya interrupciones.
		instruccion_actual = fetch(pcbb);		
		if (decode(instruccion_actual)){                        
            /*Implementar TLB y MMU para traducción de direcciones

            número_página = floor(dirección_lógica / tamanio_página)
            desplazamiento = dirección_lógica - número_página * tamanio_página

            //Pedir tamaño de página a memoria. Cuando se conecta como cliente. 
            */
        }       
        execute(instruccion_actual, pcbb);				//Agregar interrupciones.
        if(hay_interrupcion){
            //enviar_pcb_a_kernel
            t_buffer* buffer_cpu_ki = crear_buffer();    
            cargar_pcb_a_buffer(buffer_cpu_ki,pcbb);           
            log_info(cpu_logger, "Enviamos PCB de proceso FINALIZADO - PID %d ", pcbb->pid);   
            t_paquete* paquete_cpu = crear_paquete(INTERRUPTED_BY_USER, buffer_cpu_ki);
            enviar_paquete(paquete_cpu, cliente_kernel_dispatch);    
            free(pcbb);            
            destruir_paquete(paquete_cpu);    
            ctx_global = NULL;

            log_info(cpu_logger, "Valores de los registros del PCB:");

            log_info(cpu_logger, "Registro AX: %d | BX:%d | CX: %d", pcbb->registros.AX,pcbb->registros.BX,pcbb->registros.CX);
            log_info(cpu_logger, "Registro DX: %d | EAX:%d | EBX: %d", pcbb->registros.DX,pcbb->registros.EAX,pcbb->registros.EBX);
            log_info(cpu_logger, "Registro ECX: %d | EDX:%d | SI: %d", pcbb->registros.ECX,pcbb->registros.EDX,pcbb->registros.SI);
            log_info(cpu_logger, "Registro DI: %d", pcbb->registros.DI);
            

        
        //hayinterrupcion = 0;
	}    
}
}


t_instruccion solicitar_instruccion_a_memoria(t_pcb* t_pcb)
{   
    //Solicito instruccion
    t_buffer *buffer = crear_buffer();
    cargar_int_a_buffer(buffer, t_pcb->pid); //Cargo el pid del proceso al que pertenece la instruccion a solicitar
    cargar_uint32_a_buffer(buffer, t_pcb->registros.PC); //Cargo el program counter de la instruccion a solicitar
    
    t_paquete *paquete = crear_paquete(SOLICITUD_INST,buffer);

    //agregar_a_paquete(paquete, &program_counter, sizeof(int));
    //agregar_a_paquete(paquete, &pid, sizeof(int));

    enviar_paquete(paquete, conexion_memoria);
    destruir_paquete(paquete);

    //codigo_operacion cod_op = recibir_operacion(conexion_memoria);    
    op_code cod_op = recibir_operacion(conexion_memoria);
    t_buffer *buffer_recibido = recibir_buffer(conexion_memoria);
    if(cod_op == SOLICITUD_INST_OK){
        t_instruccion_a_enviar instruccion = extraer_instruccion_a_enviar_del_buffer(buffer_recibido);
        t_instruccion nueva_instruccion = crear_instruccion_nuevamente(instruccion, buffer_recibido);
        if(nueva_instruccion.operacion!= EXIT){ 
        t_pcb->registros.PC = extraer_uint32_del_buffer(buffer_recibido);
        destruir_buffer(buffer_recibido);
        }
        
        return nueva_instruccion;
        
    }
    else{
       log_error(cpu_logger, "Ocurrio un error al recibir la instruccion");
       abort();
    }
       

    
}
t_instruccion crear_instruccion_nuevamente(t_instruccion_a_enviar instruccion_a_enviar, t_buffer *buffer_recibido){
    t_instruccion instruccion ;
    instruccion.operacion = instruccion_a_enviar.operacion;
    instruccion.parametros = list_create();

    switch(instruccion.operacion){ 
        case SET: //TIenen todas 2 parametros
        case SUM:
        case SUB:
        case MOV_IN:
        case MOV_OUT:
        case JNZ:
        case IO_GEN_SLEEP:
        case IO_FS_CREATE:
        case IO_FS_DELETE:
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            break;
        case RESIZE: 
        case COPY_STRING:   //Tienen 1 parametro
        case WAIT:
        case SIGNAL:
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));            
            break;        
        case IO_STDIN_READ:
        case IO_STDOUT_WRITE:      //Tienen 3 parametros
        case IO_FS_TRUNCATE:     
        
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            break;
        case IO_FS_WRITE: //Tienen 4 parametros
        case IO_FS_READ:
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            list_add(instruccion.parametros, extraer_string_del_buffer(buffer_recibido));
            break;
        case EXIT:
            break; 


    }

    return instruccion;

}

t_instruccion fetch(t_pcb *ctx)
{
    log_info(cpu_logger,"PID: %d - FETCH - Program Counter: %u.", ctx->pid, ctx->registros.PC);

    t_instruccion instruccion = solicitar_instruccion_a_memoria(ctx);
    
    return instruccion;
} //Program counter, quien lo aumenta, memoria o CPU?

bool decode(t_instruccion instruccion){ //Muestra si requiere cambio de direccion logica a fisica

    op_code operacion = instruccion.operacion;

    return(operacion == MOV_IN || operacion == MOV_OUT || operacion == COPY_STRING   || operacion == IO_STDIN_READ   || operacion == IO_STDOUT_WRITE  || operacion == IO_FS_WRITE  || operacion == IO_FS_READ);
} //Devuelve True si requiere cambio de dirección.


void asignar_valor_a_registro(t_pcb* contexto,char* registro, int valor)
{
    uint8_t valor_8 = (uint8_t)valor; 
    uint32_t valor_32 = (uint32_t)valor; 
    // Asignar el valor al registro correspondiente
    if (strcmp(registro, "AX") == 0) {
        contexto->registros.AX = valor_8;
    } else if (strcmp(registro, "BX") == 0) {
        contexto->registros.BX = valor_8;
    } else if (strcmp(registro, "CX") == 0) {
        contexto->registros.CX = valor_8;
    } else if (strcmp(registro, "DX") == 0) {
        contexto->registros.DX = valor_8;
    } else if (strcmp(registro, "EAX") == 0) {
        contexto->registros.EAX = valor_32;
    } else if (strcmp(registro, "EBX") == 0) {
        contexto->registros.EBX = valor_32;
    } else if (strcmp(registro, "ECX") == 0) {
        contexto->registros.ECX = valor_32;
    } else if (strcmp(registro, "EDX") == 0) {
        contexto->registros.EDX = valor_32;
    } else if (strcmp(registro, "SI") == 0) {
        contexto->registros.SI = valor_32;
    } else if (strcmp(registro, "DI") == 0) {
        contexto->registros.DI = valor_32;
    } else if (strcmp(registro, "PC") == 0) {
        contexto->registros.PC = valor_32;
    } else {
        printf("Registro desconocido: %s\n", registro);
    }
}

void asignar_valor_char_a_registro(t_pcb* contexto,char* registro, uint32_t valor_32)
{       
    //uint32_t valor_32 = atoi(valor); //Convierto el valor a entero
    uint8_t valor_8 = (uint8_t)valor_32; // Convierto el valor a 8 bits
    // Asignar el valor al registro correspondiente
    if (strcmp(registro, "AX") == 0) {
        contexto->registros.AX = valor_8;
    } else if (strcmp(registro, "BX") == 0) {
        contexto->registros.BX = valor_8;
    } else if (strcmp(registro, "CX") == 0) {
        contexto->registros.CX = valor_8;
    } else if (strcmp(registro, "DX") == 0) {
        contexto->registros.DX = valor_8;
    } else if (strcmp(registro, "EAX") == 0) {
        contexto->registros.EAX = valor_32;
    } else if (strcmp(registro, "EBX") == 0) {
        contexto->registros.EBX = valor_32;
    } else if (strcmp(registro, "ECX") == 0) {
        contexto->registros.ECX = valor_32;
    } else if (strcmp(registro, "EDX") == 0) {
        contexto->registros.EDX = valor_32;
    } else if (strcmp(registro, "SI") == 0) {
        contexto->registros.SI = valor_32;
    } else if (strcmp(registro, "DI") == 0) {
        contexto->registros.DI = valor_32;
    } else if (strcmp(registro, "PC") == 0) {
        contexto->registros.PC = valor_32;
    } else {
        printf("Registro desconocido: %s\n", registro);
    }   
}

void* obtener_puntero_al_registro(t_pcb* contexto, char* registro) {
    if (strcmp(registro, "AX") == 0) {
        return &(contexto->registros.AX);
    } else if (strcmp(registro, "BX") == 0) {
        return &(contexto->registros.BX);
    } else if (strcmp(registro, "CX") == 0) {
        return &(contexto->registros.CX);
    } else if (strcmp(registro, "DX") == 0) {
        return &(contexto->registros.DX);
    } else if (strcmp(registro, "EAX") == 0) {
        return &(contexto->registros.EAX);
    } else if (strcmp(registro, "EBX") == 0) {
        return &(contexto->registros.EBX);
    } else if (strcmp(registro, "ECX") == 0) {
        return &(contexto->registros.ECX);
    } else if (strcmp(registro, "EDX") == 0) {
        return &(contexto->registros.EDX);
    } else if (strcmp(registro, "SI") == 0) {
        return &(contexto->registros.SI);
    } else if (strcmp(registro, "DI") == 0) {
        return &(contexto->registros.DI);
    } else if (strcmp(registro, "PC") == 0) {
        return &(contexto->registros.PC);
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

void execute(t_instruccion instruccion, t_pcb* contexto){ //Ejecuta instrucción 

    switch (instruccion.operacion){
    
    case SET:                                                            //SET(Registro, Valor)
        char* registro_set =  (char*) list_get(instruccion.parametros, 0);
        char* valor_inst = list_get(instruccion.parametros, 1);
        log_info(cpu_logger, "PID : %d - Ejecutando: <SET> - <%s %s>", contexto->pid, registro_set, valor_inst);
        int valor = atoi(valor_inst);
        asignar_valor_a_registro(contexto,registro_set,valor);
        list_destroy_and_destroy_elements(instruccion.parametros, free);
        break;
    
    case SUM:                                                                    //SUM(Registro Destino, Registro Origen)
            char* registro_destino_sum = (char*) list_get(instruccion.parametros, 0);
            char* registro_origen_sum = (char*) list_get(instruccion.parametros, 1);
            int valor_origen_sum = *(int*)obtener_puntero_al_registro(contexto, registro_origen_sum);
            int valor_destino_sum = *(int*)obtener_puntero_al_registro(contexto, registro_destino_sum);
            log_info(cpu_logger, "PID: %d - Ejecutando: <SUM> - <%s %s>", contexto->pid, registro_destino_sum, registro_origen_sum);
            asignar_valor_a_registro(contexto, registro_destino_sum, valor_destino_sum + valor_origen_sum);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
        break;

    case SUB:                                                                    //SUB(Registro Destino, Registro Origen)
            char* registro_destino_sub = (char*) list_get(instruccion.parametros, 0);
            char* registro_origen_sub = (char*) list_get(instruccion.parametros, 1);
            int valor_origen_sub = *(int*)obtener_puntero_al_registro(contexto, registro_origen_sub);
            int valor_destino_sub = *(int*)obtener_puntero_al_registro(contexto, registro_destino_sub);
            log_info(cpu_logger, "PID: %d - Ejecutando: <SUB> - <%s %s>", contexto->pid, registro_destino_sub, registro_origen_sub);
            asignar_valor_a_registro(contexto, registro_destino_sub, valor_destino_sub - valor_origen_sub);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
        break;

    case JNZ:                                                                     //JNZ(Registro, Instrucción)
            char* registro_jnz = (char*) list_get(instruccion.parametros, 0);
            int numero_instruccion = *((int*) list_get(instruccion.parametros, 1));
            int valor_registro = *(int*)obtener_puntero_al_registro(contexto, registro_jnz);
            log_info(cpu_logger, "PID: %d - Ejecutando: <JNZ> - <%s %d>", contexto->pid, registro_jnz, numero_instruccion);
            if (valor_registro != 0) {
                contexto->registros.PC = numero_instruccion;
            }
            list_destroy_and_destroy_elements(instruccion.parametros, free);
         break;    
    case MOV_IN:  // MOV_IN (Registro Datos, Registro Dirección) Direccion guardo en Datos
            const char* registro_datos_mov_in = (const char*) list_get(instruccion.parametros, 0);
            const char* registro_direccion_mov_in = (const char*) list_get(instruccion.parametros, 1);
            
            int dir_logica = *(int*)obtener_puntero_al_registro(contexto, registro_direccion_mov_in);
            int bytes_a_leer = calcular_bytes_a_leer(registro_direccion_mov_in);
            t_list* direccion_fisica = traducir_direccion_mmu(dir_logica, contexto,bytes_a_leer);

            if (direccion_fisica == -1) {
                log_error(cpu_logger, "PID: %d - Error al traducir dirección lógica: %d", contexto->pid, dir_logica);
                return;
            }
            
            
            if(bytes_a_leer == 1){ 
                uint8_t valor_leido = *(uint8_t*)leer_de_memoria(direccion_fisica,bytes_a_leer, contexto->pid);
                asignar_valor_a_registro(contexto,registro_datos_mov_in, valor_leido);   //TODO REVISAR
                log_info(cpu_logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d", contexto->pid, direccion_fisica, valor_leido);
                if (valor_leido == NULL) {
                    log_error(cpu_logger, "PID: %d - Error al leer memoria en la dirección física: %d", contexto->pid, direccion_fisica);
                    return;
                }  
                asignar_valor_a_registro(contexto,registro_datos_mov_in, valor_leido);   //TODO REVISAR
                log_info(cpu_logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d", contexto->pid, direccion_fisica, valor_leido);
            }
            else if(bytes_a_leer == 4){             
            uint32_t valor_leido = *(uint32_t*)leer_de_memoria(direccion_fisica,bytes_a_leer, contexto->pid);
            asignar_valor_a_registro(contexto,registro_datos_mov_in, valor_leido);   //TODO REVISAR
            log_info(cpu_logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d", contexto->pid, direccion_fisica, valor_leido);
            if (valor_leido == NULL) {
                log_error(cpu_logger, "PID: %d - Error al leer memoria en la dirección física: %d", contexto->pid, direccion_fisica);
                return;
            }  
            }         
            
            list_destroy_and_destroy_elements(instruccion.parametros, free);

            //free(valor_leido);
            break;
    case MOV_OUT:                                                                            //MOV_OUT (Registro Direccion, Registro Datos)
            char* registro_direccion = (char*) list_get(instruccion.parametros, 0);
            char* registro_datos = (char*) list_get(instruccion.parametros, 1);

            int bytes_a_escribir = calcular_bytes_a_leer(registro_datos);

            int direccion_logica = *(int*) obtener_puntero_al_registro(contexto, registro_direccion);
            void* valor_out = obtener_puntero_al_registro(contexto, registro_datos);
            
            log_info(cpu_logger, "PID: %d - Ejecutando: MOV_OUT - %s %s", contexto->pid, registro_direccion, registro_datos);
            t_list* lista_df = traducir_direccion_mmu(direccion_logica,contexto, bytes_a_escribir);    
            if (list_size(lista_df) == 0) {
                log_error(cpu_logger, "Error al traducir la dirección lógica: %d", direccion_logica);
                break;
            }
            
            escribir_a_memoria(lista_df,bytes_a_escribir, contexto, valor_out);
            
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            break;

    case RESIZE:
            int cantidad_bytes = atoi(list_get(instruccion.parametros, 0));
            log_info(cpu_logger, "PID: %d - Ejecutando: <RESIZE> - %d", contexto->pid, cantidad_bytes);
            t_buffer* buffer_kernel_resize = crear_buffer();         
            cargar_int_a_buffer(buffer_kernel_resize, contexto->pid);   
            cargar_int_a_buffer(buffer_kernel_resize, cantidad_bytes);
            t_paquete* paquete_resize = crear_paquete(RESIZE, buffer_kernel_resize);
            enviar_paquete(paquete_resize, conexion_memoria);
            destruir_paquete(paquete_resize);

            op_code cod_op_resize = recibir_operacion(conexion_memoria);
            if (cod_op_resize == OUT_OF_MEMORY) {
                log_error(cpu_logger, "Ocurrió un error al hacer RESIZE"); //Mando pcb a kernel
                t_buffer* buffer_kernel_out_of_memory = crear_buffer();
                cargar_pcb_a_buffer(buffer_kernel_out_of_memory, contexto);
                t_paquete* paquete_out_of_memory = crear_paquete(OUT_OF_MEMORY, buffer_kernel_out_of_memory);
                enviar_paquete(paquete_out_of_memory, cliente_kernel_dispatch);
                destruir_paquete(paquete_out_of_memory);
                break;
            }
            else{ //RESIZE_OK
                t_buffer* buffer_rta_resize = recibir_buffer(conexion_memoria);
                int numero = extraer_int_del_buffer(buffer_rta_resize);
                destruir_buffer(buffer_rta_resize);
                
                } 
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            break;
    case WAIT: 
            char* recurso_wait = list_get(instruccion.parametros, 0);
            log_info(cpu_logger, "PID: %d - Ejecutando: WAIT - %s", contexto->pid, recurso_wait);
            t_buffer* buffer_kernel_wait = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel_wait, contexto);
            cargar_string_a_buffer(buffer_kernel_wait, recurso_wait);
            t_paquete* paquete_wait = crear_paquete(WAIT, buffer_kernel_wait);
            enviar_paquete(paquete_wait, cliente_kernel_dispatch);
            destruir_paquete(paquete_wait);
            ctx_global = NULL;
            break;
    case SIGNAL: 
            char* recurso_signal = list_get(instruccion.parametros, 0);
            log_info(cpu_logger, "PID: %d - Ejecutando: SIGNAL - %s", contexto->pid, recurso_signal);
            t_buffer* buffer_kernel_signal = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel_signal, contexto);
            cargar_string_a_buffer(buffer_kernel_signal, recurso_signal);
            t_paquete* paquete_signal = crear_paquete(SIGNAL, buffer_kernel_signal);
            enviar_paquete(paquete_signal, cliente_kernel_dispatch);
            destruir_paquete(paquete_signal);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;
    case IO_GEN_SLEEP:                                                                     //IO_GEN_SLEEP (Interfaz, Unidades de trabajo)
            char* interfaz = (char*) list_get(instruccion.parametros, 0);
            int unidades_trabajo = *((int*) list_get(instruccion.parametros, 1));
            log_info(cpu_logger, "PID: %d - Ejecutando:<IO_GEN_SLEEP> - <%s %d>", contexto->pid, interfaz, unidades_trabajo); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel, contexto);
            cargar_string_a_buffer(buffer_kernel, interfaz);   // HAY QUE CASTEAR EL INT A CHAR*?
            cargar_int_a_buffer(buffer_kernel, unidades_trabajo);
            t_paquete* paquete = crear_paquete(IO_GEN_SLEEP, buffer_kernel);
            enviar_paquete(paquete, cliente_kernel_dispatch);
            destruir_paquete(paquete);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;
    /*case IO_STDIN_READ:  //IO_STDIN_READ (Interfaz, Registro Direccion, Registro tamaño)
            char* interfaz1 = (char*) list_get(instruccion.parametros, 0);
            char* registroDireccion1 = (char*) list_get(instruccion.parametros, 1);
            char* registroTamanio1 = (char*) list_get(instruccion.parametros, 2);

            int direccion_logica_stdin = obtener_puntero_al_registro(contexto, registroDireccion1);   

            int direccion_fisica_stdin = traducir_direccion_mmu(direccion_logica_stdin, contexto);
            log_info(cpu_logger, "PID: %d - Ejecutando: <IO_STDIN_READ> - <%s %d>", contexto->pid, interfaz1, registroDireccion1, registroTamanio1); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel1 = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel1, contexto);
            cargar_string_a_buffer(buffer_kernel1, interfaz1);   
            cargar_int_a_buffer(buffer_kernel1, direccion_fisica_stdin);
            cargar_string_a_buffer(buffer_kernel1, registroTamanio1);
            t_paquete* paquete1 = crear_paquete(IO_STDIN_READ, buffer_kernel1);
            enviar_paquete(paquete1, cliente_kernel_dispatch);
            destruir_paquete(paquete1);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;*/
    /*case IO_STDOUT_WRITE:  //IO_STDOUT_WRITE (Interfaz, Registro Direccion, Registro tamaño)
            char* interfaz2 = (char*) list_get(instruccion.parametros, 0);
            char* registroDireccion2 = (char*) list_get(instruccion.parametros, 1);
            char* registroTamanio2 = (char*) list_get(instruccion.parametros, 2);

            int direccion_logica_stdout = obtener_puntero_al_registro(contexto, registroDireccion2);   

            int direccion_fisica_stdout = traducir_direccion_mmu(direccion_logica_stdout, contexto);

            log_info(cpu_logger, "PID: %d - Ejecutando: <IO_STDOUT_WRITE> - <%s %d>", contexto->pid, interfaz2, registroDireccion2, registroTamanio2); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel2 = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel2, contexto);
            cargar_string_a_buffer(buffer_kernel2, interfaz2);   
            cargar_int_a_buffer(buffer_kernel2, direccion_fisica_stdout);
            cargar_string_a_buffer(buffer_kernel2, registroTamanio2);
            t_paquete* paquete2 = crear_paquete(IO_STDOUT_WRITE, buffer_kernel2);
            enviar_paquete(paquete2, cliente_kernel_dispatch);
            destruir_paquete(paquete2);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;*/
    case IO_FS_CREATE:  //IO_FS_CREATE (Interfaz, Nombre Archivo)
            char* interfaz3 = (char*) list_get(instruccion.parametros, 0);
            char* nombreArchivo3 = (char*) list_get(instruccion.parametros, 1);
            log_info(cpu_logger, "PID: %d - Ejecutando: <IO_FS_CREATE> - <%s %d>", contexto->pid, interfaz3, nombreArchivo3); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel3 = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel3, contexto);
            cargar_string_a_buffer(buffer_kernel3, interfaz3);   
            cargar_string_a_buffer(buffer_kernel3, nombreArchivo3);
            t_paquete* paquete3 = crear_paquete(IO_FS_CREATE, buffer_kernel3);
            enviar_paquete(paquete3, cliente_kernel_dispatch);
            destruir_paquete(paquete3);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;
    case IO_FS_DELETE:  //IO_FS_DELETE (Interfaz, Nombre Archivo)
            char* interfaz4 = (char*) list_get(instruccion.parametros, 0);
            char* nombreArchivo4 = (char*) list_get(instruccion.parametros, 1);
            log_info(cpu_logger, "PID: %d -Ejecutando: <IO_FS_DELETE> - <%s %d>", contexto->pid, interfaz4, nombreArchivo4); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel4 = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel4, contexto);
            cargar_string_a_buffer(buffer_kernel4, interfaz4);   
            cargar_string_a_buffer(buffer_kernel4, nombreArchivo4);
            t_paquete* paquete4 = crear_paquete(IO_FS_DELETE, buffer_kernel4);
            enviar_paquete(paquete4, cliente_kernel_dispatch);
            destruir_paquete(paquete4);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;
    case IO_FS_TRUNCATE:  //IO_FS_TRUNCATE (Interfaz, Nombre Archivo, Registro Tamaño)
            char* interfaz5 = (char*) list_get(instruccion.parametros, 0);
            char* nombreArchivo5 = (char*) list_get(instruccion.parametros, 1);
            char* registroTamanio5 = (char*) list_get(instruccion.parametros, 2);
            log_info(cpu_logger, "PID: %d -Ejecutando: <IO_FS_TRUNCATE> - <%s %d>", contexto->pid, interfaz5, nombreArchivo5, registroTamanio5); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel5 = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel5, contexto);
            cargar_string_a_buffer(buffer_kernel5, interfaz5);   
            cargar_string_a_buffer(buffer_kernel5, nombreArchivo5);
            cargar_string_a_buffer(buffer_kernel5, registroTamanio5);
            t_paquete* paquete5 = crear_paquete(IO_FS_TRUNCATE, buffer_kernel5);
            enviar_paquete(paquete5, cliente_kernel_dispatch);
            destruir_paquete(paquete5);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;
    /*case IO_FS_WRITE:  //IO_FS_WRITE (Interfaz, Nombre Archivo, Registro Direccion, Registro Tamaño, Registro Puntero Archivo)
            char* interfaz6 = (char*) list_get(instruccion.parametros, 0);
            char* nombreArchivo6 = (char*) list_get(instruccion.parametros, 1);
            char* registroDireccion6 = (char*) list_get(instruccion.parametros, 2);
            char* registroTamanio6 = (char*) list_get(instruccion.parametros, 3);
            char* registroPunteroArchivo6 = (char*) list_get(instruccion.parametros, 4);

            int direccion_logica_fs_write = obtener_puntero_al_registro(contexto, registroDireccion2);   

            int direccion_write_fs_write = traducir_direccion_mmu(direccion_logica_fs_write, contexto);

            log_info(cpu_logger, "PID: %d -Ejecutando: <IO_FS_WRITE> - <%s %d>", contexto->pid, interfaz6, nombreArchivo6, registroDireccion6, registroTamanio6, registroPunteroArchivo6); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel6 = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel6, contexto);
            cargar_string_a_buffer(buffer_kernel6, interfaz6);   
            cargar_string_a_buffer(buffer_kernel6, nombreArchivo6);
            cargar_int_a_buffer(buffer_kernel6, direccion_write_fs_write);
            cargar_string_a_buffer(buffer_kernel6, registroTamanio6);
            cargar_string_a_buffer(buffer_kernel6, registroPunteroArchivo6);
            t_paquete* paquete6 = crear_paquete(IO_FS_WRITE, buffer_kernel6);
            enviar_paquete(paquete6, cliente_kernel_dispatch);
            destruir_paquete(paquete6);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;*/
    /*case IO_FS_READ:  //IO_FS_READ (Interfaz, Nombre Archivo, Registro Direccion, Registro Tamaño, Registro Puntero Archivo)
            char* interfaz7 = (char*) list_get(instruccion.parametros, 0);
            char* nombreArchivo7 = (char*) list_get(instruccion.parametros, 1);
            char* registroDireccion7 = (char*) list_get(instruccion.parametros, 2);
            char* registroTamanio7 = (char*) list_get(instruccion.parametros, 3);
            char* registroPunteroArchivo7 = (char*) list_get(instruccion.parametros, 4);

            
            int direccion_logica_fs_read = obtener_puntero_al_registro(contexto, registroDireccion2);   

            int direccion_write_fs_read = traducir_direccion_mmu(direccion_logica_fs_read, contexto);

            log_info(cpu_logger, "PID: %d -Ejecutando: <IO_FS_READ> - <%s %d>", contexto->pid, interfaz7, nombreArchivo7, registroDireccion7, registroTamanio7, registroPunteroArchivo7); //TODO MANDAR A KERNEL.
            t_buffer* buffer_kernel7 = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel7, contexto);
            cargar_string_a_buffer(buffer_kernel7, interfaz7);   
            cargar_string_a_buffer(buffer_kernel7, nombreArchivo7);
            cargar_int_a_buffer(buffer_kernel7, direccion_write_fs_read);
            cargar_string_a_buffer(buffer_kernel7, registroTamanio7);
            cargar_string_a_buffer(buffer_kernel7, registroPunteroArchivo7);
            t_paquete* paquete7 = crear_paquete(IO_FS_READ, buffer_kernel7);
            enviar_paquete(paquete7, cliente_kernel_dispatch);
            destruir_paquete(paquete7);
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            ctx_global = NULL;
            break;*/
    case COPY_STRING:
            uint32_t direccion_logica_destino = contexto->registros.DI;
            uint32_t direccion_logica_origen = contexto->registros.SI;
            int size = atoi((char*) list_get(instruccion.parametros, 0));

            t_list*  lista_direccion_fisica_origen = traducir_direccion_mmu(direccion_logica_origen, contexto,size);  

            void* valor_leido_de_memoria = leer_de_memoria(lista_direccion_fisica_origen, size, contexto->pid);
                    
            
            t_list* lista_direccion_fisica_destino_copy = traducir_direccion_mmu(direccion_logica_destino, contexto,size);
            
            escribir_a_memoria(lista_direccion_fisica_destino_copy, size, contexto, valor_leido_de_memoria);            
            log_info(cpu_logger, "PID: %d -Ejecutando: <COPY_STRING> - <%d %d>", contexto->pid, direccion_logica_destino, direccion_logica_origen);            
            list_destroy_and_destroy_elements(instruccion.parametros, free);
            list_destroy_and_destroy_elements(lista_direccion_fisica_origen,free);
            break;
    
    case EXIT: 
            log_info(cpu_logger, "PID: %d -Ejecutando: <EXIT>", contexto->pid);
            t_buffer* buffer_kernel_exit = crear_buffer();
            cargar_pcb_a_buffer(buffer_kernel_exit, contexto);
            t_paquete* paquete_exit = crear_paquete(EXIT, buffer_kernel_exit);
            enviar_paquete(paquete_exit, cliente_kernel_dispatch);
            destruir_paquete(paquete_exit);
            ctx_global = NULL;
            break;

    


    default:
        break;
    }
}
void* leer_de_memoria(t_list* lista_df,int bytes_a_leer, int pid)
{   
    void* contenido_leido = malloc(bytes_a_leer);
    int bytes_leidos = 0;

    for(int i=0;i < list_size(lista_df); i++) { 
    t_direccion_fisica_io* dir_fisica = list_get(lista_df,i);

    t_buffer *buffer_envio = crear_buffer();
    cargar_int_a_buffer(buffer_envio, pid);     //PID
    cargar_int_a_buffer(buffer_envio, dir_fisica->df);  //DIR_FISICA
    cargar_int_a_buffer(buffer_envio, dir_fisica->size);    //TAMANIO A LEER

    t_paquete *paquete = crear_paquete(LEER, buffer_envio); 
    enviar_paquete(paquete, conexion_memoria);

    
    destruir_paquete(paquete);

    op_code cod_op = recibir_operacion(conexion_memoria);
    if (cod_op != LEER_OK) {
        log_error(cpu_logger, "Ocurrió un error al hacer MOV_IN");
        return NULL;
    }

    t_buffer *buffer_respuesta = recibir_buffer(conexion_memoria);
    if (buffer_respuesta == NULL) {
        log_error(cpu_logger, "Error al recibir el buffer de respuesta");
        return NULL;
    }

    void* valor_leido = extraer_de_buffer(buffer_respuesta); //extraigo lo leido de memoria
    destruir_buffer(buffer_respuesta);
    memcpy(contenido_leido + bytes_leidos,valor_leido,dir_fisica->size);

    bytes_leidos += dir_fisica->size;
    }
    return contenido_leido;
}

/*void* leer_de_memoria(t_list* lista_df,int bytes_a_leer, int pid)
{   
    char* contenido_leido = string_new();
    int tamanio = 0;

    for(int i=0;i < list_size(lista_df); i++) { 
    t_direccion_fisica_io* dir_fisica = list_get(lista_df,i);

    t_buffer *buffer_envio = crear_buffer();
    cargar_int_a_buffer(buffer_envio, pid);     //PID
    cargar_int_a_buffer(buffer_envio, dir_fisica->df);  //DIR_FISICA
    cargar_int_a_buffer(buffer_envio, dir_fisica->size);    //TAMANIO A LEER

    t_paquete *paquete = crear_paquete(LEER, buffer_envio); 
    enviar_paquete(paquete, conexion_memoria);

    
    destruir_paquete(paquete);

    op_code cod_op = recibir_operacion(conexion_memoria);
    if (cod_op != LEER_OK) {
        log_error(cpu_logger, "Ocurrió un error al hacer MOV_IN");
        return NULL;
    }

    t_buffer *buffer_respuesta = recibir_buffer(conexion_memoria);
    if (buffer_respuesta == NULL) {
        log_error(cpu_logger, "Error al recibir el buffer de respuesta");
        return NULL;
    }

    char* valor_leido = (char*)extraer_de_buffer(buffer_respuesta); //extraigo lo leido de memoria
    destruir_buffer(buffer_respuesta); 
    memcpy(contenido_leido,valor_leido,dir_fisica->size);

    
    }
    return contenido_leido;
}*/



int calcular_bytes_a_leer(char* registro){
    
    if (strcmp(registro, "AX") == 0 ||
        strcmp(registro, "BX") == 0 ||
        strcmp(registro, "CX") == 0 ||
        strcmp(registro, "DX") == 0) {
        return 1;
    } else if (strcmp(registro, "EAX") == 0 ||
               strcmp(registro, "EBX") == 0 ||
               strcmp(registro, "ECX") == 0 ||
               strcmp(registro, "EDX") == 0 ||
               strcmp(registro, "SI") == 0 ||
               strcmp(registro, "DI") == 0 ||
               strcmp(registro, "PC") == 0) {
        return 4;
    } else {
        return -1;
    }
}


/*void escribir_a_memoria(t_list* lista_paginas, int size,t_pcb* pcb, int valor) {
    int desplazamiento = 0;
    char* valor_a_enviar;
    sprintf(valor_a_enviar,"%d",valor);
    for(int i = 0;i < list_size(lista_paginas); i++){ 

    t_direccion_fisica_io* direccion_fisica = list_get(lista_paginas,i);

    void * valor_por_direccion = malloc(direccion_fisica->size);
    memcpy(valor_por_direccion, valor_a_enviar + desplazamiento, direccion_fisica->size);

    t_buffer *buffer_envio = crear_buffer();
    cargar_int_a_buffer(buffer_envio, pcb->pid);    
    cargar_int_a_buffer(buffer_envio, direccion_fisica->df);
    cargar_int_a_buffer(buffer_envio, size);
    //char* envio = malloc(sizeof(char)*20);
    //sprintf(envio, "%d", valor);
    //cargar_string_a_buffer(buffer_envio, envio);
    //free(envio);
    cargar_a_buffer(buffer_envio,valor_por_direccion,direccion_fisica->size);

    t_paquete *paquete = crear_paquete(ESCRIBIR, buffer_envio); 
    enviar_paquete(paquete, conexion_memoria);

    destruir_paquete(paquete); 

    log_info(cpu_logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pcb->pid, direccion_fisica->df, valor_por_direccion);

    op_code cod_op = recibir_operacion(conexion_memoria);
    t_buffer *buffer_respuesta = recibir_buffer(conexion_memoria);
    int basura = extraer_int_del_buffer(buffer_respuesta);
    destruir_buffer(buffer_respuesta);
    if (cod_op != ESCRIBIR_OK) {
        log_error(cpu_logger, "Ocurrió un error al hacer MOV_OUT");
   }
   desplazamiento += direccion_fisica->size;
   free(valor_por_direccion);
   }
return;
}*/

void escribir_a_memoria(t_list* lista_paginas, int size,t_pcb* pcb, void* valor) {
    
    int bytes_grabados = 0;
    int tamano_a_escribir;

    for(int i = 0;i < list_size(lista_paginas); i++){ 

    t_direccion_fisica_io* direccion_fisica = list_get(lista_paginas,i);

    if( size < direccion_fisica->size){
        tamano_a_escribir = size;
    } else {
        tamano_a_escribir = direccion_fisica->size;
    }

    t_buffer *buffer_envio = crear_buffer();
    cargar_int_a_buffer(buffer_envio, pcb->pid);    
    cargar_int_a_buffer(buffer_envio, direccion_fisica->df);
    cargar_int_a_buffer(buffer_envio, tamano_a_escribir); 
    cargar_a_buffer(buffer_envio, valor + bytes_grabados, tamano_a_escribir);

    t_paquete *paquete = crear_paquete(ESCRIBIR, buffer_envio); 
    enviar_paquete(paquete, conexion_memoria);

    destruir_paquete(paquete); 

    char* valor_parcial_escrito_para_imprimir = malloc(tamano_a_escribir + 1);
    memcpy(valor_parcial_escrito_para_imprimir, valor + bytes_grabados, tamano_a_escribir);
    valor_parcial_escrito_para_imprimir[tamano_a_escribir] = '\0';
    log_info(cpu_logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pcb->pid, direccion_fisica->df, valor_parcial_escrito_para_imprimir);
    free(valor_parcial_escrito_para_imprimir);

    bytes_grabados += tamano_a_escribir;
    size -= tamano_a_escribir;

    op_code cod_op = recibir_operacion(conexion_memoria);
    t_buffer *buffer_respuesta = recibir_buffer(conexion_memoria);
    extraer_int_del_buffer(buffer_respuesta);
    destruir_buffer(buffer_respuesta);
    if (cod_op != ESCRIBIR_OK) {
        log_error(cpu_logger, "Ocurrió un error al hacer MOV_OUT");
   }   
   }
return;
}

void escribir_a_memoria_string(int dir_fisica, int size,int pid, char* valor) {
    t_buffer *buffer_envio = crear_buffer();
    cargar_int_a_buffer(buffer_envio, pid);    
    cargar_int_a_buffer(buffer_envio, dir_fisica);
    cargar_int_a_buffer(buffer_envio, size);
    //char* envio = malloc(sizeof(char)*20);
    //sprintf(envio, "%d", valor);
    //cargar_string_a_buffer(buffer_envio, envio);
    //free(envio);
    cargar_string_a_buffer(buffer_envio, valor);

    t_paquete *paquete = crear_paquete(ESCRIBIR, buffer_envio); 
    enviar_paquete(paquete, conexion_memoria);

    destruir_paquete(paquete); 

    op_code cod_op = recibir_operacion(conexion_memoria);
    t_buffer *buffer_respuesta = recibir_buffer(conexion_memoria);
    int basura = extraer_int_del_buffer(buffer_respuesta);
    destruir_buffer(buffer_respuesta);
    if (cod_op != ESCRIBIR_OK) {
        log_error(cpu_logger, "Ocurrió un error al hacer MOV_OUT");
   }
return;
}

//MMU
t_list* traducir_direccion_mmu(int dir_logica, t_pcb *ctx,int tamanio_solicitad)
{   int tamanio = 0;
    
      
    t_list* lista_df = list_create();
    int direccion_fisica;

    while(tamanio_solicitad > 0){ 

    int desplazamiento = dir_logica % tamanio_pagina;
    int disponible = tamanio_pagina - desplazamiento;  
    int nro_pagina = numero_pagina(dir_logica);
    
    
    if ( disponible < tamanio_solicitad){
            tamanio = disponible;
        }else{
            tamanio = tamanio_solicitad;
    }
    int nro_marco;
    nro_marco= consultar_tlb(tlb, nro_pagina, ctx->pid);
	if (nro_marco != -1){        
        direccion_fisica = (nro_marco * tamanio_pagina) + desplazamiento;     
        
    }
    else 
    {   
    nro_marco = solicitar_numero_de_marco(nro_pagina, ctx->pid);
    direccion_fisica = nro_marco * tamanio_pagina + desplazamiento; 
    if(nro_marco == -1)
    {
        return -1;
    } 

    log_info(cpu_logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d",ctx->pid,nro_pagina,nro_marco);    
    agregar_entry_tlb(ctx, nro_pagina, nro_marco);
    }
    t_direccion_fisica_io * dir_fisica;
    dir_fisica = malloc(sizeof(t_direccion_fisica_io));

    dir_fisica->df = direccion_fisica;
    dir_fisica->size = tamanio;

    list_add(lista_df, dir_fisica);
    dir_logica += tamanio;
    tamanio_solicitad -= tamanio;    

    }
    return lista_df;

}

int numero_pagina(int dir_logica)
{
    //Solicitar TAMAÑO PAGINA.
    return floor(dir_logica/ tamanio_pagina);
}


int solicitar_numero_de_marco(int num_pagina, int pid)
{
    t_buffer *buffer = crear_buffer();
    cargar_int_a_buffer(buffer,pid);
    cargar_int_a_buffer(buffer,num_pagina);

    t_paquete *paquete = crear_paquete(SOLICITUD_MARCO,buffer);
    enviar_paquete(paquete,conexion_memoria);
    destruir_paquete(paquete);
    int num_marco;
    op_code cod_op = recibir_operacion(conexion_memoria);
    t_buffer *buffer_recibido = recibir_buffer(conexion_memoria);
    num_marco = extraer_int_del_buffer(buffer_recibido);
    if(num_marco < 0)
    {
        log_error(cpu_logger, "Ocurrio un error al recibir el numero de marco");
        return -1;
    }    

   
    return num_marco;
}

//TLB
int consultar_tlb(t_list* tlb, int pagina, int pid) {
	// Devuelve el marco correspondiente a la pagina, si es que se encuentra.

	t_tlb_entry* entry = NULL;

	for (int i = 0; i < list_size(tlb); i++){
		entry = list_get(tlb, i);
		if (entry->pagina == pagina && entry->pid == pid) {

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

void agregar_entry_tlb(t_pcb* cpu, int pagina, int marco) {
	
    t_tlb_entry* entry = malloc(sizeof(t_tlb_entry));
	entry->pagina = pagina;
	entry->marco = marco;
    entry->pid = cpu->pid;

	

	// Checkeamos si agregar un elemento haria que nos pasemos del maximo de entradas permitidas, y en ese caso eliminamos el primero.
    //if(strcmp(ALGORITMO_TLB, "FIFO") == 0 || strcmp(ALGORITMO_TLB, "LRU") == 0){   //TODO: Preguntar a franco si esta bien.
    log_info(cpu_logger, "CANT ENTRADAS %d", atoi(CANTIDAD_ENTRADAS_TLB));
	if (list_size(tlb) >= atoi(CANTIDAD_ENTRADAS_TLB)) {
		t_tlb_entry* first_entry = list_remove(tlb, 0);
        log_info(cpu_logger, "TLB: entry de pagina %d agregado (reemplaza a pagina %d)", pagina, first_entry->pagina);
		free(first_entry);
	} else {
        log_info(cpu_logger, "TLB: entry de pagina %d agregado", pagina);
    }

	list_add(tlb, entry);
    return;                                             //TODO: Preguntar a franco si esta bien.
}
/*else {
    log_error(cpu_logger, "Algoritmo Incompatible");
    return;
}
}*/

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
/* 
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

*/


