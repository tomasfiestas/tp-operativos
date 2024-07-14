#ifndef CPU_H
#define CPU_H

/*#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h> 
#include <utils/logging.h>
#include <utils/shared.h>
#include <pthread.h>
#include <instrucciones.h>*/
#include <gestor_cpu.h>
#include <commons/config.h>
#include <global_cpu.h>



//t_log* cpu_logger;




void atender_kernel_dispatch(void* socket_cliente_ptr);
void atender_kernel_interrupt(void* socket_cliente_ptr);
void atender_crear_pr(t_buffer* buffer);
void atender_fin_quantum(t_buffer* buffer);



#endif