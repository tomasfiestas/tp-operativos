#include "entradasalida.h"

char* TIPO_INTERFAZ;
char* TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;
t_config* entradasalida_config;
int conexion_kernel;
int conexion_kernel2;
int conexion_memoria;
char* nombre_interfaz2;

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

    interfaces = list_create();    
    conexion_kernel = crear_conexion_cliente(IP_KERNEL, PUERTO_KERNEL);
    log_info(io_logger, "Conexion con Kernel establecida"); 

    //Creo conexion como cliente hacia Memoria
    conexion_memoria = crear_conexion_cliente(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(io_logger, "Conexion con Memoria establecida");

    inicializar_interfaces(argv[1]);


   

    //Creo conexion como cliente hacia Kernel
      
       
   
    
    
       
    //realizar_handshake(HANDSHAKE_ES, conexion_kernel);
    log_info(io_logger, "Handshake con Kernel realizado");
    //realizar_handshake(HANDSHAKE_ES, conexion_memoria);
    log_info(io_logger, "Handshake con Memoria realizado");

    

    //Atender mensajes de Kernel
    pthread_t hilo_kernel;
    int* socket_cliente_kernel_ptr = malloc(sizeof(int));
    *socket_cliente_kernel_ptr = conexion_kernel;
    pthread_create(&hilo_kernel, NULL,atender_kernel, socket_cliente_kernel_ptr);
    log_trace(io_logger, "Esperando mensajes de Kernel");
    pthread_join(hilo_kernel,NULL);  
    


    

        return EXIT_SUCCESS;
    }





t_mensajes_consola mensaje_a_consola(char *mensaje_consola){
    
    if(strcmp(mensaje_consola,"CREAR") == 0){
        return CREAR;
    }    
    else
        return ERROR;
}


