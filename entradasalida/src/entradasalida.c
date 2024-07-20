#include "entradasalida.h"

char* TIPO_INTERFAZ;
char* TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
char* BLOCK_SIZE;
char* BLOCK_COUNT;
char* RETRASO_COMPACTACION;
t_config* entradasalida_config;
int conexion_kernel;
int conexion_kernel2;
int conexion_memoria;

t_log* io_logger;
t_list* interfaces;
int tiempo_fs;
//cambiar a logger a uno mas claro
int main(int argc, char* argv[]) { 
    //Inicio el logger de entradasalida 

    io_logger = iniciar_logger("entradasalida.log", "LOGGER_ENTRADASALIDA");  

    //Inicio la configuracion de entradasalida. 

    entradasalida_config = iniciar_config("entradasalida.config");

    TIPO_INTERFAZ = config_get_string_value(entradasalida_config, "TIPO_INTERFAZ");
    log_info(io_logger, "TIPO_ INTERFAZ: %s", TIPO_INTERFAZ); 

    TIEMPO_UNIDAD_TRABAJO = config_get_string_value(entradasalida_config, "TIEMPO_UNIDAD_TRABAJO");
    log_info(io_logger, "TIEMPO_UNIDAD_TRABAJO: %s", TIEMPO_UNIDAD_TRABAJO);

    IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
    log_info(io_logger, "IP_KERNEL: %s", IP_KERNEL);

    PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
    log_info(io_logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);

    IP_MEMORIA = config_get_string_value(entradasalida_config, "IP_MEMORIA");
    log_info(io_logger, "IP_MEMORIA: %s", IP_MEMORIA);

    PUERTO_MEMORIA = config_get_string_value(entradasalida_config, "PUERTO_MEMORIA");
    log_info(io_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);

    PATH_BASE_DIALFS = config_get_string_value(entradasalida_config, "PATH_BASE_DIALFS");
    log_info(io_logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);

    BLOCK_SIZE = config_get_string_value(entradasalida_config, "BLOCK_SIZE");
    log_info(io_logger, "LOCK_SIZE: %s", BLOCK_SIZE);
    
    BLOCK_COUNT = config_get_string_value(entradasalida_config, "BLOCK_COUNT");
    log_info(io_logger, "BLOCK_COUNT: %s", BLOCK_COUNT);

    RETRASO_COMPACTACION = config_get_string_value(entradasalida_config, "RETRASO_COMPACTACION");
    log_info(io_logger, "RETRASO_COMPACTACION: %s", RETRASO_COMPACTACION);

    interfaces = list_create();    
    conexion_kernel = crear_conexion_cliente(IP_KERNEL, PUERTO_KERNEL);
    log_info(io_logger, "Conexion con Kernel establecida"); 
    inicializar_interfaces(argv[1]);


    //Creo conexion como cliente hacia Memoria
    conexion_memoria = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(io_logger, "Conexion con Memoria establecida");


    //Creo conexion como cliente hacia Kernel
      
       
   
    crear_bitmap();
    crear_archivo_bloques();
    
       
    //realizar_handshake(HANDSHAKE_ES, conexion_kernel);
    log_info(io_logger, "Handshake con Kernel realizado");
    //realizar_handshake(HANDSHAKE_ES, conexion_memoria);
    log_info(io_logger, "Handshake con Memoria realizado");

    

    //Atender mensajes de Kernel
    pthread_t hilo_kernel;
    int* socket_cliente_kernel_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_ptr = conexion_kernel;
    pthread_create(&hilo_kernel, NULL,atender_kernel, socket_cliente_kernel_ptr);
    log_info(io_logger, "Esperando mensajes de Kernel");
    pthread_detach(hilo_kernel);


    pthread_t hilo_memoria;
    int* socket_cliente_memoria_ptr = malloc(sizeof(int));
    *socket_cliente_memoria_ptr = conexion_memoria;
    pthread_create(&hilo_memoria, NULL,atender_mensajes_memoria, socket_cliente_memoria_ptr);
    log_info(io_logger, "Esperando mensajes de Memoria");
    pthread_join(hilo_memoria,NULL);


    

        return EXIT_SUCCESS;
    }


