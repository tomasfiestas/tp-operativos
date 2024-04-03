#include <main.h>
int main(int argc, char* argv[]) {
    
    cpu_logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
    log_info(cpu_logger, "Logger CPU Iniciado");

    cpu_config = config_create("cpu.config");
    if (cpu_config == NULL) {
        log_error(cpu_logger, "No se pudo abrir el archivo de configuracion");
        return EXIT_FAILURE;
    }
    IP_MEMORIA = config_get_string_value(cpu_config, "IP_MEMORIA");
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
    log_info(cpu_logger, "ALGORITMO_TLB: %s", ALGORITMO_TLB);
}
