#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/logging.h>
#include <utils/conexion_cliente.h>



t_log* cpu_logger;
t_config* cpu_config;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
char* CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;


#endif