void atender_mensajes_memoria(void* socket_cliente_ptr){
    int cliente_kernel2 = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);
    bool control_key = 1;
    while (control_key){
        op_code op_code = recibir_operacion(cliente_kernel2);
        switch (op_code){
            case HANDSHAKE_KERNEL:
			log_info(io_logger, "Se conecto el Kernel");
			break;
		case HANDSHAKE_CPU:
			log_info(io_logger, "Se conecto el CPU");
			break;
		case HANDSHAKE_MEMORIA:
			log_info(io_logger, "Se conecto la Memoria");
			break;
		case HANDSHAKE_ES:
			log_info(io_logger, "Se conecto el IO");
			break;
		default:
			log_error(io_logger, "No se reconoce el handshake");
			//control_key = 0;
			break;
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

/*int crear_conexion_interfaz(char* nombre_interfaz, t_interfaz* interfaz){ 

    t_buffer* buffer = crear_buffer();

    char* ip_kernel = interfaz->IP_KERNEL;
    char* puerto_kernel = interfaz->PUERTO_KERNEL;
    char* tipo_interfaz = interfaz->TIPO_INTERFAZ;

    conexion_kernel =  crear_conexion_cliente(ip_kernel,puerto_kernel);
    log_info(io_logger, "Conexion establecida con %s ", nombre_interfaz);

    t_paquete* paquete = crear_paquete(CREAR_NUEVA_INTERFAZ, buffer);
    cargar_string_a_buffer(buffer, nombre_interfaz);
    cargar_string_a_buffer(buffer, tipo_interfaz);
    enviar_paquete(paquete, conexion_kernel);
    destruir_buffer(buffer);

}*/

void inicializar_interfaces(char* path){
 t_config* entradasalida_config2 = iniciar_config(path);
    int cantidad_interfaces = config_get_int_value(entradasalida_config2, "CANTIDAD_INTERFACES");
    for(int i = 0;i < cantidad_interfaces; i++){
        // Concatenate the value of 'i' to 'nombre_interfaz'
        
        char nombre_interfaz[16] = "NOMBRE_INTERFAZ";
        
        char buffer_nombre[5];
        sprintf(buffer_nombre, "%d", i);
        strcat(nombre_interfaz, buffer_nombre);
        char* nombre_interfaz2 = config_get_string_value(entradasalida_config2,nombre_interfaz);
        
        
        log_info(io_logger, "TIPO: %s", nombre_interfaz2);
        char tipo_interfaz[14] = "TIPO_INTERFAZ";
        char buffer_tipo[5];
        sprintf(buffer_tipo, "%d", i);
        strcat(tipo_interfaz, buffer_tipo);
        char* TIPO_INTERFAZ = config_get_string_value(entradasalida_config2, tipo_interfaz);
        log_info(io_logger, "TIPO_INTERFAZ: %s", TIPO_INTERFAZ);

        char tiempo[30] = "TIEMPO_UNIDAD_TRABAJO";
        
        char buffer_tiempo[5];
        sprintf(buffer_tiempo, "%d", i);
        strcat(tiempo, buffer_tiempo);
        int tiempo_obtenido = config_get_int_value(entradasalida_config2,tiempo);
        log_info(io_logger, "TIEMPO_UNIDAD_TRABAJO: %d", tiempo_obtenido);

        if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0){            
            int block_size = config_get_int_value(entradasalida_config2, "BLOCK_SIZEFS");
            log_info(io_logger, "BLOCK_SIZE: %d", block_size);
            int block_count = config_get_int_value(entradasalida_config2, "BLOCK_COUNTFS");
            log_info(io_logger, "BLOCK_COUNT: %d", block_count);
            int retraso = config_get_int_value(entradasalida_config2, "RETRASO_COMPACTACION");
            log_info(io_logger, "RETRASO: %d", retraso);
            crear_interfaz_fs(nombre_interfaz2, TIPO_INTERFAZ, tiempo_obtenido, block_size, block_count, retraso);
        }else{
            crear_interfaz(nombre_interfaz2, TIPO_INTERFAZ,tiempo_obtenido);
        }
        



    }
 }
 void crear_interfaz(char* nombre, char* tipo,int unidades_trabajo){
    t_tipo_interfaz tipo_enum = map_tipo_a_enum(tipo);
    t_interfaz* interfaz = malloc(sizeof(t_interfaz));
    interfaz->nombre = nombre;
    interfaz->tipo = tipo_enum;
    interfaz->tiempo_unidad_trabajo = unidades_trabajo;



    t_buffer* buffer = crear_buffer();    
    cargar_string_a_buffer(buffer, nombre);
    cargar_string_a_buffer(buffer, tipo);
    t_paquete* paquete = crear_paquete(CREAR_NUEVA_INTERFAZ,buffer);
    enviar_paquete(paquete, conexion_kernel);
    destruir_paquete(paquete);

    list_add(interfaces, interfaz);
    

    log_info(io_logger, "Interfaz %s creada", nombre);

 }

 void crear_interfaz_fs(char* nombre, char* tipo, int tiempo_unidad_trabajo,int block_size, int block_count, int retraso_compactacion){
    t_tipo_interfaz tipo_enum = map_tipo_a_enum(tipo);
    t_dial_fs* dial_fs = malloc(sizeof(t_dial_fs));
    dial_fs->nombre = nombre;
    dial_fs->tipo = tipo_enum;
    dial_fs->tiempo_unidad_trabajo = tiempo_unidad_trabajo;
    dial_fs->block_size = block_size;
    dial_fs->block_count = block_count;
    dial_fs->retraso_compactacion = retraso_compactacion;

    t_buffer* buffer = crear_buffer();

    
    tiempo_fs = tiempo_unidad_trabajo;    
    cargar_string_a_buffer(buffer, nombre);
    cargar_string_a_buffer(buffer, tipo);
    t_paquete* paquete = crear_paquete(CREAR_NUEVA_INTERFAZ,buffer);
    enviar_paquete(paquete, conexion_kernel);
    destruir_paquete(paquete);

    
    

    log_info(io_logger, "DialFS %s creada", nombre);
 }

 int map_tipo_a_enum(char* tipo){
    if(strcmp(tipo, "GENERICA") == 0){
        return GENERICA;
    }
    if(strcmp(tipo, "STDIN") == 0){
        return STDIN;
    }
    if(strcmp(tipo, "STDOUT") == 0){
        return STDOUT;
    }
    if(strcmp(tipo, "DIALFS") == 0){
        return DIALFS;
    }
    return -1;
 }

 void atender_kernel(void* socket_cliente_ptr){   
    int cliente_kernel2 = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr); 
    bool control_key = 1;
    while(control_key){    
    op_code op_code = recibir_operacion(cliente_kernel2);
    t_buffer* buffer = recibir_buffer(cliente_kernel2);
    pthread_t hilo_kernel;    
    t_struct_atender_kernel* struct_atender_kernel = malloc(sizeof(t_struct_atender_kernel));
    struct_atender_kernel->codigo_operacion = op_code;
    struct_atender_kernel->buffer = buffer;
    pthread_create(&hilo_kernel, NULL,atender_peticiones_de_kernel, struct_atender_kernel);
    pthread_detach(hilo_kernel);    
    }
 }

 void atender_peticiones_de_kernel(t_struct_atender_kernel* struct_atender_kernel){
    
        t_buffer* buffer_recibido = struct_atender_kernel->buffer;
        int pid = extraer_int_del_buffer(buffer_recibido);
        char* nombre_recibido = extraer_string_del_buffer(buffer_recibido);
        t_interfaz *interfaz = buscar_interfaz(nombre_recibido);   

                
        switch(struct_atender_kernel->codigo_operacion){           
        case IO_GEN_SLEEP:       
            

            int tiempo_unidad_trabajo = interfaz->tiempo_unidad_trabajo;
            int cantidad_dormir = atoi(extraer_string_del_buffer(buffer_recibido));
            
            destruir_buffer(buffer_recibido);
            int tiempo_sleep = tiempo_unidad_trabajo * cantidad_dormir;

            if(tiempo_sleep > 0){

                    usleep(tiempo_sleep*1000);

                    log_info(io_logger,"Dormi %d ", tiempo_sleep);

            } else {

                log_info(io_logger,"El parametro no es valido");

            }
        
        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_GEN_SLEEP");       

        break;

        case IO_STDIN_READ:
        
        int tamanio = atoi(extraer_string_del_buffer(buffer_recibido));// Tamanio       
        
        

        t_list* lista_direcciones = crear_lista_direcciones(buffer_recibido);

        char* entrada_teclado = leer_de_consola(tamanio);

                
        enviar_para_escribir(lista_direcciones ,entrada_teclado ,pid);
        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_STDIN_READ");        

        break;
        
        case IO_STDOUT_WRITE:
        int tamanio_a_leer = atoi(extraer_string_del_buffer(buffer_recibido));
        t_list* direcciones_a_leer = crear_lista_direcciones(buffer_recibido);        
        //int* tamanio_a_leer = tamanio_a_leer_direcciones(direcciones_a_leer, cantidad_a_leer);

        char* dato_a_leer = (char*)leer_de_memoria(direcciones_a_leer, tamanio_a_leer, pid, conexion_memoria);
        log_info(io_logger,"Dato leido: %s", dato_a_leer); //Es necesario el tamanio?

        list_destroy_and_destroy_elements(direcciones_a_leer,free);
        free(dato_a_leer);

        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_STDOUT_WRITE");

        break;
        
        case IO_FS_CREATE:

        usleep(tiempo_fs * 1000);
        char* nombre_archivo_nuevo =  extraer_string_del_buffer(buffer_recibido);
        t_fcb* fcb_crear = crear_fcb(nombre_archivo_nuevo);
        bitmap_marcar_bloque_ocupado(fcb_crear->BLOQUE_INICIAL);
        crear_archivo_metadata(fcb_crear);

        free(nombre_archivo_nuevo);
        free(fcb_crear);
    
        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_FS_CREATE");

        break;
         
        case IO_FS_DELETE:

        usleep(tiempo_fs* 1000);
        char* nombre_archivo_a_borrar = extraer_string_del_buffer(buffer_recibido);
        marcar_bloques_libres(nombre_archivo_a_borrar);
        eliminar_archivo_metadata(nombre_archivo_a_borrar);

        free(nombre_archivo_a_borrar);

        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_FS_DELETE");
        
        break;

        /* 
        case IO_FS_TRUNCATE:

        usleep(interfaz->tiempo_unidad_trabajo * 1000);

        char* nombre_archivo_a_truncar = extraer_string_del_buffer(buffer_recibido);
        uint32_t tamanio_a_truncar = extraer_int_del_buffer(buffer_recibido); //int o uint????
        t_fcb* fcb_truncar = leer_metadata(nombre_archivo_a_truncar);
        if(fcb_truncar->TAMANIO_ARCHIVO > tamanio_a_truncar){
            achicar_archivo(fcb_truncar, tamanio_a_truncar);
        } else {
            agrandar_archivo(fcb_truncar, tamanio_a_truncar, pid);
        }

        free(fcb_truncar);

        instruccion_realizada(buffer_response, conexion_kernel, nombre_recibido, pid, "IO_FS_TRUNCATE");
        break;


        case IO_FS_WRITE:
        //Aca me van a mandar nombre archivo, registro direccion, registro tamaño y registro puntero archivo (offset)
        //Arreglar bien el orden de las cosas por las dudas.
        //Nombre archivo, lista df, tamanio, offset
        usleep(interfaz->tiempo_unidad_trabajo * 1000);
        
        char* nombre_archivo_escribir = extraer_string_del_buffer(buffer_recibido); 
        t_list* lista_direcciones_escribir = extraer_lista_de_direcciones_de_buffer(buffer_recibido);
        int tamanio_lectura = tamanio_a_leer_direcciones(lista_direcciones_escribir); // consultar  si saco tamaño de las fs o de lo que me pasa
        int offset = extraer_int_del_buffer(buffer_recibido); // ojo tipos de datos.
        

        char* dato_a_escribir = (char*)leer_de_memoria(lista_direcciones_escribir, tamanio_lectura, pid, conexion_memoria);
        log_info(io_logger, "Escribir archivo %s, PID: %i, Tamaño a escribir: %i, Offset: %i", nombre_archivo_escribir, pid, tamanio_lectura, offset);


        t_fcb *fcb_fs_write = leer_metadata(nombre_archivo_escribir);
        escribir_archivo(fcb_fs_write->BLOQUE_INICIAL, offset, dato_a_escribir, tamanio_lectura);

        free(nombre_archivo_escribir);
        free(fcb_fs_write);
        free(dato_a_escribir);

        list_destroy(lista_direcciones_escribir);
        instruccion_realizada(buffer_response, conexion_kernel, nombre_recibido, pid, "IO_FS_WRITE");

        break;

        case IO_FS_READ:
        usleep(interfaz->tiempo_unidad_trabajo * 1000);
        char* nombre_archivo_leer = extraer_string_del_buffer(buffer_recibido);
        t_list* lista_direcciones_a_escribir = extraer_lista_de_direcciones_de_buffer(buffer_recibido);
        int tamanio_escritura = tamanio_a_leer_direcciones(lista_direcciones_a_escribir);
        int offset_archivo = extraer_int_del_buffer(buffer_recibido); 

        log_info(io_logger, "Escribir archivo %s, PID: %i, Tamaño a leer: %i, Offset: %i", nombre_archivo_leer, pid, tamanio_escritura, offset_archivo);

        t_fcb *fcb_read = leer_metadata(nombre_archivo_leer);
        char* dato_leido = leer_archivo(tamanio_escritura, fcb_read, offset_archivo);

        enviar_para_escribir(lista_direcciones_a_escribir, dato_leido, pid, conexion_memoria); 

        free(dato_leido);
        free(nombre_archivo_leer);
        free(fcb_read);
        list_destroy(lista_direcciones_a_escribir);

        instruccion_realizada(buffer_response, conexion_kernel, nombre_recibido, pid, "IO_FS_READ");
        break;
        */

        default: 

            printf("Instruccion no reconocida");

        break;

    } 
    free(nombre_recibido);   
    free(struct_atender_kernel);


    
 }

 t_interfaz* buscar_interfaz(char* nombre) {
	for (int i = 0; i < list_size(interfaces); i++) {
		t_interfaz* interfaz = list_get(interfaces, i);
		if (strcmp(interfaz->nombre, nombre) == 0) {
			return interfaz;			
		}		
	}
	return NULL;
}



