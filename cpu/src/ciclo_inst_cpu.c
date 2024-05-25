#include "../include/ciclo_inst_cpu.h"

int hayInterrupcion;
sem_t recibir_inst;
sem_t mutex_ctx;

void empezar_ciclo_instruccion(t_contexto_ejecucion *ctx)
{

    context_global = ctx;
    sem_init(&recibir_inst,0,0);
    sem_init(&mutex_ctx,0,1);
    t_instruccion *inst_a_ejecutar;
    hayInterrupcion = 0;
    pthread_t hilo_interrupciones;

    //creo hilo que recibe interrupciones de kernel
    pthread_create(&hilo_interrupciones, NULL, (void*)check_interrupt, ctx);

    while(context_global != NULL && !hayInterrupcion)
    {
        inst_a_ejecutar = fetch(context_global);

        if(!decode(inst_a_ejecutar))
        {
            context_global-> program_counter ++;
        }

        op_code operacion = execute(inst_a_ejecutar, context_global);

        switch (operacion)
        {
        case SET:

            log_info(logger_cpu, "PID: %d - Ejecutando SET - %s %s", ctx->pid, instruccion->parametros[0], instruccion->parametros[1]);
            asignar_registro(instruccion->parametros[0]);
            return NADA,;
        

        //completar


        case NADA:
            break;
        default:
            break;
        }

    }

    if(hayInterrupcion)
    {
        sem_post(&recibir_inst);
        pthread_join(hilo_interrupciones, NULL);
    }

}

t_instruccion *fetch(t_contexto_ejecucion *ctx)
{
    log_info(logger_cpu, "PID: %d - FETCH - Program Counter: %d", ctx->pid, ctx->program_counter);

    t_instruccion *instruccion = solicitar_inst_a_memoria(ctx->program_counter, ctx->pid);

    return instruccion;
}

bool decode(t_instruccion *inst)
{
    op_code operacion = inst->operacion;

    return(operacion == MOV_IN || operacion == MOV_OUT || operacion == F_READ || operacion == F_WRITE);
}

int execute(t_contexto_ejecucion** contexto_ejec, t_instruccion instruccion_actual)
{
    int direc_fisica;
    uint32_t *registro;
    switch (instruccion_actual -> operacion)
    {
    case SET: 
        asignar_registro(instruccion_actual->parametros[0], instruccion_actual->parametros[1], contexto_ejec->registros);
        break;;
    
    case SUM: 
        operar_con_registros(instruccion_actual->operacion, instruccion_actual->parametros[0], instruccion_actual->parametros[1], contexto_ejec->registros);
        break;

    case SUM: 
        operar_con_registros(instruccion_actual->operacion, instruccion_actual->parametros[0], instruccion_actual->parametros[1], contexto_ejec->registros);
        break;

    //aca irian las demas instrucciones de proximo checkpoint
    default:
        break;
    }
}


void check_interrupt(t_contexto_ejecucion *contexto_ejec)
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