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

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}


t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete = crear_paquete();

	// Leemos y esta vez agregamos las lineas al paquete
	leido = readline("> ");
	while(strlen(leido) > 0)
	{
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
		free(leido);
		leido = readline("> ");
	}


	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	free(leido);

	//Envio el paquete
	enviar_paquete(paquete, conexion);

	//Elimino el paquete
	eliminar_paquete(paquete);
	
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

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


/*int esperar_cliente(int socket_servidor)
{
	
		// Aceptamos un nuevo cliente
		int socket_cliente = accept(socket_servidor, NULL, NULL);
		log_info(logger, "Se conecto un cliente");

		if(socket_cliente == -1)
		{
			log_error(logger, "Error al aceptar un cliente");
			abort();
		} else {
		// Crear un hilo para atender al cliente
        pthread_t thread_id;
        int* socket_cliente_ptr = malloc(sizeof(int));
        *socket_cliente_ptr = socket_cliente;
        pthread_create(&thread_id, NULL, atender_cliente, socket_cliente_ptr);
		pthread_detach(thread_id);
		}
	
}*/

int esperar_cliente(int socket_servidor)
{
	
		// Aceptamos un nuevo cliente
		int socket_cliente = accept(socket_servidor, NULL, NULL);
		log_info(logger, "Se conecto un cliente");

		if(socket_cliente == -1)
		{
			log_error(logger, "Error al aceptar un cliente");
			abort();
		} else {
		/*// Crear un hilo para atender al cliente
        pthread_t thread_id;
        int* socket_cliente_ptr = malloc(sizeof(int));
        *socket_cliente_ptr = socket_cliente;
        pthread_create(&thread_id, NULL, atender_cliente, socket_cliente_ptr);
		pthread_detach(thread_id);*/
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

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
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
