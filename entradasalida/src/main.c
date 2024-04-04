#include "main.h"
int main(int argc, char* argv[]) { 
    //Inicio el logger de entradasalida 
    entradasalida_logger = iniciar_logger("entradasalida.log", "LOGGER_ENTRADASALIDA");  
    //Inicio la configuracion de entradasalida
    entradasalida_config = iniciar_config("entradasalida.config");
    tipo_interfaz = config_get_string_value(entradasalida_config, "TIPO_INTERFAZ");
    log_info(entradasalida_logger, "TIPO_ INTERFAZ: %s", tipo_interfaz); 
}
