#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#include <cuda.h>

#define MAXTHREADS 128
#define MAXBLOCKS 64

/* A bitwise function to determine the maximum exponent x that satisfies the
 * inequality 2^x < n.
 */
int floorLog2(unsigned int n) {
  int pos = 0;
  if (n >= 1<<16) { n >>= 16; pos += 16; }
  if (n >= 1<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1<< 1) {           pos +=  1; }
  return ((n == 0) ? (-1) : pos);
}

/* A bitwise function to determine the minimum number n that satisfies the
 * inequality n > x, where n = 2^a for arbitrary a.
 */
unsigned int nextPow2(unsigned int x) {
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return ++x;
}

/* The GPU kernel that performs the power-of-two maximum value search
 * algorithm.
 */
__global__ void entrySearch_max_kernel(int *_gin, int *_gout,
  int N)
{
    // create shared memory
    extern __shared__ int _sA[];

    // load shared mem
    int tid = threadIdx.x;                              // thread index
    int i = blockIdx.x * blockDim.x * 2 + threadIdx.x;  // A index
 
    // the larger of the two values gets written to shared memory
    if(i + blockDim.x < N) {                // if within blocked area
      if(_gin[i] > _gin[i + blockDim.x]) {
        _sA[tid] = _gin[i];
      } else {
        _sA[tid] = _gin[i + blockDim.x];
      }
    } else if (i < N) {                     // if not power of two
      _sA[tid] = _gin[i];
    } else {                                // if outside array
      _sA[tid] = 0;
    }

    __syncthreads();  // wait for all threads to catch up

    // do comparison in shared mem
    for(unsigned int s=blockDim.x/2; s>0; s>>=1) {
      if(tid < s) {
        if(_sA[tid] < _sA[tid + s]) {
          _sA[tid] = _sA[tid + s];
        }
      }
      __syncthreads();  // wait for all threads to catch up
    }
  
    // write result for this block to global mem
    if(tid == 0) {
      _gout[blockIdx.x] = _sA[0];
    }
}

/* The base function of the maximum search algorithm. */
int find_max(int N, int *_A)
{
  int nthreads = 0;                   // number of threads per block
  if(N < MAXTHREADS * 2) {
    nthreads = nextPow2((N+1)/2);
  } else {
    nthreads = MAXTHREADS;
  }
  int nblocks = (N + (nthreads * 2 - 1)) / (nthreads * 2);  // number of blocks

  // create temporary search array on device
  int *_tmp;
  cudaMalloc((void**)&_tmp, nblocks * sizeof(int));

  dim3 dimBlocks(nthreads, 1, 1);         // the dimension of a thread block
  dim3 numBlocks(nblocks, 1, 1);          // the layout of thread blocks
  int smemSize = nthreads * sizeof(int);  // amount of shared memory

  // run kernel
  entrySearch_max_kernel<<<numBlocks, dimBlocks, smemSize>>>(_A, _tmp, N);

  cudaThreadSynchronize();  // wait until all threads catch up

  // if there was more than one block, re-run the kernel on the minimum values 
  // from each of the blocks
  while(nblocks > 1) {
    // use only the first block_number indices in min_arr
    N = nblocks;

    // recalculate kernel launch parameters
    if(N < MAXTHREADS * 2) {
      nthreads = nextPow2((N+1)/2);
    } else {
      nthreads = MAXTHREADS;
    }
    nblocks = (N + (nthreads * 2 - 1)) / (nthreads * 2);

    // run kernel
    entrySearch_max_kernel<<<numBlocks, dimBlocks, smemSize>>>(_tmp, _tmp, N);

    cudaThreadSynchronize();  // wait until all threads catch up
  }

  // copy back final answer, which resides in position zero
  int max;
  cudaMemcpy(&max, _tmp, sizeof(int), cudaMemcpyDeviceToHost);
  cudaFree(_tmp);
  return max;
}

/* The main test function that creates a test array of random values and calls
 * find_min(...).  It displays both the known result as maintained through the
 * CPU-generated array and the GPU test result.
 */
int main(int argc, char** argv) 
{
  int j = 0;
  double wT = 0;
  int repeat = 100;

/**** SEARCH ****/
  int dev = 0;
  int i = 0;
  int N = 0;
  int* A;
  int* _A;
  int result = 0;

  // set CUDA device
  cudaSetDevice(dev);
/****************/

  // read A.dat
  printf("Reading A.dat...");
  fflush(stdout);
  FILE *ifile = fopen("A.dat", "r");
  int scan = fscanf(ifile, "%d\n", &N);
  A = (int*) malloc(N * sizeof(int));
  for(i = 0; i < N; i++) {
    scan = fscanf(ifile, "%d\n", &A[i]);
  }
  printf("done\n");
  fflush(stdout);

/**** SEARCH ****/
  // copy host array to device
  cudaMalloc((void**)&_A, N * sizeof(int));
  cudaMemcpy(_A, A, N * sizeof(int), cudaMemcpyHostToDevice);
/****************/

  wT = omp_get_wtime();
  for(j = 0; j < repeat; j++) {
/**** SEARCH ****/
    result = find_max(N, _A);   // search
/****************/
  }
  wT = omp_get_wtime() - wT;
  printf("\nGPU:    searching A %d times...", repeat);
  fflush(stdout);
  printf("done\n        found A_max = %d in %f seconds.\n", result, wT);
  fflush(stdout);

/**** SEARCH ****/
  cudaFree(_A);  // clean up device array
/****************/
  free(A);

  cudaThreadExit();
}
