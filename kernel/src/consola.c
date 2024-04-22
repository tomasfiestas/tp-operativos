#include "consola.h"
t_mensajes_consola mensaje_a_consola(char *mensaje_consola){
    if(strcmp(mensaje_consola,"EJECUTAR_SCRIPT") == 0){
        return EJECUTAR_SCRIPT;
    }
    if(strcmp(mensaje_consola,"INICIAR_PROCESO") == 0){
        return INICIAR_PROCESO;
    }
    if(strcmp(mensaje_consola,"FINALIZAR_PROCESO") == 0){
        return FINALIZAR_PROCESO;
    }
    if(strcmp(mensaje_consola,"INICIAR_PLANIFICACION") == 0){
        return INICIAR_PLANIFICACION;
    }
    if(strcmp(mensaje_consola,"DETENER_PLANIFICACION") == 0){
        return DETENER_PLANIFICACION;
    }
    if(strcmp(mensaje_consola,"MULTIPROGRAMACION") == 0){
        return MULTIPROGRAMACION;
    }
    if(strcmp(mensaje_consola,"PROCESO_ESTADO") == 0){
        return PROCESO_ESTADO;
    }
    if(strcmp(mensaje_consola,"EXIT") == 0){
        return EXIT;
    }
    else
        return ERROR;
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
            t_mensajes_consola mensaje_consola;
            mensaje_consola = mensaje_a_consola(linea);
            switch(mensaje_consola){
                case EJECUTAR_SCRIPT:
                    printf("EJECUTAR_SCRIPT\n");
                    break;
                case INICIAR_PROCESO:
                    printf("INICIAR_PROCESO\n");
                    break;
                case FINALIZAR_PROCESO:
                    printf("FINALIZAR_PROCESO\n");
                    break;
                case INICIAR_PLANIFICACION:
                    printf("INICIAR_PLANIFICACION\n");
                    break;
                case DETENER_PLANIFICACION:
                    printf("DETENER_PLANIFICACION\n");
                    break; 
                case MULTIPROGRAMACION:
                    printf("MULTIPROGRAMACION\n");
                    break;
                case PROCESO_ESTADO:
                    printf("PROCESO_ESTADO\n");
                    break;
                case EXIT:
                    exit(0);
                    break;
                case ERROR:
                    printf("Este comando es invalido\n");
                    break;               

            }           
        
        free(linea);
    }
    
}
	

}