#include "filesystem.h"
#include "entradasalida.h"
#include "entradasalida.h"
#include <fcntl.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <math.h>
#include <sys/mman.h>

void crear_bitmap()
{
    // Tamaño del bitmap en bytes.
    // Un bit por bloque. Por lo tanto 8 bloques por byte.
    bitmap = malloc(ceil((double)BLOCK_COUNT / 8));

    bitarray = bitarray_create_with_mode(bitmap, ceil((double)BLOCK_COUNT / 8), LSB_FIRST);

    // Por las dudas para que arranquen todos los bits en cero.
    for (int i = 0; i < atoi(BLOCK_COUNT); i++)
    {
        bitarray_clean_bit(bitarray, i);
    }
}

void crear_archivo_bloques()
{
    // Abre el archivo para leer y escribir, lo crea si no existe,
    // seteandole permisos de lectura y escritura para el usuario.
    int fd = open("bloques.dat", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        log_error(logger, "El archivo de bloques no se creo o abrió correctamente");
        abort();
    }

    // Tamaño del archivo en bytes.
    int tamanio_archivo = BLOCK_SIZE * BLOCK_COUNT;
    if (ftruncate(fd, tamanio_archivo) == -1)
    {
        log_error(logger, "Error al truncar el archivo");
        abort();
    }

    close(fd);
}

void crear_archivo_bitmap()
{
    // Abre el archivo para leer y escribir, lo crea si no existe,
    // seteandole permisos de lectura y escritura para el usuario.
    int fd = open("bitmap.dat", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        log_error(logger, "El archivo de bitmap no se creo o abrió correctamente");
        abort();
    }

    // Tamaño del bitmap en bytes.
    int tamanio_archivo = ceil((double)atoi(BLOCK_COUNT) / 8);
    if (ftruncate(fd, tamanio_archivo) == -1)
    {
        log_error(logger, "Error al truncar el archivo");
        abort();
    }

    close(fd);
}

void crear_archivo_metadata(char* nombre) {
    // Abre el archivo para leer y escribir, lo crea si no existe,
    // seteandole permisos de lectura y escritura para el usuario.
    int fd = open(nombre, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        log_error(logger, "El archivo de metadata no se creo o abrió correctamente");
        abort();
    }
    close(fd);
}

void borrar_archivo(char* nombre) {
    // Abre el archivo para leer y escribir, lo crea si no existe,
    // seteandole permisos de lectura y escritura para el usuario.
    int fd = open(nombre, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        log_error(logger, "El archivo de metadata no se creo o abrió correctamente");
        abort();
    }
    close(fd);
}

void bitmap_marcar_bloque_libre(int numero_bloque)
{
    int bitmap_size = (ceil((double)BLOCK_COUNT / 8)); // Tamaño en bytes
    bitarray_clean_bit(bitarray, numero_bloque);       // Marca el bloque como libre
    int sincro = msync(bitmap, bitmap_size, MS_SYNC);  // Se encarga de sincronizar los cambios
    log_info(logger, "Se liberó el bloque N° %d", numero_bloque);
    if (sincro == -1) // Verifico que los cambios se hayan sincronizado correctamente
    { 
        log_error(logger, "Error al sincronizar los cambios");
    }
}

void bitmap_marcar_bloque_ocupado(int numero_bloque)
{
    int bitmap_size = (ceil((double)BLOCK_COUNT / 8));
    int sincro = bitarray_set_bit(bitarray, numero_bloque);
    log_info(logger, "Se ocupó el bloque N° %d", numero_bloque);
    if (sincro == -1) // Verifico que los cambios se hayan sincronizado correctamente
    { 
        log_error(logger, "Error al sincronizar los cambios");
    }
}

void encontrar_bloque
