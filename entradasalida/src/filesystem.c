#include "filesystem.h"

void generar_archivo_bloques(){
    int tamanio_archivo_bloques = config->block_size  * config->block_count;

    int fd_bloques = open("bloques.dat",  O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); 

    if(fd_bloques == -1){
        //cambiar log
        log_error(log_filesystem , "El archivo de bloques no se creo correctamente");
    }

    if(ftruncate(fd_bloques, tamanio_archivo_bloques) == -1){
        log_error(log_filesystem, "error al truncar el archivo");
    }   

    close(fd_bloques);
}

void crear_bitmap(){
    bitmap = malloc(sizeof(t_bitmap));

    int fd_bitmap = open("bitmap.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bitmap == -1){
        log_error(log_filesystem, "Error al abrir el archivo bitmap");
    }

    bitmap->tamanio = (config->block_count / 8);
    
    if (ftruncate(fd_bitmap, bitmap->tamanio) == -1) {
        log_error(log_filesystem, "Error al truncar el archivo Bitmap");
    }

    bitmap->direccion = mmap(NULL, bitmap->tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    if (bitmap->direccion == MAP_FAILED) {
        log_error(log_filesystem, "Error al mapear el Bitmap");
    }

    bitmap->bitarray = bitarray_create_with_mode(bitmap->direccion, bitmap->tamanio, LSB_FIRST);

    close(fd_bitmap);

}

void bitmap_marcar_bloque_libre(int numero_bloque){

    bitarray_clean_bit(bitmap->bitarray, numero_bloque);
    int i = msync(bitmap->direccion, bitmap->tamanio, MS_SYNC);
    log_trace(log_filesystem, "Se marco el bloque libre %i", numero_bloque);
    if ( i  == -1) {
        log_error(log_filesystem, "Error al sincronizar los cambios en el Bitmap");
    }
    return;
}

void bitmap_marcar_bloque_ocupado(int numero_bloque){

    bitarray_set_bit(bitmap->bitarray, numero_bloque);
    log_trace(log_filesystem, "Se marco el bloque ocupado %i", numero_bloque);
    if (msync(bitmap->direccion, bitmap->tamanio, MS_SYNC) == -1) {
        log_error(log_filesystem, "Error al sincronizar los cambios en el Bitmap");
    }
}