void enviar_para_escribir(t_list* lista_direcciones_escribir ,char* string ,int pid_read, int socket_memoria){
    
    int tamanio_a_sacar = 0;
    
    for(int j = 0; j < list_size(lista_direcciones_escribir); j++ ){
        t_direccion_fisica_io *t_df = list_get(lista_direcciones_escribir, j);
        char* string_a_mandar = string_substring(string, tamanio_a_sacar, t_df->size);

        log_trace(io_logger, "direccion a mandar: %i", t_df->df);
        log_trace(io_logger, "string que se manda: %s", string_a_mandar);

        enviar_solicitud_escritura(pid_read, t_df->df,t_df->size, string_a_mandar);
        op_code op_code = recibir_operacion(conexion_memoria);
        t_buffer* buffer_respuesta = recibir_buffer(conexion_memoria);
        int algo = extraer_int_del_buffer(buffer_respuesta);
        if(op_code == ESCRIBIR_OK){
            log_error(io_logger, "No se escribió correctamente en memoria");
        }
        tamanio_a_sacar += t_df->size;
        destruir_buffer(buffer_respuesta);

    }
}



void enviar_solicitud_escritura(int pid, int direccion_fisica, int tamanio,char* valor_a_escribir){

   t_buffer* buffer_escritura = crear_buffer();
    cargar_int_a_buffer(buffer_escritura, pid);    //PID
    cargar_int_a_buffer(buffer_escritura, direccion_fisica); //Direccion fisica
    cargar_int_a_buffer(buffer_escritura, tamanio); //Tamanio
    cargar_string_a_buffer(buffer_escritura, valor_a_escribir); //Valor a escribir


    t_paquete* paquete_escritura = crear_paquete(IO_STDIN_READ, buffer_escritura);
    enviar_paquete(paquete_escritura, conexion_kernel);
    destruir_paquete(paquete_escritura);

    
}

