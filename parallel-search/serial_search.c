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
  int i = 0;    // iterator
  int N = 0;    // number of elements in array
  int *A;       // array to search
  int tmp = 0;  // temporary result
/****************/

  // read A.dat
  read(&A, &N);

  wT = omp_get_wtime();
  for(j = 0; j < repeat; j++) {
/**** SEARCH ****/
    for(i = 0; i < N; i++) {      // loop over all elements
      if(A[i] > tmp) tmp = A[i];  // check if current is greatest so far
    }
/****************/
  }
  wT = omp_get_wtime() - wT;

  printf("\nSerial: searching A %d times...", repeat);
  fflush(stdout);
  printf("done\n        found A_max = %d in %f seconds.\n", tmp, wT);
  fflush(stdout);

  // clean up
  free(A);

  return EXIT_SUCCESS;
}
