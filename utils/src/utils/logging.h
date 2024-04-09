#ifndef LOGGING_H
#define LOGGING_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>



t_log* iniciar_logger(char* nombre_archivo, char* nombre_proceso);
t_config* iniciar_config(char* nombre_archivo);

#endif