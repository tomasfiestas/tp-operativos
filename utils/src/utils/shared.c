#include "shared.h"
#include <netdb.h>

t_log* logger = NULL;

//CLIENTE
int crear_conexion_cliente(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);


	freeaddrinfo(server_info);

	return socket_cliente;
}




void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}


void enviar_paquete(t_paquete* paquete,int socket_cliente)
{	void* a_enviar = serializar_paquete(paquete);
	int bytes = paquete->buffer->size + 2*sizeof(int);
	

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}
void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

//SERVIDOR

int iniciar_servidor(char* puerto)
{
	
	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    if(setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
        perror("setsockopt(SO_REUSEADDR) failed");
    }
	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	//log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}






int esperar_cliente(int socket_servidor)
{
	
		// Aceptamos un nuevo cliente
		int socket_cliente = accept(socket_servidor, NULL, NULL);
		//log_info(logger, "Se conecto un cliente");

		if(socket_cliente == -1)
		{
			log_error(logger, "Error al aceptar un cliente");
			abort();
		}

		return socket_cliente;
	
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}



void realizar_handshake(op_code module, int server){
    int *handshake = malloc(sizeof(int));
    if (handshake == NULL) {
        log_info(logger,"Failed to allocate memory for handshake");
        return;
    }

    *handshake = module;
    ssize_t bytes_sent = send(server, handshake, sizeof(int), 0);
    if (bytes_sent == -1) {
        log_info(logger,"Failed to send handshake");
    }

    free(handshake);
}



//AGREGO FUNCIONES DE PRUEBA PARA MANDAR PAQUETES Y CREAR BUFFERS

t_buffer* crear_buffer(){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	buffer->stream = NULL;
	return buffer;
}
void destruir_buffer(t_buffer* buffer){
    if(buffer->stream != NULL){
        free(buffer->stream);
     }    
    free(buffer);
}

void cargar_a_buffer(t_buffer* buffer, void* valor, int tamanio){
    if(buffer -> size == 0){
        buffer->stream = malloc(sizeof(int) + tamanio);
        memcpy(buffer->stream, &tamanio, sizeof(int));
        memcpy(buffer->stream + sizeof(int), valor, tamanio);
    }
    else{
        buffer->stream = realloc(buffer->stream, buffer->size + sizeof(int) + tamanio);
        memcpy(buffer->stream + buffer->size, &tamanio, sizeof(int));
        memcpy(buffer->stream + buffer->size + sizeof(int), valor, tamanio);
    }
    buffer->size += sizeof(int);
    buffer->size += tamanio;
}
void cargar_int_a_buffer(t_buffer* buffer, int valor){
    cargar_a_buffer(buffer, &valor, sizeof(int));
}
void cargar_string_a_buffer(t_buffer* buffer, char* valor){
    cargar_a_buffer(buffer, valor, strlen(valor) + 1);
}

void cargar_uint32_a_buffer(t_buffer* buffer, uint32_t valor){
    cargar_a_buffer(buffer, &valor, sizeof(uint32_t));
}
void cargar_uint8_a_buffer(t_buffer* buffer, uint8_t valor){
    cargar_a_buffer(buffer, &valor, sizeof(uint8_t));
}
void cargar_contexto_ejecucion_a_buffer(t_buffer* buffer, t_pcb* pcb){
    cargar_int_a_buffer(buffer, pcb->pid);
    cargar_estado_a_buffer(buffer, pcb->estado);
    cargar_registros_a_buffer(buffer, pcb->registros);
    cargar_int_a_buffer(buffer, pcb->quantum);
    cargar_int_a_buffer(buffer, pcb->ejecuto);   
    
}
void cargar_estado_a_buffer(t_buffer* buffer, t_estado estado){
    cargar_a_buffer(buffer, &estado, sizeof(t_estado));
}
void cargar_registros_a_buffer(t_buffer* buffer, t_registros registros){
    cargar_a_buffer(buffer, &registros, sizeof(t_registros));
}

void* extraer_de_buffer(t_buffer* buffer){
    if(buffer->size == 0){
       printf("\n Error al extraer contenido del buffer VACIO\n");
       exit(EXIT_FAILURE);
    }
    if(buffer->size < 0){
        printf("\n Error al extraer contenido del buffer por tamanio negativo\n");
        exit(EXIT_FAILURE);
    }

    int size_buffer;
    memcpy(&size_buffer, buffer->stream, sizeof(int));
    void* valor = malloc(size_buffer);
    memcpy(valor, buffer->stream + sizeof(int), size_buffer);

    int new_size = buffer->size - sizeof(int) - size_buffer;
    if(new_size == 0){
        buffer->size = 0;
        free(buffer->stream);
        buffer->stream = NULL;
        return valor;
    }
    if(new_size < 0){
        printf("\n Error al extraer contenido del buffer por tamanio negativo\n");
        exit(EXIT_FAILURE);
    }
    void* nuevo_stream = malloc(new_size);
    memcpy(nuevo_stream, buffer->stream + sizeof(int) + size_buffer, new_size);
    free(buffer->stream);
    buffer->size = new_size;
    buffer->stream = nuevo_stream;

    return valor;
}

/*t_pcb recibir_contexto_ejecucion(t_buffer* buffer){
    t_pcb pcb = extraer_pcb_del_buffer(buffer);    
    //Esto es solo para probar que se recibio bien el pcb
    log_info(logger, "PID: %d", pcb.pid);
    log_info(logger, "PC: %d", pcb.program_counter);
    log_info(logger,"ejecuto: %d", pcb.ejecuto);

    return pcb;
}*/
/*t_pcb extraer_pcb_del_buffer(t_buffer* buffer){
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb = extraer_de_buffer(buffer);
    t_pcb valor_pcb = *pcb;
    free(pcb);
    return valor_pcb;
}*/
t_pcb* extraer_pcb_del_buffer(t_buffer* buffer){
    t_pcb* pcb ;
    pcb = extraer_de_buffer(buffer);
    
    return pcb;
}
void cargar_pcb_a_buffer(t_buffer* buffer, t_pcb* pcb){
    cargar_a_buffer(buffer, pcb, sizeof(t_pcb));
}

void cargar_pcb_a_buffer2(t_buffer* buffer, t_pcb pcb){
    cargar_a_buffer(buffer, &pcb, sizeof(t_pcb));
}

void cargar_instruccion_a_buffer(t_buffer* buffer, t_instruccion* instruccion) {
    cargar_a_buffer(buffer, instruccion, sizeof(t_instruccion));
}

t_estado extraer_estado_del_buffer(t_buffer* buffer){
    t_estado* estado = malloc(sizeof(t_estado));
    estado = extraer_de_buffer(buffer);
    t_estado valor_estado = *estado;
    free(estado);
    return valor_estado;
}

t_registros extraer_registros_del_buffer(t_buffer* buffer){
    t_registros* registros = malloc(sizeof(t_registros));
    registros = extraer_de_buffer(buffer);
    t_registros valor_registros = *registros;
    free(registros);
    return valor_registros;
}

t_instruccion extraer_instruccion_del_buffer(t_buffer* buffer){
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    instruccion = extraer_de_buffer(buffer);
    t_instruccion valor_instruccion = *instruccion;
    free(instruccion);
    return valor_instruccion;
}


int extraer_int_del_buffer(t_buffer* buffer){
    int* entero = extraer_de_buffer(buffer);
    int valor_int = *entero;
    free(entero);
    return valor_int;
}

char* extraer_string_del_buffer(t_buffer* buffer){
    char* string = extraer_de_buffer(buffer);
    return string;
}
char extraer_string_del_buffer2(t_buffer* buffer){
    char* string = extraer_de_buffer(buffer);
    char valor_string = *string;
    free(string);
    return valor_string;
}
uint8_t extraer_uint8_del_buffer(t_buffer* buffer){
    uint8_t* entero = extraer_de_buffer(buffer);
    uint8_t valor_int = *entero;
    free(entero);
    return valor_int;
}
uint32_t extraer_uint32_del_buffer(t_buffer* buffer){
    uint32_t* entero = extraer_de_buffer(buffer);
    uint32_t valor_int = *entero;
    free(entero);
    return valor_int;
}

t_buffer* recibir_buffer(int conexion){
    t_buffer* buffer = malloc(sizeof(t_buffer));

    if(recv(conexion, &(buffer->size), sizeof(int), MSG_WAITALL) > 0){
        buffer -> stream = malloc(buffer->size);
        if(recv(conexion, buffer->stream, buffer->size, MSG_WAITALL) > 0){
            return buffer;
        }else {
            printf("Error al recibir el buffer\n");
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("Error al recibir el tamanio del buffer\n");
        exit(EXIT_FAILURE);
    }
    return buffer;
}   

t_paquete* crear_paquete(op_code cod_op, t_buffer* buffer){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = cod_op;
    paquete->buffer = buffer;
    return paquete;
}
void destruir_paquete(t_paquete* paquete){
    destruir_buffer(paquete->buffer);
    free(paquete);
}

void* serializar_paquete(t_paquete* paquete){
	int size = paquete->buffer->size + 2*sizeof(int);
	void* coso = malloc(size);
	int desplazamiento = 0;

	memcpy(coso + desplazamiento, &(paquete->codigo_operacion),sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(coso + desplazamiento, &(paquete->buffer->size),sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(coso + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return coso;
}