#include <iostream>
#include <cuda.h>
#include <cuda_runtime.h>
#include<stdlib.h>
#include<stdio.h>

#define MAX 1024
__global__ void reduce(int *g_idata, int *g_odata, int length){

    extern __shared__ int sdata[];

    int next_power_of_2 = (int)pow( (float)2.0, (int) ceil( log2( (float) blockDim.x  ) ) );

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;

    if(tid>length) return;
   
    sdata[tid] = g_idata[i];
    __syncthreads();

    for(unsigned int s=next_power_of_2/2; s>0; s>>=1) {
        if (tid < s && tid+s< blockDim.x) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) g_odata[blockIdx.x] = sdata[0];
}

int soma_seq(int *x, int tamanho){
    int i,sum=0;
    for(i=0;i<tamanho;i++) {

        sum+=x[i];
    }

    return sum;
}


int main(int argc, char * argv[])
{
        const int N = atoi(argv[1]);
	
	int a[MAX], b[MAX], size = N;
	int *dev_a, *dev_b, *dev_size;
	
	cudaMalloc( (void**)&dev_a, N * sizeof(int) );
	cudaMalloc( (void**)&dev_b, N * sizeof(int) );
	cudaMalloc( (void**)&dev_size, sizeof(int) );

	for (int i=0; i<N; i++ )
	    a[i]= 1;	

	int valor_seq = soma_seq(a, N);

	float time;
	cudaEvent_t start, stop;

	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);

	cudaMemcpy (dev_a,a, N*sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy (dev_b,b, N*sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy (dev_size, &size, 1*sizeof(int), cudaMemcpyHostToDevice);

	reduce<<<64,N, N* sizeof(int)>>>(dev_a, dev_b, size);

	cudaMemcpy(b, dev_b, N*sizeof(int),cudaMemcpyDeviceToHost);
	
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&time, start, stop);

	printf("%.10f\n", time);

	if(b[0] != valor_seq)
            printf("Soma incorreta\n");

	//printf("Soma: %d\n", b[0]);

	cudaFree(dev_a);
	cudaFree(dev_b);

	return 0;
}
