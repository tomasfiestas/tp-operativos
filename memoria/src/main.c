#include "main.h"
int main(int argc, char* argv[]) { 
    
    //Inicio el logger de la memoria
    memoria_logger = iniciar_logger("memoria.log", "LOGGER_MEMORIA");  
    
    //Inicio la configuracion de la memoria
    memoria_config = iniciar_config("memoria.config");
    
    //Obtengo los valores de la configuracion
    PUERTO_ESCUCHA = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
    log_info(memoria_logger, "PUERTO_ESCUCHA: %s", PUERTO_ESCUCHA); 
    TAM_MEMORIA = config_get_string_value(memoria_config, "TAM_MEMORIA");
    log_info(memoria_logger, "TAM_MEMORIA: %s", TAM_MEMORIA);
    TAM_PAGINA = config_get_string_value(memoria_config, "TAM_PAGINA");
    log_info(memoria_logger, "TAM_PAGINA: %s", TAM_PAGINA);
    PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
    log_info(memoria_logger, "PATH_INSTRUCCIONES: %s", PATH_INSTRUCCIONES);
    RETARDO_RESPUESTA = config_get_string_value(memoria_config, "RETARDO_RESPUESTA");
    log_info(memoria_logger, "RETARDO_RESPUESTA: %s", RETARDO_RESPUESTA);

    log_info(memoria_logger, "________________");

    //Inicio servidor Memoria
    int servidor_memoria = iniciar_servidor(PUERTO_ESCUCHA, memoria_logger);
    log_info(memoria_logger, "Servidor de memoria iniciado en el puerto %s", PUERTO_ESCUCHA);
    int cliente = esperar_cliente(servidor_memoria, memoria_logger);

    
    




    

}
