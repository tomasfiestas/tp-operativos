#include "main.h"
int main(int argc, char* argv[]) { 
    //Inicio el logger de la memoria
    memoria_logger = iniciar_logger("memoria.log", "LOGGER_MEMORIA");  
    //Inicio la configuracion de la memoria
    memoria_config = iniciar_config("memoria.config");
    puerto_escucha = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
    log_info(memoria_logger, "PUERTO_ESCUCHA: %s", puerto_escucha); 
}
