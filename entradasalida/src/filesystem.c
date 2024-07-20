#include <filesystem.h>
#include <entradasalida.h>
#include <fcntl.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <math.h>
#include <sys/mman.h>

t_fcb *fcb;
t_bitmap *bitmap;
t_log* log_fs;

int max(int a, int b){
    return (a > b) ? a : b;
}

void crear_archivo_bloques()

{   int tamanio_archivo_bloques = atoi(BLOCK_SIZE) * atoi(BLOCK_COUNT);

    int fd_bloques = open("bloques.dat",  O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); 

    if(fd_bloques == -1){
        log_error(log_fs , "El archivo de bloques no se creo correctamente");
    }

    if(ftruncate(fd_bloques, tamanio_archivo_bloques) == -1){
        log_error(log_fs, "error al truncar el archivo");
    }   

    close(fd_bloques);
}

void crear_archivo_bitmap()
{   
    // Abre el archivo para leer y escribir, lo crea si no existe,
    // seteandole permisos de lectura y escritura para el usuario.
   bitmap = malloc(sizeof(t_bitmap));
   
   
    int fd = open("bitmap.dat", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        log_error(log_fs, "El archivo de bitmap no se creo o abrió correctamente");
        abort(); //verificar
    }


    // Tamaño del bitmap en bytes.
    int tamanio_archivo = ceil((double)atoi(BLOCK_COUNT) / 8);
    bitmap->tamanio = tamanio_archivo;
    if (ftruncate(fd, tamanio_archivo) == -1)
    {
        log_error(log_fs, "Error al truncar el archivo");
        abort(); //verificar
    }

    bitmap->direccion = mmap(NULL, bitmap->tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(bitmap->direccion == MAP_FAILED)
    {
        log_error(log_fs, "Error al mapear el archivo");
        abort(); //verificar
    }
    
    bitmap->bitarray = bitarray_create_with_mode(bitmap->direccion, tamanio_archivo, LSB_FIRST);

    close(fd);
}

int bitmap_encontrar_bloque_libre(){
    int i;
    bool bloque;
    for( i = 0; i< atoi(BLOCK_COUNT); i++)
    {
        bloque = bitarray_test_bit(bitmap->bitarray,i);
        if(!bloque){
            return i;
            break;
        }
    }
    return -1;
}

void bitmap_marcar_bloque_libre(int numero_bloque)
{
    bitarray_clean_bit(bitmap->bitarray, numero_bloque);
    int i = msync(bitmap->direccion, bitmap->tamanio, MS_SYNC);
    log_info(log_fs, "Se liberó el bloque N° %d", numero_bloque);
    if( i == -1){
        log_error(log_fs, "Error al sincronizar los cambios");
    }
    return;
}

void bitmap_marcar_bloques_libres(int bloque_inicial, int bloque_final){
    for(int i = bloque_inicial; i<=bloque_final; i++){
        bitmap_marcar_bloque_libre(i);
    }
}

void bitmap_marcar_bloque_ocupado(int numero_bloque){
    bitarray_set_bit(bitmap->bitarray, numero_bloque);
    log_info(log_fs, "Se marco el bloque N° %d como ocupado", numero_bloque);
    int i = msync(bitmap->direccion, bitmap->tamanio, MS_SYNC);
    if( i == -1){
        log_error(log_fs, "Error al sincronizar los cambios");  
    }
}

void bitmap_marcar_bloques_ocupados(int bloque_inicial, int bloque_final){
    for(int i = bloque_inicial; i<=bloque_final; i++){
        bitmap_marcar_bloque_ocupado(i);
    }
    return;
}

int bitmap_encontrar_bloques_libres_continuos(int tamanio_archivo){
    int i, j;
    bool bloque_ocupado;
    int bloques_encontrados = 0;
    int cantidad_bloques_necesarios = bloques_necesarios(tamanio_archivo);

    for(i = 0; i <= atoi(BLOCK_COUNT) - cantidad_bloques_necesarios; i++){
        bloques_encontrados = 0;

        for(j = 0; j < cantidad_bloques_necesarios; j++){
            bloque_ocupado = bitarray_test_bit(bitmap->bitarray, i + j);

            if(bloque_ocupado){
                break;
            } else {
                bloques_encontrados++;
            }
        }
        if(bloques_encontrados == cantidad_bloques_necesarios){
            return i;
        }

    }
    log_error(log_fs, "No se encontraron bloques libres continuos");
    return -1;

}

int bloques_necesarios(int size){
    int cantidad_bloques = (int)ceil((double)size/ atoi(BLOCK_SIZE));
    return max(cantidad_bloques,1);
}

bool hay_espacio_contiguo(t_fcb* fcb, int bloques, int bloques_necesarios){
    int ultimo_bloque = fcb->BLOQUE_INICIAL + bloques;
    for(int i = ultimo_bloque; i < ultimo_bloque + bloques_necesarios; i++){
        bool estado = bitarray_test_bit(bitmap->bitarray, i);
          if(estado){
            return false;
          }
    
    return true;
} 
}


void crear_archivo_metadata(t_fcb *fcb){

    char* path = get_fullpath(fcb->nombre_archivo);

    log_trace(log_fs, "El path del archivo es: %s", path);

    int fd= open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(fd == -1)    {
        log_error(log_fs, "El archivo de metadata %s no se creo o abrió correctamente", fcb->nombre_archivo);
    }

    t_config* config_fcb = config_create(path);
    config_set_value(config_fcb, "NOMBRE_ARCHIVO", fcb->nombre_archivo);
    char* tam_arch = malloc(sizeof(uint32_t));
    config_set_value(config_fcb, "TAMANIO_ARCHIVO", tam_arch);
    sprintf(tam_arch, "%d", fcb->TAMANIO_ARCHIVO);
    char* bloque_inicial = malloc(sizeof(uint32_t));
    sprintf(bloque_inicial, "%d", fcb->BLOQUE_INICIAL);
    config_set_value(config_fcb, "BLOQUE_INICIAL", bloque_inicial); //CORREGIR
 
    config_save(config_fcb);

    free(tam_arch);
    free(bloque_inicial);
    free(path);
    config_destroy(config_fcb);

}

char* get_fullpath(char* nombre_archivo){
    char* fullpath = strdup(PATH_BASE_DIALFS);

    log_trace(log_fs,  "El Path base es: %s", fullpath);

    string_append(&fullpath, "/");
    string_append(&fullpath, nombre_archivo);

    return fullpath;
} 

t_fcb* leer_metadata(char* nombre_archivo){
    t_fcb *config_fcb = malloc(sizeof(t_fcb));

    char* path = get_fullpath(nombre_archivo);

    t_config* archivo_metadata = config_create(path);

    config_fcb->nombre_archivo = strdup(config_get_string_value(archivo_metadata, "NOMBRE_ARCHIVO"));
    config_fcb->BLOQUE_INICIAL = atoi(config_get_string_value(archivo_metadata, "BLOQUE_INICIAL"));
    config_fcb->TAMANIO_ARCHIVO = atoi(config_get_string_value(archivo_metadata, "TAMANIO_ARCHIVO"));

    config_save(archivo_metadata);
    config_destroy(archivo_metadata);
    free(path);

    return config_fcb;
}

void eliminar_archivo_metadata(int size){
    int cantidad_bloques = (int)ceil((double)size / atoi(BLOCK_SIZE));
    return max(cantidad_bloques, 1);
}

void borrar_archivo(char* nombre_archivo) {
    char* path = get_fullpath(nombre_archivo);

    if(remove(path) == 0){
        log_trace(log_fs, "El archivo %s fue eliminado", path);
    } else {
        log_error(log_fs, "El archivo %s no pudo ser eliminado", path);
    }

    free(path);
}

void marcar_bloques_libres(char* nombre_archivo){
    t_fcb* fcb = leer_metadata(nombre_archivo);

    int bloques_a_liberar = bloques_necesarios(fcb->TAMANIO_ARCHIVO);
    int bloque_inicial = fcb->BLOQUE_INICIAL;

    for(int i = 1; i<= bloques_a_liberar; i++){
        bitmap_marcar_bloque_libre(bloque_inicial);
        bloque_inicial++;
    };

    free(fcb);
}

void escribir_archivo(int bloque_inicial, int offset, void *dato, int size){
     int archivo_bloques = open("bloques.dat", O_RDWR);

     int offset_total = (bloque_inicial * atoi(BLOCK_SIZE)) + offset;
     lseek(archivo_bloques, offset_total, SEEK_SET);

     write(archivo_bloques, dato, size);

     free(dato);

     close(archivo_bloques);
}

char* leer_archivo(int tamanio, t_fcb *fcb, int offset){
    char *dato = malloc(tamanio);

    int archivo_bloques = open("bloques.dat", O_RDWR);

    lseek(archivo_bloques,(fcb->BLOQUE_INICIAL * atoi(BLOCK_SIZE)) + offset, SEEK_SET);
    ssize_t bytes_leidos = read(archivo_bloques, dato, tamanio);
    if(bytes_leidos == -1){
        log_error(log_fs, "Error al leer el archivo");
        free(dato);
        close(archivo_bloques);
    }

    close(archivo_bloques);
    return dato;
}

void agrandar_archivo(t_fcb *fcb, int tamanio_nuevo, int pid){
    int bloques_actuales = bloques_necesarios(fcb->TAMANIO_ARCHIVO);
    int bloques_finales = bloques_necesarios(tamanio_nuevo);
    int nuevo_inicial = 0;

    if(!hay_espacio_contiguo(fcb, bloques_actuales, bloques_finales - bloques_actuales)){
        log_info(log_fs, "Comienzo compatación para pid: %d", pid);
        nuevo_inicial = compactar_fcb(fcb);
        usleep(atoi(RETRASO_COMPACTACION) * 1000);
        log_info(log_fs, "Fin compactación para pid: %d", pid);
    }   

    for(int i = bloques_actuales; i < bloques_finales; i++){
        bitmap_marcar_bloque_ocupado(fcb->BLOQUE_INICIAL + i);
    }

    fcb->TAMANIO_ARCHIVO = tamanio_nuevo;
    fcb-> BLOQUE_INICIAL = nuevo_inicial;
    crear_archivo_metadata(fcb);
}


void achicar_archivo(t_fcb *fcb, int tamanio_nuevo){
    int bloques_actuales = bloques_necesarios(fcb->TAMANIO_ARCHIVO);
    int bloques_finales = bloques_necesarios(tamanio_nuevo);

    for(int i = bloques_actuales - 1; i>= bloques_finales; i--){
        bitmap_marcar_bloque_libre(fcb->BLOQUE_INICIAL + i);
    }
    fcb->TAMANIO_ARCHIVO = tamanio_nuevo;
    crear_archivo_metadata(fcb);

}

t_list* leer_directorio(){
    struct dirent *dir;
    t_list* lista_directorio = list_create();
    DIR *directorio = opendir(PATH_BASE_DIALFS);

    if(directorio){
        dir = readdir(directorio);
        while(dir != NULL){
            if(dir->d_type != DT_REG){
                dir = readdir(directorio);
                continue;
            }

            list_add(lista_directorio, leer_metadata(dir->d_name));

            dir = readdir(directorio);

        }

        closedir(directorio);
    }

    list_sort(lista_directorio, comparar_tamanios_fcbs);
    log_trace(io_logger, "Se leyeron los archivos del directorio correctamente");
   
    return lista_directorio;
}

bool comparar_tamanios_fcbs(void *e1, void* e2) {
    t_fcb *f1 = (t_fcb*) e1; 
    t_fcb *f2 = (t_fcb*) e2; 

    return f1->BLOQUE_INICIAL < f2->BLOQUE_INICIAL;
}


int compactar_fcb(t_fcb* fcb){
    t_list* lista_fcb = leer_directorio();
    t_fcb* fcb_a_cambiar = buscar_fcb(fcb, lista_fcb);

    int tamanio_a_agrandar = bloques_necesarios(fcb_a_cambiar->TAMANIO_ARCHIVO) * atoi(BLOCK_SIZE);
    char* contenido_a_agrandar = malloc(tamanio_a_agrandar);
    //revisar
    contenido_a_agrandar = buscar_contenido_fcb(fcb_a_cambiar);

    bitmap_marcar_bloques_libres(fcb->BLOQUE_INICIAL, max(bloques_necesarios(fcb_a_cambiar->TAMANIO_ARCHIVO) + fcb_a_cambiar->BLOQUE_INICIAL -1,0));

    for(int i = 0; i < list_size(lista_fcb); i++){
        t_fcb* fcb_actual = list_get(lista_fcb, i);
        if(fcb_actual->BLOQUE_INICIAL == fcb_a_cambiar->BLOQUE_INICIAL){
            continue;
        }
        int bloque_inicial = fcb_actual->BLOQUE_INICIAL;
        int bloque_final = max(bloques_necesarios(fcb_actual->TAMANIO_ARCHIVO) + bloque_inicial - 1, 0);
        int tamanio = bloques_necesarios(fcb_actual->TAMANIO_ARCHIVO) * atoi(BLOCK_SIZE);

        char* contenido_bloques;
        contenido_bloques = buscar_contenido_fcb(fcb_actual);

        bitmap_marcar_bloques_libres(bloque_inicial, bloque_final);
        int nuevo_bloque_inicial = copiar_contenido_a(contenido_bloques, tamanio);
        fcb_actual->BLOQUE_INICIAL = nuevo_bloque_inicial;

        crear_archivo_metadata(fcb_actual);
        free(fcb_actual->nombre_archivo);
        free(fcb_actual);
    }

    int archivo_nuevo_bloque_inicial = copiar_contenido_a(contenido_a_agrandar, tamanio_a_agrandar);
    list_destroy(lista_fcb);
    return archivo_nuevo_bloque_inicial;


}

t_fcb* buscar_fcb(t_fcb* fcb1, t_list* lista_fcbs){
    bool comparar_fcb(void *fcb){
        t_fcb *fcb_a_comparar = (t_fcb *) fcb;
        return fcb_a_comparar->BLOQUE_INICIAL == fcb1->BLOQUE_INICIAL;
    }

    if(lista_fcbs == NULL){
        log_error(io_logger, "La lista de fcbs es nula");
        return NULL;
    }

    t_fcb *fcb_encontrado = (t_fcb *)list_find(lista_fcbs, comparar_fcb);

    return fcb_encontrado;
}


char* leer_bloques(int bloque_inicial, int tamanio){
    char *dato= malloc(tamanio);

    int file_bloques_dat = open("bloques.dat", O_RDWR);
    int offset = bloque_inicial * atoi(BLOCK_SIZE);

    lseek(file_bloques_dat, offset, SEEK_SET);
    ssize_t bytes_leidos = read(file_bloques_dat, dato, tamanio);
    if(bytes_leidos == -1){
        perror("Error al leer el archivo");
        close(file_bloques_dat);
    }

    close(file_bloques_dat);

    return dato;
}


char* buscar_contenido_fcb(t_fcb* fcb){
    int bloque_inicial = fcb->BLOQUE_INICIAL;
    int bloques_a_leer = bloques_necesarios(fcb->TAMANIO_ARCHIVO);
    int tamanio_lectura = bloques_a_leer * atoi(BLOCK_SIZE);

    char* contenido = malloc(tamanio_lectura);

    contenido = leer_bloques(bloque_inicial, tamanio_lectura);
    return contenido;
}


int copiar_contenido_a(char* contenido, int tamanio){
    int bloque_inicial = bitmap_encontrar_bloque_libre();
    log_trace(io_logger, "Se encontró el bloque N° %d", bloque_inicial);

    escribir_archivo(bloque_inicial, 0, contenido, tamanio);

    if(bloques_necesarios(tamanio) == 1){
        bitmap_marcar_bloque_ocupado(bloque_inicial);
    } else {
        bitmap_marcar_bloques_ocupados(bloque_inicial, bloques_necesarios(tamanio));
    }

    return bloque_inicial;

}

void crear_bitmap(){
    bitmap = malloc(sizeof(t_bitmap));
    log_fs = iniciar_logger("filesystem.log", "Filesystem");
    log_info(log_fs, "Se inicio el logger de filesystem");

    int fd_bitmap = open("bitmap.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bitmap == -1){
        log_error(io_logger, "Error al abrir el archivo bitmap");
    }

    bitmap->tamanio = (atoi(BLOCK_COUNT) / 8);
    
    if (ftruncate(fd_bitmap, bitmap->tamanio) == -1) {
        log_error(io_logger, "Error al truncar el archivo Bitmap");
    }

    bitmap->direccion = mmap(NULL, bitmap->tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    if (bitmap->direccion == MAP_FAILED) {
        log_error(io_logger, "Error al mapear el Bitmap");
    }

    bitmap->bitarray = bitarray_create_with_mode(bitmap->direccion, bitmap->tamanio, LSB_FIRST);

    close(fd_bitmap);
}








