#include "entradasalida.h"
t_log* logger;
int main(int argc, char* argv[]) { 
    //Inicio el logger de entradasalida 

    logger = iniciar_logger("entradasalida.log", "LOGGER_ENTRADASALIDA");  

    //Inicio la configuracion de entradasalida

    entradasalida_config = iniciar_config("entradasalida.config");
    TIPO_INTERFAZ = config_get_string_value(entradasalida_config, "TIPO_INTERFAZ");
    log_info(logger, "TIPO_ INTERFAZ: %s", TIPO_INTERFAZ); 
    TIEMPO_UNIDAD_TRABAJO = config_get_string_value(entradasalida_config, "TIEMPO_UNIDAD_TRABAJO");
    log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %s", TIEMPO_UNIDAD_TRABAJO);
    IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
    log_info(logger, "IP_KERNEL: %s", IP_KERNEL);
    PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
    log_info(logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);
    IP_MEMORIA = config_get_string_value(entradasalida_config, "IP_MEMORIA");
    log_info(logger, "IP_MEMORIA: %s", IP_MEMORIA);
    PUERTO_MEMORIA = config_get_string_value(entradasalida_config, "PUERTO_MEMORIA");
    log_info(logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    PATH_BASE_DIALFS = config_get_string_value(entradasalida_config, "PATH_BASE_DIALFS");
    log_info(logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);
    BLOCK_SIZE = config_get_string_value(entradasalida_config, "BLOCK_SIZE");
    log_info(logger, "LOCK_SIZE: %s", BLOCK_SIZE);
    BLOCK_COUNT = config_get_string_value(entradasalida_config, "BLOCK_COUNT");
    log_info(logger, "BLOCK_COUNT: %s", BLOCK_COUNT);


    //Creo conexion como cliente hacia Memoria

    conexion_memoria = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(logger, "Conexion con Memoria establecida");


    //Creo conexion como cliente hacia Kernel

    conexion_kernel = crear_conexion_cliente(IP_KERNEL, PUERTO_KERNEL);
    log_info(logger, "Conexion con Kernel establecida");   
    t_buffer* buffer = crear_buffer();
    cargar_string_a_buffer(buffer, "Nueva 1");
    cargar_string_a_buffer(buffer, "Tipo INTEL-ULTRA");

    t_paquete* paquete = crear_paquete(CREAR_NUEVA_INTERFAZ, buffer);
    enviar_paquete(paquete, conexion_kernel);

    //sleep(5);
    log_info(logger, "Mensaje enviado a Kernel");
    destruir_buffer(buffer);

    conexion_kernel2 = crear_conexion_cliente(IP_KERNEL, PUERTO_KERNEL);
    log_info(logger, "Conexion con Kernel establecida");   
    t_buffer* buffer2 = crear_buffer();
    cargar_string_a_buffer(buffer2, "Nueva 2");
    cargar_string_a_buffer(buffer, "Tipo ATOM");
    t_paquete* paquete2 = crear_paquete(CREAR_NUEVA_INTERFAZ, buffer2);
    enviar_paquete(paquete, conexion_kernel2);
    log_info(logger, "Mensaje enviado a Kernel");
    destruir_buffer(buffer2);

       
    realizar_handshake(HANDSHAKE_ES, conexion_kernel);
    realizar_handshake(HANDSHAKE_ES, conexion_memoria);

    //Leer consola
    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, (void *)leer_consola, NULL);
    pthread_detach(hilo_consola);

    pthread_t hilo_memoria;
    int* socket_cliente_memoria_ptr = malloc(sizeof(int));
    *socket_cliente_memoria_ptr = conexion_kernel;
    pthread_create(&hilo_memoria, NULL,atender_mensajes_memoria, socket_cliente_memoria_ptr);
    log_info(logger, "Esperando mensajes de Memoria");
    pthread_join(hilo_memoria,NULL);


    crear_bitmap();
    crear_archivo_bloques();

    
//Agregando verificacion de interfaz...

    while(1){
    

    t_paquete* paquete = recibir_paquete(conexion_kernel);
    op_code instruccion_recibida = recibir_operacion(paquete); 
    t_buffer* buffer = recibir_buffer(conexion_kernel);
    
    char* tipoInterfaz;
    char* nombre_interfaz_paquete;
    int tamanio;
    int direccion;

        switch(instruccion_recibida){
        
        case IO_GEN_SLEEP:
        
            tipoInterfaz = "GENERICA";
            nombre_interfaz_paquete = extraer_string_del_buffer(paquete);


            //Agregar paquete para mandar op_code error a Kernel.
            if(strcmp(tipoInterfaz, nombre_interfaz_paquete)){
            log_info(logger,"Interfaz incorrecta");
            break;
            }

            int tiempo_unidad_trabajo = config_get_int_value(entradasalida_config, "TIEMPO_UNIDAD_TRABAJO");
            int* cantidad_dormir = extraer_int_del_buffer(paquete);
            int tiempo_sleep = tiempo_unidad_trabajo * (*cantidad_dormir);

            if(tiempo_sleep > 0){

                    usleep(tiempo_sleep*1000);

                    printf("Dormi %d ", tiempo_sleep);

            } else {

                printf("El parametro no es valido");

            }

        break;

        case IO_STDIN_READ:

        tipoInterfaz = "STDIN";
        nombre_interfaz_paquete = extraer_string_del_buffer(paquete);
        
        if(strcmp(tipoInterfaz, nombre_interfaz_paquete)){
            log_info(logger,"Interfaz incorrecta");
            break;
        }
    
        direccion = extraer_uint32_del_buffer(paquete);
        tamanio = extraer_uint32_del_buffer(paquete);
        char* entrada_teclado = readline("Ingrese un texto: ");

        if(entrada_teclado == NULL){
            printf("Error: No se ingreso ningun texto.\n");
            break;
        }


        if(strlen(entrada_teclado) > tamanio){
            printf("Error: El texto ingresado es demasiado largo, se recortara al tamanio indicado.\n");
        }

        char* entrada_final =  malloc(tamanio + 1); // +1 para el carácter espurio al final
        strncpy(entrada_final, entrada_teclado, tamanio);

    

        //Creo paquete y se lo envio al socket de memoria 
        t_buffer* buffer2 = crear_buffer();
        cargar_uint32_a_buffer(buffer2, direccion);
        cargar_uint32_a_buffer(buffer2, tamanio);
        t_paquete* paqueteIN = crear_paquete(IO_STDIN_READ,buffer2);
        enviar_paquete(paqueteIN, conexion_memoria);
        //eliminar_paquete(paqueteIN);
        break;

        case IO_STDOUT_WRITE:

        tipoInterfaz= "STDOUT";
        nombre_interfaz_paquete = extraer_string_del_buffer(paquete);

        if(strcmp(tipoInterfaz, nombre_interfaz_paquete)){
            log_info(logger,"Interfaz incorrecta");
            break;
        }
        
        direccion = extraer_uint32_del_buffer(paquete);
        tamanio = extraer_uint32_del_buffer(paquete);


        t_buffer* buffer = crear_buffer();
        cargar_uint32_a_buffer(buffer, direccion);
        cargar_uint32_a_buffer(buffer, tamanio);
        t_paquete* paqueteOUT = crear_paquete(SOLICITAR_LECTURA,buffer);
        enviar_paquete(paqueteOUT, conexion_memoria);

        default: 

            printf("Instruccion no reconocida");

        break;

    }

    destruir_buffer(buffer);    
    return EXIT_SUCCESS;

}
}

