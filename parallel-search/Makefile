# Makefile

CUDA = nvcc
CUDAOPT = -arch=sm_30 -Xcompiler -fopenmp
CUDA_DIR = /usr/local/cuda/lib64

CUDAINCS = -I $(CUDA_DIR)/include
CUDALIBS = -L $(CUDA_DIR) -lcuda -lcudart

SRCCUDA = gpu_search.cu

# compile normally
all: CUDAOPT += -O2
all: gpu_search

OBJSCUDA = $(addsuffix .o, $(basename $(SRCCUDA)))

%.o:%.cu
	$(CUDA) $(CUDAOPT) -dc $< $(CUDAINCS)

gpu_search: $(OBJSCUDA)
	$(CUDA) $(CUDAOPT) -o $@ $(OBJSCUDA) $(LDLIBS) $(CUDALIBS)

clean:
	rm -f *.o gpu_search
