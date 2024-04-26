#include "shared.h"

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

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}


void* atender_cliente(void* socket_cliente_ptr)
{
    int socket_cliente = *(int*)socket_cliente_ptr;
    free(socket_cliente_ptr);

    // manejar aca la conexion con el cliente

	module_code handshake = recibir_operacion(socket_cliente);
    handle_handshake(handshake);
}




int esperar_cliente(int socket_servidor)
{
	
		// Aceptamos un nuevo cliente
		int socket_cliente = accept(socket_servidor, NULL, NULL);
		log_info(logger, "Se conecto un cliente");

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



void realizar_handshake(module_code module, int server){
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

void handle_handshake(module_code module) {
	switch(module) {
		case KERNEL:
			log_info(logger, "Se conecto el Kernel");
			break;
		case CPU:
			log_info(logger, "Se conecto el CPU");
			break;
		case MEMORIA:
			log_info(logger, "Se conecto la Memoria");
			break;
		case IO:
			log_info(logger, "Se conecto el IO");
			break;
		default:
			log_error(logger, "No se reconoce el handshake");
			abort();
			break;
	}
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