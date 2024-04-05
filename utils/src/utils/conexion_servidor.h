#ifndef CONEXION_SERVIDOR_H_
#define CONEXION_SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>



typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto, t_log* logger);
int esperar_cliente(int socket_servidor, t_log* logger);
t_list* recibir_paquete(int);
void recibir_mensaje(int socket_cliente, t_log* logger);
int recibir_operacion(int);


#endif /* UTILS_H_ */