void enviar_solicitud_lectura(int pid, int direccion_fisica, int tamanio, int socket_memoria){
    t_buffer* buffer_lectura = crear_buffer();
    cargar_int_a_buffer(buffer_lectura, pid);
    cargar_int_a_buffer(buffer_lectura, direccion_fisica);
    cargar_int_a_buffer(buffer_lectura, tamanio);

    t_paquete* paquete_lectura = crear_paquete(IO_STDOUT_WRITE, buffer_lectura); 
    enviar_paquete(paquete_lectura, conexion_kernel);
    destruir_paquete(paquete_lectura);

}

t_list* crear_lista_direcciones(t_buffer* buffer){
    t_list *lista_direcciones_fs_read = list_create();  //TODO VER DE IMPLEMENTAR ESTO
    int cantidad_direcciones = extraer_int_del_buffer(buffer);    
    for(int i = 0; i < cantidad_direcciones; i++){
        t_direccion_fisica_io* direccion = malloc(sizeof(t_direccion_fisica_io));
        direccion->df = extraer_int_del_buffer(buffer);
        direccion->size = extraer_int_del_buffer(buffer);
        list_add(lista_direcciones_fs_read, direccion);
    }
    return lista_direcciones_fs_read;
}

int tamanio_a_leer_direcciones(t_list* lista_direcciones){
    int tamanio_a_leer = 0;
    int cantidad_a_leer = list_size(lista_direcciones);

    for(int i = 0; i < cantidad_a_leer; i++){
        t_direccion_fisica_io* direccion_fisica = list_get(lista_direcciones, i);
        tamanio_a_leer += direccion_fisica->size;
    }
    return tamanio_a_leer;
}


