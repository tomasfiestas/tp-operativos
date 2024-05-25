#include "registros.h"

void asignar_registro(char* registro, char* valor, t_registros* registros)
{
    uint32_t *registro_obtenido = obtener_registro(registro, registros);

    uint32_t valor_ent = strtoul(valor, NULL, 10);

    //completar
   

}

uint32_t *obtener_registro(const char *nombre, t_registros *registros)
{
    if(strcmp(nombre, "AX" == 0))
        return &(registros->AX);
    if(strcmp(nombre, "BX" == 0))
        return &(registros->AX);
    if(strcmp(nombre, "CX" == 0))
        return &(registros->AX);
    if(strcmp(nombre, "DX" == 0))
        return &(registros->AX);
    else    
        return NULL;
}

void operar_registros(op_code operacion, char* reg_origen, char* reg_destino, t_registros *registros)
{
    uint32_t *registro_destino = obtener_registro(reg_destino, registros);
    uint32_t *registro_origen = obtener_registro(reg_origen, registros);

    if(registro_destino != NULL && registro_origen != NULL)
   {
    switch (operacion)
    {
    case SUM:
        *registro_destino += *registro_origen;
        break;
    case SUB:
        *registro_destino -= registro_origen;
        break;
    default:
        break;
    }
   } 

}


//falta void crear registros?