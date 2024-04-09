#include "logging.h"

t_log* iniciar_logger(char* nombre_archivo, char* nombre_proceso) {
    t_log* nuevo_logger;
    nuevo_logger = log_create(nombre_archivo, nombre_proceso, 1, LOG_LEVEL_INFO);
    log_info(nuevo_logger, "%s Iniciado",nombre_proceso);

    if (nuevo_logger == NULL) {
        log_error(nuevo_logger, "No se pudo crear el logger");
        exit (EXIT_FAILURE);
    }
    return nuevo_logger;
    }

t_config* iniciar_config(char* nombre_archivo)
{
	t_config* nuevo_config = config_create(nombre_archivo);
	if(nuevo_config == NULL)
	{
		perror("No se pudo crear el config\n");
		exit(EXIT_FAILURE);
	}

	return nuevo_config;
}
/*     IP_MEMORIA = config_get_string_value(cpu_config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(cpu_config, "PUERTO_MEMORIA");
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(cpu_config, "PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(cpu_config, "PUERTO_ESCUCHA_INTERRUPT");
    CANTIDAD_ENTRADAS_TLB = config_get_string_value(cpu_config, "CANTIDAD_ENTRADAS_TLB");
    ALGORITMO_TLB = config_get_string_value(cpu_config, "ALGORITMO_TLB");

    log_info(cpu_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(cpu_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(cpu_logger, "PUERTO_ESCUCHA_DISPATCH: %s", PUERTO_ESCUCHA_DISPATCH);
    log_info(cpu_logger, "PUERTO_ESCUCHA_INTERRUPT: %s", PUERTO_ESCUCHA_INTERRUPT);
    log_info(cpu_logger, "CANTIDAD_ENTRADAS_TLB: %s", CANTIDAD_ENTRADAS_TLB);
    log_info(cpu_logger, "ALGORITMO_TLB: %s", ALGORITMO_TLB); */