char* leer_de_consola(int tamanio){

    char* string_a_leer;
    
    string_a_leer = readline("> ");

    if(string_length(string_a_leer) > tamanio){
        log_trace(io_logger, "La cadena ocupa %i y deberia ocupar %i, se recortara", string_length(string_a_leer), tamanio);
    }
    char* string_recortado = string_substring_until(string_a_leer, tamanio);
    log_trace(io_logger, "La cadena ingresada es: %s ", string_recortado);
    free(string_a_leer);
    return string_recortado;
}


void* leer_de_memoria(t_list* lista_df,int bytes_a_leer, int pid, int socket_memoria)
{
    void* contenido = malloc(bytes_a_leer);
    int bytes_datos = 0;

    for(int i=0;i < list_size(lista_df); i++) { 
    t_direccion_fisica_io* dir_fisica = list_get(lista_df,i);

    t_buffer* buffer_envio = crear_buffer();
    cargar_int_a_buffer(buffer_envio, pid);  //PID
    cargar_int_a_buffer(buffer_envio, dir_fisica->df);  //DIR_FISICA
    cargar_int_a_buffer(buffer_envio, dir_fisica->size);    //TAMANIO A LEER

    t_paquete* paquete = crear_paquete(IO_STDOUT_WRITE, buffer_envio);  //LEER 
    enviar_paquete(paquete, socket_memoria);
    destruir_paquete(paquete);

    op_code cod_op = recibir_operacion(socket_memoria);
    if (cod_op != LEER_OK) {
        log_error(io_logger, "Ocurrió un error al hacer MOV_IN");
        return NULL;
    }

    t_buffer* buffer_respuesta = recibir_buffer(socket_memoria);
    if (buffer_respuesta == NULL) {
        log_error(io_logger, "Error al recibir el buffer de respuesta");
        return NULL;
    }

    void* valor_leido = extraer_de_buffer(buffer_respuesta); //extraigo lo leido de memoria
    destruir_buffer(buffer_respuesta);
    memcpy(contenido + bytes_datos,valor_leido,dir_fisica->size); //va + bytes_datos?
    //Tengo entendido que este memcpy me junta contenido con valor leido y asi va iterando.

    bytes_datos += dir_fisica->size;

    free(valor_leido);
    free(dir_fisica);
    }

    return contenido; //preguntar
}