void inicializar_interfaces(char* path){
 t_config* entradasalida_config2 = iniciar_config(path);    
    
        
        nombre_interfaz2 = config_get_string_value(entradasalida_config2,"NOMBRE_INTERFAZ");
        
        
        
        char* TIPO_INTERFAZ = config_get_string_value(entradasalida_config2, "TIPO_INTERFAZ");
        log_info(io_logger, "TIPO_INTERFAZ: %s", TIPO_INTERFAZ);

        
        int tiempo_obtenido = config_get_int_value(entradasalida_config2,"TIEMPO_UNIDAD_TRABAJO");
        log_info(io_logger, "TIEMPO_UNIDAD_TRABAJO: %d", tiempo_obtenido);

        if(strcmp(TIPO_INTERFAZ, "DIALFS") == 0){            
            BLOCK_SIZE = config_get_int_value(entradasalida_config2, "BLOCK_SIZE");
            log_info(io_logger, "BLOCK_SIZE: %d", BLOCK_SIZE);
            BLOCK_COUNT = config_get_int_value(entradasalida_config2, "BLOCK_COUNT");
            log_info(io_logger, "BLOCK_COUNT: %d", BLOCK_COUNT);
            RETRASO_COMPACTACION = config_get_int_value(entradasalida_config2, "RETRASO_COMPACTACION");
            log_info(io_logger, "RETRASO: %d", RETRASO_COMPACTACION);
            crear_interfaz_fs(nombre_interfaz2, TIPO_INTERFAZ, tiempo_obtenido, BLOCK_SIZE, BLOCK_COUNT, RETRASO_COMPACTACION);
        }else{
            crear_interfaz(nombre_interfaz2, TIPO_INTERFAZ,tiempo_obtenido);
        }
        

    t_buffer* buffer_memoria = crear_buffer();    
    cargar_string_a_buffer(buffer_memoria, nombre_interfaz2);    
    t_paquete* paquete_memoria = crear_paquete(CREAR_NUEVA_INTERFAZ,buffer_memoria);
    enviar_paquete(paquete_memoria, conexion_memoria);
    destruir_paquete(paquete_memoria);

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

    crear_bitmap();
    crear_archivo_bloques();
    
    

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
    log_trace(io_logger, "Operacion recibida: %d", op_code);
    t_buffer* buffer = recibir_buffer(cliente_kernel2);       
    t_struct_atender_kernel* struct_atender_kernel = malloc(sizeof(t_struct_atender_kernel));
    struct_atender_kernel->codigo_operacion = op_code;
    struct_atender_kernel->buffer = buffer;
    atender_peticiones_de_kernel(struct_atender_kernel);  
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

                    log_trace(io_logger,"Dormi %d ", tiempo_sleep);

            } else {

                log_trace(io_logger,"El parametro no es valido");

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
        
        char* dato_a_leer; 
        log_info(io_logger,"Tamanio a leer %d ",tamanio_a_leer);
        dato_a_leer = (char*)leer_de_memoria(direcciones_a_leer, tamanio_a_leer, pid, conexion_memoria);
        dato_a_leer[tamanio_a_leer] = '\0';
        log_info(io_logger,"Dato leido: %s",dato_a_leer); //Es necesario el tamanio?

        list_destroy(direcciones_a_leer);  //VERIFICAR PARA LIBERAR MEMORIA BIEN !!!
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
        log_info(io_logger, "PID: %d - Crear Archivo: %s",pid,nombre_archivo_nuevo);
        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_FS_CREATE");
        

        break;
         
        case IO_FS_DELETE:

        usleep(tiempo_fs* 1000);
        char* nombre_archivo_a_borrar = extraer_string_del_buffer(buffer_recibido);
        marcar_bloques_libres(nombre_archivo_a_borrar);
        eliminar_archivo_metadata(nombre_archivo_a_borrar);

        free(nombre_archivo_a_borrar);
        log_info(io_logger, "PID: %d - Eliminar Archivo: %s",pid,nombre_archivo_nuevo);
        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_FS_DELETE");
        
        
        break;

        
        case IO_FS_TRUNCATE:

        usleep(tiempo_fs* 1000);

        char* nombre_archivo_a_truncar = extraer_string_del_buffer(buffer_recibido);
        int tamanio_a_truncar = atoi(extraer_string_del_buffer(buffer_recibido)); //int o uint????
        t_fcb* fcb_truncar = leer_metadata(nombre_archivo_a_truncar);
        if(fcb_truncar->TAMANIO_ARCHIVO > tamanio_a_truncar){
            achicar_archivo(fcb_truncar, tamanio_a_truncar);
        } else {
            agrandar_archivo(fcb_truncar, tamanio_a_truncar, pid);
        }

        free(fcb_truncar);
        log_info(io_logger, "PID: %d - Truncar Archivo: %s - %d",pid,nombre_archivo_nuevo,tamanio_a_truncar);
        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_FS_TRUNCATE");
        
        break;

        
        case IO_FS_WRITE:
        //Aca me van a mandar nombre archivo, registro direccion, registro tamaño y registro puntero archivo (offset)
        //Arreglar bien el orden de las cosas por las dudas.
        //Nombre archivo, lista df, tamanio, offset
        usleep(tiempo_fs* 1000);
        
        char* nombre_archivo_escribir = extraer_string_del_buffer(buffer_recibido); 
        
        int tamanio_lectura = atoi(extraer_string_del_buffer(buffer_recibido));
        int offset = atoi(extraer_string_del_buffer(buffer_recibido)); // ojo tipos de datos.
        t_list* lista_direcciones_escribir = extraer_lista_de_direcciones_de_buffer(buffer_recibido);
        

        void* dato_a_escribir = leer_de_memoria(lista_direcciones_escribir, tamanio_lectura, pid, conexion_memoria);
        log_info(io_logger, "PID: %d - Escribir Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d",pid,nombre_archivo_escribir,tamanio_lectura,offset);


        t_fcb *fcb_fs_write = leer_metadata(nombre_archivo_escribir);
        escribir_archivo(fcb_fs_write->BLOQUE_INICIAL, offset, dato_a_escribir, tamanio_lectura);

        free(nombre_archivo_escribir);
        free(fcb_fs_write);
        free(dato_a_escribir);

        list_destroy(lista_direcciones_escribir);
        
        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_FS_WRITE");

        break;
        
        case IO_FS_READ:
        usleep(tiempo_fs* 1000);
        char* nombre_archivo_leer = extraer_string_del_buffer(buffer_recibido);
        
        int tamanio_escritura = atoi(extraer_string_del_buffer(buffer_recibido));
        int offset_archivo = atoi(extraer_string_del_buffer(buffer_recibido)); 
        t_list* lista_direcciones_a_escribir = extraer_lista_de_direcciones_de_buffer(buffer_recibido);
        

        t_fcb *fcb_read = leer_metadata(nombre_archivo_leer);
        char* dato_leido = leer_archivo(tamanio_escritura, fcb_read, offset_archivo);

        enviar_para_escribir(lista_direcciones_a_escribir, dato_leido, pid, conexion_memoria); 

        free(dato_leido);
        free(nombre_archivo_leer);
        free(fcb_read);
        list_destroy(lista_direcciones_a_escribir);

        log_info(io_logger, "PID: %d - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d",pid,nombre_archivo_leer,tamanio_escritura,offset_archivo);

        instruccion_realizada(conexion_kernel, nombre_recibido, pid, "IO_FS_READ");
        break;
        

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
        if(op_code != ESCRIBIR_OK){
            log_error(io_logger, "No se escribió correctamente en memoria");
        }
        tamanio_a_sacar += t_df->size;
        destruir_buffer(buffer_respuesta);

    }
}



