#define __CL_ENABLE_EXCEPTIONS
#include <iostream>
#include <CL/cl.hpp>
#include <vector>
#include <utility>
#include <cstdlib>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <exception>
#include <string>
#include <omp.h>
#include <stdlib.h>
#include <sys/time.h>

#include "include/CLRadixSort.hpp"

using namespace std;

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

const char * kernel_srt;

struct MException : public std::exception
{
   std::string s;
   MException(std::string ss) : s(ss) {}
   ~MException() throw () {}
   const char* what() const throw() { return s.c_str(); }
};

std::string LoadKernel(std::string fileName)
{
    std::ifstream file(fileName.c_str());
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

double getRealTime()
{
    struct timeval tv;
    gettimeofday(&tv,0);
    return (double)tv.tv_sec + 1.0e-6*(double)tv.tv_usec;
}


void sortup(int *, int, int);
void sortdown(int *, int, int);
void mergeup(int *,int, int);
void mergedown(int *,int, int);

int greatestPowerOfTwoLessThan(int n)
{
    int k=1;
    while (k<n)
        k=k<<1;
    return k>>1;
}

void Swap(int *a,int i, int j)
{
    int t=a[i];
    a[i]=a[j];
    a[j]=t;
}

void compare(int *a,int i, int j, int dir)
{
    if (dir==(a[i]>a[j]))
        Swap(a, i, j);
}

void mergeup(int *a,int m, int n) {

    if (n==0) return;
    int i, t;

    int p=greatestPowerOfTwoLessThan(n);

    for (i=0;i<n+m-p;i++) {
        compare(a, m+i, m+i+n, 1);
    }

    mergeup(a, m, p);
    mergeup(a,m+p, n-m);
}



void mergedown(int *a, int m, int n) {

    if (n==0) return;

    int p=greatestPowerOfTwoLessThan(n);

    int i,t;
    for (i=0;i<n;i++) {
          compare(a, m+i, m+i+n,0);
    }

    mergedown(a,m, p);
    mergedown(a,m+n, n-m);
}


void sortup(int *a, int m, int n) {//de m a m+n

    if (n==1) return;

    int p=n/2;

    sortup(a, m, p);
    sortdown(a, m+n/2,n/2);
    mergeup(a, m,n/2);

}

void sortdown(int *a, int m, int n) {//de m a m+n

    if (n==1) return;
    sortup(a, m,n/2);
    sortdown(a, m+n/2,n/2);
    mergedown(a, m,n/2);

}


void exchange(int *a, int i, int j)
{
    int t=a[i];
    a[i]=a[j];
    a[j]=t;
}


void comparar(int *a, int i, int j, int dir)
{
    if (dir==(a[i]>a[j]))
        exchange(a,i, j);
}

 void bitonicMerge(int *a, int lo, int n, int dir)
{
    if (n>1)
    {
        int m=greatestPowerOfTwoLessThan(n);
        for (int i=lo; i<lo+n-m; i++)
            comparar(a, i, i+m, dir);
        bitonicMerge(a,lo, m, dir);
        bitonicMerge(a,lo+m, n-m, dir);
    }
}




void bitonicSort(int *a, int lo, int n, int dir)
{
    if (n>1)
    {
        int m=n/2;
        bitonicSort(a,lo, m, !dir);
        bitonicSort(a,lo+m, n-m, dir);
        bitonicMerge(a,lo, n, dir);
    }
}
/*
Implementação recursiva do Bitonic Sort.
*/
double BitonicRec(int *a, int n){

    double start = getRealTime();

    bitonicSort(a, 0, n, 1);

    return getRealTime() - start;
}


/*
Implementação sequencial do Bitonic Sort.
*/

double BitonicSortSequencial(int *a, int N){

    double start = getRealTime();

    int i,j,k;
    for (k=2;k<=N;k=2*k) {
      for (j=k>>1;j>0;j=j>>1) {
        for (i=0;i<N;i++) {
          int ixj=i^j, t;
          if ((ixj)>i) {
            if ((i&k)==0 && a[i]>a[ixj]){
                t = a[i];
                a[i] =  a[ixj];
                a[ixj] = t;
            }
            if ((i&k)!=0 && a[i]<a[ixj]) {
                t = a[i];
                a[i] =  a[ixj];
                a[ixj] = t;
            }
          }
        }
      }
    }

    return getRealTime() -start;
}


double SortQuickSort(int *a, int N) {

    double start = getRealTime();
    qsort (a, N, sizeof(int), compare);
    return getRealTime() - start;
}

int GetNextPowerOfTwo(int N){

    return pow(2, ceil( log2( N ) ) );

}

double SortOpenCLSelecionSort(int *a, int N, bool isCPU, const cl::Context &context,
    cl::CommandQueue &queue, cl::Kernel &kernel) {

    cl::Buffer bufferIn(context, CL_MEM_READ_WRITE, N*sizeof(cl_int));
    cl::Buffer bufferOut(context, CL_MEM_READ_WRITE,  N*sizeof(cl_int));

    queue.enqueueWriteBuffer(bufferIn, CL_TRUE,0, N*sizeof(cl_int) , a);

    int j, k;
    kernel.setArg(0, bufferIn);
    kernel.setArg(1, bufferOut);

    double start = getRealTime();

    queue.enqueueNDRangeKernel(kernel, cl::NDRange(),
    cl::NDRange(N), cl::NDRange());
    queue.finish();

    double end = getRealTime() - start;

    queue.enqueueReadBuffer(bufferOut, CL_TRUE,0, N*sizeof(cl_int), a);

    return end;
}


double SortOpenCLBitonicSort(int *a, int N, bool isCPU, const cl::Context &context,
    cl::CommandQueue &queue, cl::Kernel &kernel) {

    cl::Buffer bufferIn(context, CL_MEM_READ_WRITE, N*sizeof(cl_int));

    queue.enqueueWriteBuffer(bufferIn, CL_TRUE, 0, N*sizeof(cl_int) , a);

    int j, k;

    kernel.setArg(0, bufferIn);

    double start = getRealTime();

    for (k=2; k<=N; k=2*k) {
        kernel.setArg(1, k);
        for (j=k>>1; j>0; j=j>>1) {
            kernel.setArg(2, j);
            queue.enqueueNDRangeKernel(kernel, cl::NDRange(),
                cl::NDRange(N), cl::NDRange());
            queue.finish();
        }
    }

    double end = getRealTime() - start;

    queue.enqueueReadBuffer(bufferIn, CL_TRUE,0, N*sizeof(cl_int), a);

    return end;
}

void assert_sort(const std::string msg,
                 const std::vector<int> &expected,
                 const std::vector<int> &actual) {

    if (expected != actual){
        throw MException(msg);
    }
}

cl::Kernel kernelBitonic;

void SortTest(bool &first,
                int N,
                const cl::Context &cpuContext,
                cl::CommandQueue &cpuQueue,
                cl::Kernel &cpuKernel,
                const cl::Context &gpuContext,
                cl::CommandQueue &gpuQueue,
                cl::Kernel &gpuKernel) {

    std::vector<int> unsorted;
    unsorted.resize(N);
    std::srand(0);
    for (int i=0; i<N; i++)
        unsorted[i] = std::rand();

    std::vector<int> sorted(unsorted.begin(), unsorted.end());
    std::sort(sorted.begin(), sorted.end());

    double media = 0;
    int exec = 10;

    // STL sort
    std::vector<int> a(unsorted.begin(), unsorted.end());

    for(int i = 1; i<=exec;i++){

        media+= SortQuickSort(&a[0], a.size());
        assert_sort("SortQuickSort", sorted, a);

    }

    double sortSTLTime = media/exec;

    media = 0;

    for(int i = 1; i<=exec;i++){

        // Implementação sequencial do Bitonic Sort
        std::copy(unsorted.begin(), unsorted.end(), a.begin());
        media += BitonicSortSequencial(&a[0], a.size());
        assert_sort("BitonicSortSequencial", sorted, a);

    }
     double sortBitonicSeqTime = media/exec;

    // OpenCL em CPU
    /*std::copy(unsorted.begin(), unsorted.end(), a.begin());
    double sortOpenCLCPUTime = SortOpenCLSelecionSort(&a[0], a.size(), false,
        cpuContext, cpuQueue, cpuKernel);
    assert_sort("SortOpenCLCPU", sorted, a);
*/

    for(int i = 1; i<=exec;i++){

        // Implementação recursiva do Bitonic Sort
        std::copy(unsorted.begin(), unsorted.end(), a.begin());
        media +=  SortOpenCLBitonicSort(&a[0], a.size(), false,
            cpuContext, cpuQueue, kernelBitonic);

        assert_sort("BitonicOpenCL", sorted, a);

    }

    double sortBitonicOpenCLTime = media/exec;

    media=0;

    for(int i = 1; i<=exec;i++){

        // OpenCL em GPU
        std::copy(unsorted.begin(), unsorted.end(), a.begin());
        media += SortOpenCLSelecionSort(&a[0], a.size(), false,
            cpuContext, cpuQueue, cpuKernel);
        assert_sort("SortOpenCLGPU", sorted, a);

    }

    double sortOpenCLGPUTime = media/exec;

    printf("----------------------------------\n");
    printf("N: %d\n", N);


        //std::cout <<  "\"N\",\"QuickSort\", \"Bitonic rec\" , \"Bitonic seq\" , \"OpenCLCPU\",\"OpenCLGPU\"" << std::endl;

    printf("QuickSort: \t%.10f \n", sortSTLTime);
    printf("Bitonic seq: \t%.10f \n", sortBitonicSeqTime);
    printf("OpenCLb: \t%.10f \n", sortBitonicOpenCLTime);
    printf("OpenCLs:  \t%.10f \n", sortOpenCLGPUTime);


    printf("----------------------------------\n");
}


void GetOpenCLObjects(std::vector<cl::Platform> &platforms,
    cl_device_type type, cl::Context &context, cl::CommandQueue &queue,
    cl::Kernel &kernel) {
    // create context and queue
    cl_context_properties cprops[3] = { CL_CONTEXT_PLATFORM,
        (cl_context_properties)platforms[0](), 0 };
    context = cl::Context(type, cprops);
    std::vector<cl::Device> devices =
        context.getInfo<CL_CONTEXT_DEVICES>();
    if (devices.size() == 0)
        throw MException("Nenhum dispositivo disponivel");
    queue = cl::CommandQueue(context, devices[0]);


    cl::Program program;
    try {

        std::string kernel_string = LoadKernel("kernel.cl");
        kernel_srt = kernel_string.c_str();

        cl::Program::Sources sources(1,  std::make_pair(kernel_srt, strlen(kernel_srt)));
        program = cl::Program(context, sources);
        program.build(devices);

        kernel = cl::Kernel(program, "RankSort");
        kernelBitonic = cl::Kernel(program, "sort");

    } catch (cl::Error &err) {

        if (err.err() == CL_BUILD_PROGRAM_FAILURE)
            std::cout <<
                program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0])
                << std::endl;
        throw;
    }
}

int main() {

    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.size() == 0){
            throw MException("OpenCL nao disponivel");
        }

        cl::Context cpuContext, gpuContext;
        cl::CommandQueue cpuQueue, gpuQueue;
        cl::Kernel cpuKernel, gpuKernel;

        GetOpenCLObjects(platforms, CL_DEVICE_TYPE_CPU, cpuContext, cpuQueue, cpuKernel);

        //GetOpenCLObjects(platforms, CL_DEVICE_TYPE_GPU, gpuContext, gpuQueue, gpuKernel);

        bool first = true;
        for (int b=1; b<=14; b++)
            SortTest(first, 1<<b,
                cpuContext, cpuQueue, cpuKernel,
                gpuContext, gpuQueue, gpuKernel);

    } catch (cl::Error &err) {
        std::cout << "Error: " << err.what() << "(" << err.err() << ")"
            << std:: endl;
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
