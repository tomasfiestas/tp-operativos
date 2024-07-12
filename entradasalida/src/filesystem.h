#ifndef _MANEJO_DE_FILESYSTEM_H_
#define _MANEJO_DE_FILESYSTEM_H_
#include <commons/string.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <commons/string.h>
#include <commons/config.h>


extern t_bitarray* bitmap;
extern char* blocks;
extern char* mapeo_bitmap_en_memoria;


void crear_bitmap();

struct bitmap
{
    char *direccion;
    uint32_t tamanio;
    t_bitarray *bitarray;
};

typedef struct bitmap t_bitmap;

typedef struct fcb {
    char* nombre_archivo;
    __uint32_t size;
    __uint32_t bloque_inicial;
} t_fcb;

#endif // !_MANEJO_DE_FILESYSTEM_H_