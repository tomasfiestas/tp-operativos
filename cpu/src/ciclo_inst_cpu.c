#include "ciclo_inst_cpu.h"

int existe_interrupcion;
sem_t recibir_inst;
sem_t mutex_ctx;

void empezar_ciclo_instruccion(t_pcb *ctx)
{

    ctx;
    sem_init(&recibir_inst,0,0);
    sem_init(&mutex_ctx,0,1);
    t_instruccion *inst_a_ejecutar;
    existe_interrupcion = 0;
    pthread_t hilo_interrupciones;

    //creo hilo que recibe interrupciones de kernel
    pthread_create(&hilo_interrupciones, NULL, (void*)check_interrupt, ctx);

    while(ctx != NULL && !existe_interrupcion)
    {
        inst_a_ejecutar = fetch(ctx);

        if(!decode(inst_a_ejecutar))
        {
            ctx-> program_counter ++;
        }

        op_code operacion = execute(inst_a_ejecutar, ctx);

        switch (operacion)
        {
        case SET:

            log_info(logger_cpu, "PID: %d - Ejecutando SET - %s %s", ctx->pid, instruccion->parametros[0], instruccion->parametros[1]);
            asignar_registro(instruccion->parametros[0]);
            break;  
        

        //Completar en siguiente checkpoint


        case NADA:
            break;
        default:
            break;
        }

    }

    if(existe_interrupcion)
    {
        sem_post(&recibir_inst);
        pthread_join(hilo_interrupciones, NULL);
    }

}

t_instruccion *fetch(t_pcb *ctx)
{
    log_info(cpu_logger, "PID: %d - FETCH - Program Counter: %d", ctx->pid, ctx->program_counter);

    t_instruccion *instruccion = solicitar_instruccion_a_memoria(ctx->program_counter, ctx->pid);

    return instruccion;
}

bool decode(t_instruccion *inst)
{
    op_code operacion = inst->operacion;

    return(operacion == MOV_IN || operacion == MOV_OUT || operacion == F_READ || operacion == F_WRITE);
}

int execute(t_pcb** contexto_ejec, t_instruccion instruccion_actual)
{
    int direc_fisica;
    uint32_t *registro;
    switch (instruccion_actual -> operacion)
    {
    case SET: 
        log_info(cpu_logger, "PID: %d - Ejecutando: SET - %s %s", contexto_ejec->pid, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
        asignar_registro(instruccion_actual->parametros[0], instruccion_actual->parametros[1], contexto_ejec->registros);
        break;
    
    case SUM: 
        log_info(cpu_logger, "PID: %d - Ejecutando: SUM - %s %s", contexto_ejec->pid, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
        operar_con_registros(instruccion_actual->operacion, instruccion_actual->parametros[0], instruccion_actual->parametros[1], contexto_ejec->registros);
        break;

    case SUB: 
        log_info(cpu_logger, "PID: %d - Ejecutando: SUB - %s %s", contexto_ejec->pid, instruccion_actual->parametros[0], instruccion_actual->parametros[1]);
        operar_con_registros(instruccion_actual->operacion, instruccion_actual->parametros[0], instruccion_actual->parametros[1], contexto_ejec->registros);
        break;

    //aca irian las demas instrucciones de proximo checkpoint
    default:
        break;
    }
}


void check_interrupt(t_pcb *contexto_ejec)
{
    op_code causa_interrupcion;
    int pid;

    recv(cliente_kernel_interrupt, &causa_interrupcion, sizeof(op_code), MSG_WAITALL);
    recv(cliente_kernel_interrupt, &pid, sizeof(int), MSG_WAITALL);

    existe_interrupcion = 1;

    sem_wait(&receive_instruccion);
    sem_wait(&mutex_contexto_ejec);

    if(context_global != NULL && context_global->pid == pid)
    {
        enviar_contexto(context_global, causa_interrupcion, cliente_kernel_dispatch);
        liberar_contexto(context_global);
        context_global = NULL;
    }

    sem_post(&mutex_contexto_ejec);

}


//--------------------
//  Op con registros
//--------------------

void asignar_registro(char* reg, char* valor, t_registros *registros)
{

    if(!strcmp("AX", reg))
        memcpy((registros->AX), valor, 4);
    if(!strcmp("BX", reg))
        memcpy((registros->AX), valor, 4);
    if(!strcmp("CX", reg))
        memcpy((registros->AX), valor, 4);
    if(!strcmp("DX", reg))
        memcpy((registros->AX), valor, 4);
    

}

void operar_con_registros(op_code operacion, char* reg_origen, char* reg_destino, t_registros *registros)
{
    uint32_t *reg_origen = obtener_registro(reg_origen, registros);
    uint32_t *reg_destino = obtener_registro(reg_destino, registros);


    if(reg_destino != NULL && reg_origen != NULL)
    {
        switch (operacion)
        {
        case SUM:
            *reg_destino += reg_origen;
            break;
        case SUB:
            *reg_destino -= reg_origen;
            break;
        
        default:
            break;
        }
    }

}

uint32_t obtener_registro(const char *reg_actual, t_registros *registros)
{
    if(strcmp(reg_actual, "AX") ==0)
        return (registros->AX);
    if(strcmp(reg_actual, "BX") ==0)
        return (registros->BX);
    if(strcmp(reg_actual, "CX") ==0)
        return (registros->CX);
    if(strcmp(reg_actual, "DX") ==0)
        return (registros->DX);
}

t_instruccion *solicitar_instruccion_a_memoria(int program_counter, int pid)
{   
    //Solicito instruccion
    t_buffer* buffer_cpu_memoria = crear_buffer();    
    cargar_int_a_buffer(buffer_cpu_memoria, program_counter);
    cargar_int_a_buffer(buffer_cpu_memoria, pid);
    t_paquete *paquete = crear_paquete(SOLICITUD_INST,buffer_cpu_memoria);
    
    enviar_paquete(paquete, conexion_memoria);
    destruir(buffer_cpu_memoria);
    

   
    bool control_key = 1;
    while (control_key){
    op_code op_code = recibir_operacion(conexion_memoria);    
    switch(op_code) {
        case SOLICITUD_INST_OK:
            log_info(logger, "Me llegó la instrucción a ejecutar");
            t_buffer* buffer = recibir_buffer(conexion_memoria);
            log_info(logger, "EJECUTAMOS la instruccion recibida");                
            //atender_crear_pr(buffer); // deserializar instruccion de a una
            extraer_instrucciones_del_buffer(buffer);//retornar una instrucción
            break;
        case HANDSHAKE_CPU:
            log_info(logger, "Se conecto el CPU");
            break;
        case HANDSHAKE_MEMORIA:
            log_info(logger, "Se conecto la Memoria");
            break;
        case HANDSHAKE_ES:
            log_info(logger, "Se conecto el IO");
            break;
        default:
            log_error(logger, "No se reconoce el handshake");
            control_key = 0;
            break;
    }   } 
    
    

    
}