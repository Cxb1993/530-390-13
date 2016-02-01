#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void read(int **A, int *N) {
  int i = 0;
  int scan = 0;
  printf("Reading A.dat...");
  fflush(stdout);
  FILE *ifile = fopen("A.dat", "r");
  scan = fscanf(ifile, "%d\n", N);
  *A = (int*) malloc(*N * sizeof(int));
  for(i = 0; i < *N; i++) {
    scan = fscanf(ifile, "%d\n", &(*A)[i]);
  } 
  fclose(ifile);
  printf("done\n");
  fflush(stdout);
} 

int main(int argc, char *argv[])
{
  int j = 0;        // iterator
  int repeat = 100; // number of times to repeat search
  double wT = 0;    // wall time

/**** SEARCH ****/
  int i = 0;      // iterator
  int N = 0;      // number of elements in array
  int *A;         // array to search
  int np = 0;     // number of processors
  int *tmp;       // temporary result for each processor
  int result = 0; // final result
/****************/

  // parse command line arguments to find np
  if(argc < 2) {
    printf("search usage: search np\n");
    return EXIT_FAILURE;
  } else {
    np = atoi(argv[1]);
  }

  // read A.dat
  read(&A, &N);

  wT = omp_get_wtime();
  for(j = 0; j < repeat; j++) {
/**** SEARCH ****/
    tmp = (int*) malloc(np * sizeof(int));   // allocate tmp for each processor
    #pragma omp parallel num_threads(np)      // launch omp threading
    {
      int tid = omp_get_thread_num();         // thread id = this thread
      int i = 0;                              // iterator
      int split = N / np;                     // size of chunk of A for thread
      int _tmp = 0;                           // temporary result for thread
      for(i = tid*split; i < (tid+1)*split; i++) {  // loop over thread's chunk
        if(A[i] > _tmp) _tmp = A[i];      // check if current is greatest so far
      }
      tmp[tid] = _tmp;                        // copy back result of chunk
    }
    for(i = 0; i < np; i++) {                 // search for max of chunk max
      if(tmp[i] > result) result = tmp[i];
    }
    free(tmp);                                // clean up allocated tmp space
/****************/
  }
  wT = omp_get_wtime() - wT;
  printf("\nOpenMP: searching A %d times...", repeat);
  fflush(stdout);
  printf("done\n        found A_max = %d in %f seconds.\n", result, wT);
  fflush(stdout);

  // clean up
  free(A);

  return EXIT_SUCCESS;
}
