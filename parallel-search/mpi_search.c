#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include <mpi.h>

int main(int argc, char *argv[])
{
  int j;              // iterator
  int repeat = 100;   // number of times to repeat search
  double wT = 0;      // wall time
  int scan = 0;       // for reading

/**** SEARCH ****/
  int np = 0;         // the total number of processors
  int rank = 0;       // the number of this processor
  MPI_Status status;  // status of MPI functions
  int MASTER = 0;     // keep track of the master node
  int i = 0;          // iterator
  int N = 0;          // number of elements in array
  int split = 0;      // size of chunk of A for each processor
  int *A;             // array to search
  int *_A;            // array local to each processor
  int *tmp;           // result of chunk for each processor
  int _tmp = 0;       // temporary result for each processor
  int result = 0;     // final result

  MPI_Init(&argc, &argv);               // start MPI
  MPI_Comm_size(MPI_COMM_WORLD, &np);   // get total number of processors
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // get number of this processor
/****************/

  // read file
  if(rank == MASTER) {
    // read
    printf("Reading A.dat...");
    fflush(stdout);
    FILE *ifile = fopen("A.dat", "r");
    scan = fscanf(ifile, "%d\n", &N);
    A = (int*) malloc(N * sizeof(int));
    for(i = 0; i < N; i++) {
      scan = fscanf(ifile, "%d\n", &A[i]);
    }
    printf("done\n");
    fflush(stdout);
  }

/**** SEARCH ****/
  if(rank == MASTER) {
    split = N / np;   // calculate size of chunks

    // send N and split to other processors
    for(i = 1; i < np; i++) {
      MPI_Send(&N, 1, MPI_INT, i, i+42, MPI_COMM_WORLD);
      MPI_Send(&split, 1, MPI_INT, i, i, MPI_COMM_WORLD);
    }

  } else {
    // receive N and split from MASTER
    MPI_Recv(&N, 1, MPI_INT, MASTER, rank+42, MPI_COMM_WORLD, &status);
    MPI_Recv(&split, 1, MPI_INT, MASTER, rank, MPI_COMM_WORLD, &status);
  }
/****************/

  if(rank == MASTER) {
    wT = omp_get_wtime();
  }

  for(j = 0; j < repeat; j++) {
/**** SEARCH ****/
    if(rank == MASTER) {
      // allocate space for results of each chunk
      tmp = (int*) malloc(np * sizeof(int));

      // send chunks to other processors
      for(i = 1; i < np; i++) {
        MPI_Send(&(A[i*split]), split, MPI_INT, i, i, MPI_COMM_WORLD);
      }

      _tmp = 0;       // initialize tmp value for this chunk
      for(i = 0; i < split; i++) {    // loop over this processor's chunk
        if(A[i] > _tmp) _tmp = A[i];  // check if current is greatest so far
      }
      
      // receive results from other processors
      tmp[MASTER] = _tmp;
      for(i = 1; i < np; i++) {
        MPI_Recv(&tmp[i], 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
      }

      // finish searching over all chunk results
      for(i = 0; i < np; i++) {    // search for max of chunk max
        if(tmp[i] > result) result = tmp[i];
      }

      // clean up temporary space
      free(tmp);
    } else {
      // receive chunks from MASTER
      _A = (int*) malloc(split * sizeof(int));
      MPI_Recv(_A, split, MPI_INT, MASTER, rank, MPI_COMM_WORLD, &status);

      _tmp = 0;       // initialize tmp value for this chunk
      for(i = 0; i < split; i++) {    // loop over this processor's chunk
        if(_A[i] > _tmp) _tmp = _A[i];  // check if current is greatest so far
      }

      // copy result of chunk back to MASTER
      MPI_Send(&_tmp, 1, MPI_INT, MASTER, rank, MPI_COMM_WORLD);

      // clean up temporary space
      free(_A);
    }
/****************/
  }

  if(rank == MASTER) {
    wT = omp_get_wtime() - wT;

    printf("\nMPI:    searching A %d times...", repeat);
    fflush(stdout);
    printf("\n        found A_max = %d in %f seconds.\n", result, wT);
    fflush(stdout);

    // clean up
    free(A);
  }

/**** SEARCH ****/
  MPI_Finalize();   // end MPI
/****************/

  return EXIT_SUCCESS;
}
