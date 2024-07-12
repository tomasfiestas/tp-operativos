#include <commons/bitarray.h>


struct typedef {
    int BLOQUE_INICIAL;
    int TAMANIO_ARCHIVO;
} t_filesystem;

typedef struct fcb{
    char* nombre_archivo;
    uint32_t TAMANIO_ARCHIVO;
    uint32_t BLOQUES_INICIAL;
    uint32_t CANTIDAD_BLOQUES;
} t_fcb;


extern char* bitmap;
extern t_bitarray* bitarray;

void crear_bitmap();
void crear_archivo_bloques();