void atender_mensajes_memoria(void* socket_cliente_ptr){
    int cliente_kernel2 = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
    while (control_key){
        op_code op_code = recibir_operacion(cliente_kernel2);
        switch (op_code){
            case HANDSHAKE_KERNEL:
			log_info(logger, "Se conecto el Kernel");
			break;
		case HANDSHAKE_CPU:
			log_info(logger, "Se conecto el CPU");
			break;
		case HANDSHAKE_MEMORIA:
			log_info(logger, "Se conecto la Memoria");
			break;
		case HANDSHAKE_ES:
			log_info(logger, "Se conecto el IO");
			break;
		default:
			log_error(logger, "No se reconoce el handshake");
			control_key = 0;
			break;
        }
    }
}

void leer_consola()
{
	char *linea;
    
    
    while (1) {
        linea = readline(">");
        
        if (!linea) {
            break;
        }

        if (linea) {

            add_history(linea);
            char** argumentos = string_split(linea, " ");
            t_mensajes_consola mensaje_consola;
            mensaje_consola = mensaje_a_consola(argumentos[0]);                         

            switch(mensaje_consola){
                case CREAR:
                t_buffer* buffer = crear_buffer();
                cargar_string_a_buffer(buffer, argumentos[1]); 
                cargar_string_a_buffer(buffer, argumentos[2]);
                t_paquete* paquete = crear_paquete(CREAR_NUEVA_INTERFAZ,buffer);
                log_info(logger, "Conexion con Kernel establecida");   
                enviar_paquete(paquete, conexion_kernel);
                eliminar_paquete(paquete);
                break;
                
                case EXIT:
                    exit(0);
                    break;
                case ERROR:
                    printf("Este comando es invalido\n");
                    break;
                default:
                    printf("Este comando es invalido\n");
                    break;               

            }           
        
        free(linea);
    }
    
}
 

}

t_mensajes_consola mensaje_a_consola(char *mensaje_consola){
    
    if(strcmp(mensaje_consola,"CREAR") == 0){
        return CREAR;
    }    
    else
        return ERROR;
}