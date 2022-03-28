#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <pthread.h>
#include <omp.h>
#include <fcntl.h>
#include <stdio.h>


int columna;
int fila;
char sudoku[9][9];


int cols(){   

    omp_set_nested(1);
    omp_set_num_threads(9);
    int grid[9];
    int v = 0;
    int i = 0;

    #pragma omp parallel for private(grid) schedule(dynamic)
    for (i = 0; i < 9; i++){
        char nums[] = "123456789";
        char *num;
        for (num = &nums[0]; *num != '\0'; num++){
            int nn = 0;
            int j = 0;
            while (nn == 0 && j < 9){
                if (sudoku[j][i] == *num)
                    nn = 1;
                j++;
            }
            if (nn == 0)
                v = -1;
        }
        printf("En la verificacion de las columnas el thread en ejecucion es: %ld \n", syscall(SYS_gettid));
    }
    return v;
}


int filas(){

    omp_set_nested(1);
    omp_set_num_threads(9);
    int grid[9];
    int v = 0;
    int i = 0;
   #pragma omp parallel for private(grid) schedule(dynamic)
    for (i = 0; i < 9; i++){
        char nums[] = "123456789";
        char *num;
        for (num = &nums[0]; *num != '\0'; num++){
            int nn = 0;
            int j = 0;
            while (nn == 0 && j < 9){
                if (sudoku[i][j] == *num)
                    nn = 1;
                j++;
            }
            if (nn == 0)
                v = -1;
        }
        printf("En la verificacion de las filas el thread en ejecucion es: %ld \n", syscall(SYS_gettid));
    }
    return v;
}






int filanums(char t[9][9]){

    omp_set_nested(1);
    omp_set_num_threads(9);
    int grid[9];
    int v = 0;
    int i = 0;
    #pragma omp parallel for private(grid) schedule(dynamic)
    for (i = 0; i < 9; i++){
        char nums[] = "123456789";
        char *num;
        for (num = &nums[0]; *num != '\0'; num++){
            int nn = 0;
            int j = 0;
            while (nn == 0 && j < 9){
                if (t[i][j] == *num)
                    nn = 1;
                j++;
            }
            if (nn == 0)
                v = -1;
        }
    }
    return v;
}



int ver3x3(){

    omp_set_nested(1);
    omp_set_num_threads(9);
    char t[9][9];
    int row = 0, column = 0;
    int grid[9];
    int x, y, i, j = 0;
    
    #pragma omp parallel for private(grid) schedule(dynamic)
    for (x = 0; x < 3; x++){   
        for (y = 0; y < 3; y++){           
            for (i = 0; i < 3; i++){              
                for (j = 0; j < 3; j++){
                    t[9][9] = sudoku[i + (x * 3)][j + (y * 3)];
                    column++;
                }
            }
            column = 0;
            row++;
        }
    }
    return filanums(t);
}

void *colver(){
    printf("El thread que ejecuta el metodo para ejecutar el metodo de revision de columnas es: %ld \n", syscall(SYS_gettid));
    columna = cols();
    pthread_exit(0);
}

void *filver(){
    printf("El thread que ejecuta el metodo para ejecutar el metodo de revision de filas es: %ld \n", syscall(SYS_gettid));
    fila = filas();
    pthread_exit(0);
}



void mapear(int name){
    omp_set_nested(1);
    omp_set_num_threads(9);
    struct stat stat_s;
    int sudokustatus = fstat(name, &stat_s);
    int size = stat_s.st_size;
    char *ptr = (char *)mmap(0, size, PROT_READ, MAP_PRIVATE, name, 0);
    int cp = 0;
    int grid[9];
    int i, j = 0;
    #pragma omp parallel for private(grid) schedule(dynamic)
    for (i = 0; i < 9; i++){
        for (j = 0; j < 9; j++){
            sudoku[i][j] = ptr[cp];
            cp++;
        }
    }
    munmap(ptr,size);
    close(name);
}

int main(int argc, char *argv[]){

    
    omp_set_num_threads(1);
    
    if (argc < 2){
        printf("Archivo ingresado incorrectamente. \n");
        return 1;
    }

    int input;
  
    if ((input = open(argv[1], O_RDONLY)) < 0){
        perror("Error al abrir sudoku \n");
        return 1;
    }
    
    else{
        mapear(input);
        pid_t padre = getpid();
        int hijo = fork();
        if (hijo < 0){
            perror("Error de fork.");
            return 1;
        }
        else if (hijo == 0){

            char pp[6];
            sprintf(pp, "%d", (int)padre);
            execlp("ps", "ps", "-p", pp, "-lLf", NULL);
        }
        else{
        
            pthread_t cv;
          
            if (pthread_create(&cv, NULL, colver, NULL)){
                perror("Error de creacion de thread");
                return 1;
            }
            if (pthread_join(cv, NULL)){
                perror("Error de join thread.");
                return 1;
            }

            printf("Hijo es : %ld \n", syscall(SYS_gettid));
            usleep(30000);
            printf("Hijo finalizado \n");


            pthread_t rv;
            if (pthread_create(&rv, NULL, filver, NULL)){
                perror("Error al crear el Hilo");
                return 1;
            }
            if (pthread_join(rv, NULL)){
                perror("Error de join thread.");
                return 1;
            }


            if (fila == 0 && columna == 0){
                printf("SOLUCION CORRECTA!\n");
            }
            else{
                printf("SOLUCION INCORRECTA :(\n");
            }

        int hijito = fork();
        if (hijito == 0){ 
            
            char pp[6];
            sprintf(pp, "%d", (int)padre);
            execlp("ps", "ps", "-p", pp, "-lLf", NULL);
        }
        else{
            usleep(30000);
            printf("Hijo finalizado\n");
            return 0;
        }
    }

}
}
