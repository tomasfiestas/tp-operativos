#include "main.h"
int main(int argc, char* argv[]) { 

    //Inicio el logger del kernel
    kernel_logger = iniciar_logger("kernel.log", "LOGGER_KERNEL");  

    //Inicio la configuracion del kernel
    kernel_config = iniciar_config("kernel.config");
    puerto_escucha = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
    log_info(kernel_logger, "PUERTO_ESCUCHA %s", puerto_escucha); 
}
