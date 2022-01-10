#include <mpi.h>    // import MPI header
#include <stdio.h>  // needed for printf
#include <math.h>   // needed for sqrt
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 2


int createRandomMatrix(int startPosition, int rows, int cols, int (*a))
{
    for(size_t i = 0; i < rows; ++i)
    {
        for(size_t j = 0; j < cols; ++j) {
            int value = rand() % 10 - rand() % 10;
            a[startPosition + i * cols + j] = value;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int send[SIZE * SIZE * 2];
  int rank, size, root, res[SIZE];
  MPI_Status status;

  MPI_Init(&argc, &argv); // initialize MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get own rank/ID
  MPI_Comm_size(MPI_COMM_WORLD, &size); // get total number of processes

  if(rank == 0) {
    puts(" -- start creating 1st matrix");
    createRandomMatrix(0, SIZE, SIZE, send);
    puts(" -- start creating 2nd matrix");
    createRandomMatrix(SIZE * SIZE, SIZE, SIZE, send);
  }

    int number = SIZE * SIZE * 2;
    MPI_Bcast(send, number, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Size of sent buffer: %d", number);
    
    
  // each node now processes its share of the numbers
    printf("\nProcess %d got following data\n", rank);
    for(int i = 0; i < number; i++) {
        printf("%-3d ", send[i]);
    }
    printf("\n");
    
    double xRows = 1.0 * SIZE / size;
    int startRow = floor(rank * xRows);
    int endRow = SIZE;
    if (rank < size - 1) {
        endRow = floor((rank + 1) * xRows);
    }
    
    printf("Process %d start row %d end row %d", rank, startRow, endRow);
    int result[number];
    int rIndex = 0;
    for(int r = startRow; r < endRow; r++) {
        for(int i = 0; i < SIZE; i++){
            result[rIndex] = 0;
            for(int j = 0; j < SIZE; j++){
                result[rIndex] += send[r * SIZE + j] * send[SIZE * SIZE + SIZE * j + i];
            }
            rIndex++;
        }
    }
        
    printf("\nProcess %d got following results\n", rank);
    for(int i = 0; i < rIndex; i++) {
        printf("%-3d ", result[i]);
    }
    printf("\n");

    int counts[size];
    int displacements[size];
    for(int i=0; i<size; i++){
        int startRow = floor(i * xRows);
        int endRow = SIZE;
        if (i < size - 1) {
            endRow = floor((i + 1) * xRows);
        }
        counts[i] = SIZE * (endRow - startRow);
        displacements[i] = SIZE * startRow;
    }
    MPI_Gatherv(&result, SIZE * (endRow - startRow), MPI_INT, res, counts, displacements, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank == 0) { //if root, print
        printf("\nResult: \n");
        for (int i = 0; i < SIZE * SIZE; i++) {
            printf("%-3d ", res[i]);
        }
        printf("\n");
    }

  MPI_Finalize(); // shut down MPI
  return 0;
}
