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

// Method that determines if numbers 1-9 only appear once in a column
void *isColumnValid(void* param) {
	// Confirm that parameters indicate a valid col subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;		
	if (row != 0 || col > 8) {
		fprintf(stderr, "Invalid row or column for col subsection! row=%d, col=%d\n", row, col);
		pthread_exit(NULL);
	}

	// Check if numbers 1-9 only appear once in the column
	int validityArray[9] = {0};
	int i;	
	for (i = 0; i < 9; i++) {
		int num = sudoku[i][col];
		if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
			pthread_exit(NULL);
		} else {
			validityArray[num - 1] = 1;		
		}
	}
	// If reached this point, col subsection is valid.
	valid[18 + col] = 1;
	pthread_exit(NULL);
}

// Method that determines if numbers 1-9 only appear once in a row
void *isRowValid(void* param) {
	// Confirm that parameters indicate a valid row subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;		
	if (col != 0 || row > 8) {
		fprintf(stderr, "Invalid row or column for row subsection! row=%d, col=%d\n", row, col);
		pthread_exit(NULL);
	}

	// Check if numbers 1-9 only appear once in the row
	int validityArray[9] = {0};
	int i;
	for (i = 0; i < 9; i++) {
		// If the corresponding index for the number is set to 1, and the number is encountered again,
		// the valid array will not be updated and the thread will exit.
		int num = sudoku[row][i];
		if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
			pthread_exit(NULL);
		} else {
			validityArray[num - 1] = 1;		
		}
	}
	// If reached this point, row subsection is valid.
	valid[9 + row] = 1;
	pthread_exit(NULL);
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

// Confirm that parameters indicate a valid 3x3 subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;		
	if (row > 6 || row % 3 != 0 || col > 6 || col % 3 != 0) {
		fprintf(stderr, "Invalid row or column for subsection! row=%d, col=%d\n", row, col);
		pthread_exit(NULL);
	}
	int validityArray[9] = {0};
	int i, j;
	for (i = row; i < row + 3; i++) {
		for (j = col; j < col + 3; j++) {
			int num = sudoku[i][j];
			if (num < 1 || num > 9 || validityArray[num - 1] == 1) {
				pthread_exit(NULL);
			} else {
				validityArray[num - 1] = 1;		
			}
		}
	}
	// If reached this point, 3x3 subsection is valid.
	valid[row + col/3] = 1; // Maps the subsection to an index in the first 8 indices of the valid array
	pthread_exit(NULL);


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

    pthread_t threads[num_threads];
	
	int threadIndex = 0;	
	int i,j;

	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {						
			if (i%3 == 0 && j%3 == 0) {
				parameters *data = (parameters *) malloc(sizeof(parameters));	
				data->row = i;		
				data->column = j;
				pthread_create(&threads[threadIndex++], NULL, is3x3Valid, data); // 3x3 subsection threads
			}
			if (i == 0) {
				parameters *columnData = (parameters *) malloc(sizeof(parameters));	
				columnData->row = i;		
				columnData->column = j;
				pthread_create(&threads[threadIndex++], NULL, isColumnValid, columnData);	// column threads
			}
			if (j == 0) {
				parameters *rowData = (parameters *) malloc(sizeof(parameters));	
				rowData->row = i;		
				rowData->column = j;
				pthread_create(&threads[threadIndex++], NULL, isRowValid, rowData); // row threads
			}
		}
	}

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
