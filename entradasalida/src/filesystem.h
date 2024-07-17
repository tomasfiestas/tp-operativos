#include <commons/bitarray.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
//#include <utils/conexion.h>
#include <commons/string.h>
#include <math.h>
#include <commons/config.h>
#include <dirent.h>

typedef struct fcb{
    char* nombre_archivo;
    uint32_t TAMANIO_ARCHIVO;
    uint32_t BLOQUE_INICIAL;
} t_fcb;

typedef struct t_bitmap{
    char* direccion;
    uint32_t tamanio;
    t_bitarray *bitarray;
} t_bitmap;


extern char* bitmap;

extern t_bitarray* bitarray;

int max(int a, int b);

void crear_archivo_bloques();

void crear_archivo_bitmap();

int bitmap_encontrar_bloque_libre();

void bitmap_marcar_bloque_libre(int numero_bloque);

void bitmap_marcar_bloques_libres(int bloque_inicial, int bloque_final);

void bitmap_marcar_bloque_ocupado(int numero_bloque);

void bitmap_marcar_bloques_ocupados(int bloque_inicial, int bloque_final);

int bitmap_encontrar_bloques_libres_continuos(int tamanio_archivo);

int bloques_necesarios(int size);

bool hay_espacio_contiguo(t_fcb* fcb, int bloques, int bloques_necesarios);

void crear_archivo_metadata(t_fcb *fcb);

char* get_fullpath(char* nombre_archivo);

t_fcb* leer_metadata(char* nombre_archivo);

void eliminar_archivo_metadata(int size);

void borrar_archivo(char* nombre_archivo);

void marcar_bloques_libres(char* nombre_archivo);

void escribir_archivo(int bloque_inicial, int offset, void *dato, int size);

char* leer_archivo(int tamanio, t_fcb *fcb, int offset);

void agrandar_archivo(t_fcb *fcb, int tamanio_nuevo, int pid);

int compactar(t_fcb* fcb);

void achicar_archivo(t_fcb *fcb, int tamanio_nuevo);

t_list* leer_directorio();

bool comparar_tamanios_fcbs(void *e1, void* e2);

char* leer_bloques(int bloque_inicial, int tamanio);

char* buscar_contenido_fcb(t_fcb* fcb);

int copiar_contenido_a(char* contenido, int tamanio);


