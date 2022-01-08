#include <mpi.h>    // import MPI header
#include <stdio.h>  // needed for printf
#include <math.h>   // needed for sqrt
#include <stdlib.h>
#include <string.h>

#define DATA_SIZE 1024 // let's count the primes among the first 1024 numbers
#define SIZE 3


int readmatrixByRows(int rows, int cols, int (*a), const char* filename)
{

    FILE *pf;
    pf = fopen (filename, "r");
    if (pf == NULL)
        return 0;

    int value;

    for(size_t i = 0; i < rows; ++i)
    {
        for(size_t j = 0; j < cols; ++j) {
            fscanf(pf, "%d", &value);
            a[i * cols + j] = value;
        }
    }


    fclose (pf); 
    return 1; 
}

int readmatrixByCols(int rows, int cols, int (*a), const char* filename)
{
    
    FILE *pf;
    pf = fopen (filename, "r");
    if (pf == NULL)
        return 0;
    
    int value;
    
    for(size_t i = 0; i < rows; ++i)
    {   
        for(size_t j = 0; j < cols; ++j) {
            fscanf(pf, "%d", &value);
            a[rows * cols + i * cols + j] = value;
        }
    }

    
    fclose (pf);
    return 1;
}


int main(int argc, char *argv[]) {
  int first[DATA_SIZE], second[DATA_SIZE];
  int send[DATA_SIZE], recv[DATA_SIZE];
  int rank, size, count, root, res[SIZE], i, j;
  int rows_count, cols_count;
  MPI_Status status;

  MPI_Init(&argc, &argv); // initialize MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get own rank/ID
  MPI_Comm_size(MPI_COMM_WORLD, &size); // get total number of processes

//  MPI_Datatype col;
//  MPI_Type_vector(SIZE, 2, SIZE*2, MPI_INT, &col);
//  MPI_Type_commit(&col);

  MPI_Datatype row;
  MPI_Type_vector(1, SIZE, SIZE, MPI_INT, &row);
  MPI_Type_commit(&row);

  if(rank == 0) {
    puts(" -- start reading 1st matrix");
    readmatrixByRows(SIZE, SIZE, send, "first.dat");
    puts(" -- start reading 2nd matrix");
    readmatrixByCols(SIZE, SIZE, send, "second.dat");

//    for(i = 0; i < SIZE * SIZE * 2; i++) {
//        send[i] = first[i];
//        send[SIZE * SIZE + i] = second[i];
//    }
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

    
//  res = count; //here: count how many prime numbers are contained in the array
//  for(i = count; (--i) >= 0; ) { //j: test all odd numbers 1<j<sqrt(j)|1
//    for(j = ((int)(sqrt(recv[i]))|1); j>1; j -= 2) {
//      if((recv[i] % j) == 0) { // if a number can be divided by j
//        res--; // it cannot be a prime number, reduce number of primes
//        break; } // break inner loop to test next number
//    }
//  }
//  printf("Process %d discovered %d primes in the numbers from %d to %d.\n", rank, res, recv[0], recv[count-1]);

  // reduce: each node takes results, applies operator MPI_SUM locally, sends result to root, where MPI_SUM is
  // applied again. (here: locally summing up does not matter, as only 1 number). The final result is returned.
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