void escribir_a_memoria(t_list* lista_paginas, int size,t_pcb* pcb, void* valor) {

    int bytes_grabados = 0;
    int tamano_a_escribir;

    for(int i = 0;i < list_size(lista_paginas); i++){ 

    t_direccion_fisica_io* direccion_fisica = list_get(lista_paginas,i);

    if( size < direccion_fisica->size){
        tamano_a_escribir = size;
    } else {
        tamano_a_escribir = direccion_fisica->size;
    }

    t_buffer *buffer_envio = crear_buffer();
    cargar_int_a_buffer(buffer_envio, pcb->pid);
    cargar_int_a_buffer(buffer_envio, direccion_fisica->df);
    cargar_int_a_buffer(buffer_envio, tamano_a_escribir); 
    cargar_a_buffer(buffer_envio, valor + bytes_grabados, tamano_a_escribir);

    t_paquete* paquete = crear_paquete(IO_STDIN_READ, buffer_envio); 
    enviar_paquete(paquete, conexion_memoria);

    destruir_paquete(paquete); 

    char* valor_parcial_escrito_para_imprimir = malloc(tamano_a_escribir + 1);
    memcpy(valor_parcial_escrito_para_imprimir, valor + bytes_grabados, tamano_a_escribir);
    valor_parcial_escrito_para_imprimir[tamano_a_escribir] = '\0';
    log_info(io_logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", pcb->pid, direccion_fisica->df, valor_parcial_escrito_para_imprimir);
    free(valor_parcial_escrito_para_imprimir);

    bytes_grabados += tamano_a_escribir;
    size -= tamano_a_escribir;

    op_code cod_op = recibir_operacion(conexion_memoria);
    t_buffer *buffer_respuesta = recibir_buffer(conexion_memoria);
    extraer_int_del_buffer(buffer_respuesta);
    destruir_buffer(buffer_respuesta);
    if (cod_op != ESCRIBIR_OK) {
        log_error(io_logger, "Ocurrió un error al hacer MOV_OUT");
   }
   }
return;
}

void instruccion_realizada(int socket_kernel, char* nombre_recibido, int pid, char* instruccion_realizada){
        t_buffer* buffer_response = crear_buffer();
        cargar_string_a_buffer(buffer_response, nombre_recibido);
        cargar_int_a_buffer(buffer_response, pid);

        t_paquete* paquete_response = crear_paquete(OPERACION_FINALIZADA, buffer_response);
        
        enviar_paquete(paquete_response, conexion_kernel);
        destruir_paquete(paquete_response);

        log_info(io_logger, "Operacion finalizada de %s, PID: %d", instruccion_realizada, pid);

}
/*
t_list* extraer_lista_de_direcciones_de_buffer(t_buffer* buffer){
    int cantidad_direcciones = extraer_int_del_buffer(buffer);
    t_list* lista_direcciones = list_create();

    for(int i=0;i < cantidad_direcciones; i++){
        t_direccion_fisica_io* direccion_fisica = extraer_direccion_de_buffer(buffer);
        list_add(lista_direcciones,direccion_fisica);
    }
    return lista_direcciones;
}
*/
t_fcb* crear_fcb(char* nombre_archivo){
    t_fcb* fcb = malloc(sizeof(t_fcb));

    if(fcb == NULL) { return NULL; } 

    fcb->nombre_archivo = nombre_archivo;
    fcb->TAMANIO_ARCHIVO = 0;
    fcb->BLOQUE_INICIAL = bitmap_encontrar_bloque_libre();
    log_trace(io_logger, "EL bloque inicial designado es %i", fcb->BLOQUE_INICIAL);

    return fcb;
}

