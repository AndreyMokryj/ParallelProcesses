#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MATRIX_SIZE 1000


int createRandomMatrix(int startPosition, int rows, int cols, double (*a))
{
    for(size_t i = 0; i < rows; ++i)
    {
        for(size_t j = 0; j < cols; ++j) {
            double value = 1.0 * (rand() % 10) - rand() % 10;
            a[startPosition + i * cols + j] = value;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
    int totalNumber = MATRIX_SIZE * MATRIX_SIZE * 2;
    srand(time(NULL));
    double *send;
    send = (double*)malloc(totalNumber * sizeof(double));
    double *res;
    res = (double*)malloc(totalNumber / 2 * sizeof(double));
  int rank, size, root;

  MPI_Init(&argc, &argv); // initialize MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get own rank/ID
  MPI_Comm_size(MPI_COMM_WORLD, &size); // get total number of processes

  if(rank == 0) {
    printf("\nMatrix size: %d \n", MATRIX_SIZE);
    createRandomMatrix(0, MATRIX_SIZE, MATRIX_SIZE, send);
    createRandomMatrix(MATRIX_SIZE * MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE, send);
  }

    time_t startTime = time(NULL);
    MPI_Bcast(send, totalNumber, MPI_DOUBLE, 0, MPI_COMM_WORLD);
//    printf("Size of sent buffer: %d", number);
    
    
  // each node now processes its share of the numbers
//    printf("\nProcess %d got following data\n", rank);
//    for(int i = 0; i < number; i++) {
//        printf("%-3.2f ", send[i]);
//    }
//    printf("\n");
    
    double xRows = 1.0 * MATRIX_SIZE / size;
    int startRow = floor(rank * xRows);
    int endRow = MATRIX_SIZE;
    if (rank < size - 1) {
        endRow = floor((rank + 1) * xRows);
    }
    
//    printf("Process %d start row %d end row %d", rank, startRow, endRow);
    double *result;
    result = (double*)malloc(totalNumber / 2 * sizeof(double));
    int rIndex = 0;
    for(int r = startRow; r < endRow; r++) {
        for(int i = 0; i < MATRIX_SIZE; i++){
            result[rIndex] = 0;
            for(int j = 0; j < MATRIX_SIZE; j++){
                result[rIndex] += send[r * MATRIX_SIZE + j] * send[MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE * j + i];
            }
            rIndex++;
        }
    }
        
//    printf("\nProcess %d got following results\n", rank);
//    for(int i = 0; i < rIndex; i++) {
//        printf("%-3.2f ", result[i]);
//    }
//    printf("\n");

    int counts[size];
    int displacements[size];
    for(int i=0; i<size; i++){
        int startRow = floor(i * xRows);
        int endRow = MATRIX_SIZE;
        if (i < size - 1) {
            endRow = floor((i + 1) * xRows);
        }
        counts[i] = MATRIX_SIZE * (endRow - startRow);
        displacements[i] = MATRIX_SIZE * startRow;
    }
    MPI_Gatherv(result, MATRIX_SIZE * (endRow - startRow), MPI_DOUBLE, res, counts, displacements, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if(rank == 0) { //if root, print
        time_t endTime = time(NULL);
//        printf("\nResult: \n");
//        for (int i = 0; i < SIZE * SIZE; i++) {
//            printf("%-3.2f ", res[i]);
//        }
//        printf("\n");
        free(send);
        long long int st = (long long int)(startTime);
        long long int et = (long long int)(endTime);
        long long int diff = et - st;
        printf("Time taken: %lld seconds\n", diff);
    }

    free(res);
    free(result);
    
  MPI_Finalize(); // shut down MPI
  
    
//    free(send);
//    free(send);
//    free(send);

    return 0;
}