void enviar_solicitud_escritura(int pid, int direccion_fisica, int tamanio,void* valor_a_escribir){

   t_buffer* buffer_escritura = crear_buffer();
    cargar_string_a_buffer(buffer_escritura, nombre_interfaz2); //Nombre interfaz
    cargar_int_a_buffer(buffer_escritura, pid);    //PID
    cargar_int_a_buffer(buffer_escritura, direccion_fisica); //Direccion fisica
    cargar_int_a_buffer(buffer_escritura, tamanio); //Tamanio
    cargar_a_buffer(buffer_escritura, valor_a_escribir,tamanio); //Valor a escribir


    t_paquete* paquete_escritura = crear_paquete(IO_STDIN_READ, buffer_escritura);
    enviar_paquete(paquete_escritura, conexion_memoria);
    destruir_paquete(paquete_escritura);

    
}

void enviar_solicitud_lectura(int pid, int direccion_fisica, int tamanio, int socket_memoria){
    t_buffer* buffer_lectura = crear_buffer();
    cargar_string_a_buffer(buffer_lectura, nombre_interfaz2);
    cargar_int_a_buffer(buffer_lectura, pid);
    cargar_int_a_buffer(buffer_lectura, direccion_fisica);
    cargar_int_a_buffer(buffer_lectura, tamanio);

    t_paquete* paquete_lectura = crear_paquete(IO_STDOUT_WRITE, buffer_lectura); 
    enviar_paquete(paquete_lectura, conexion_memoria);
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
    log_info(io_logger, "Ingrese una cadena de texto");
    string_a_leer = readline("> ");

    if(string_length(string_a_leer) > tamanio){
        log_trace(io_logger, "La cadena ocupa %i y deberia ocupar %i, se recortara", string_length(string_a_leer), tamanio);
    }
    char* string_recortado = string_substring_until(string_a_leer, tamanio);
    log_info(io_logger, "La cadena ingresada es: %s ", string_recortado);
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
    cargar_string_a_buffer(buffer_envio, nombre_interfaz2);
    cargar_int_a_buffer(buffer_envio, pid);  //PID
    cargar_int_a_buffer(buffer_envio, dir_fisica->df);  //DIR_FISICA
    cargar_int_a_buffer(buffer_envio, dir_fisica->size);    //TAMANIO A LEER

    t_paquete* paquete = crear_paquete(IO_STDOUT_WRITE, buffer_envio);  //LEER 
    enviar_paquete(paquete, conexion_memoria);
    destruir_paquete(paquete);

    op_code cod_op = recibir_operacion(conexion_memoria);
    if (cod_op != LEER_OK) {
        log_error(io_logger, "Ocurrió un error al hacer MOV_IN");
        return NULL;
    }

    t_buffer* buffer_respuesta = recibir_buffer(conexion_memoria);
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
    cargar_string_a_buffer(buffer_envio, nombre_interfaz2);
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
    
        log_trace(io_logger, "Operacion finalizada de %s, PID: %d", instruccion_realizada, pid);

}
t_fcb* crear_fcb(char* nombre_archivo){
    t_fcb* fcb = malloc(sizeof(t_fcb));

    if(fcb == NULL) { return NULL; } 

    fcb->nombre_archivo = nombre_archivo;
    fcb->TAMANIO_ARCHIVO = 0;
    fcb->BLOQUE_INICIAL = bitmap_encontrar_bloque_libre();
    log_trace(io_logger, "EL bloque inicial designado es %i", fcb->BLOQUE_INICIAL);

    return fcb;
}