#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int N = 0;
  int i = 0;
  // parse command line arguments to find N
  if(argc < 2) {
    printf("search usage: search N\n");
    return EXIT_FAILURE;
  } else {
    N = atoi(argv[1]);
  }

  FILE *ofile = fopen("A.dat", "w+");

  int tmp = 0;
  int A = 0;
  srand(time(NULL));
  fprintf(ofile, "%d\n", N);
  for(i = 0; i < N; i++) {
    A = rand() % 100;
    if(A > tmp) tmp = A;
    fprintf(ofile, "%d\n", A);
  }
  printf("A generated and written to A.dat\n\nA_max = %d\n", tmp);

  fclose(ofile);
}
