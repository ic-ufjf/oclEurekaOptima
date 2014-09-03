#define __CL_ENABLE_EXCEPTIONS
#include <iostream>
#include <fstream>
#include<sstream>
#include<string.h>
#include <CL/cl.h>
#include <vector>
#include <utility>
#include <cstdlib>
#include <math.h>

using namespace std;

const char * kernel_srt;

std::string LoadKernel()
{
    std::ifstream file("kernel.cl");
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int local_size, global_size;

int get_max_compute_units(cl_device_id device){

  char buffer[10240];
  cl_uint buf_uint;
  cl_ulong buf_ulong;

  clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buf_uint), &buf_uint, NULL);
	
  int next_power_of_2 = (int)pow( (float)2.0, (int) ceil( log2( (float) buf_uint ) ) );

  return next_power_of_2;
}

void soma_paralela(int *x, const int elementos){

    cl_int error;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint platforms, devices;
    cl_event event1, event2, event3;

    error=clGetPlatformIDs(1, &platform, &platforms);

    if (error != CL_SUCCESS) {
            printf("\n Error number %d", error);
    }

    error= clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &devices);

    cl_context_properties properties[]={
                CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0};

    cl_context context = clCreateContext(properties, 1, &device, NULL, NULL, &error);

    cl_command_queue cq = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &error);

    std::string kernel_string = LoadKernel();
    kernel_srt = kernel_string.c_str();

    size_t srcsize = (size_t)strlen(kernel_srt), worksize = (size_t)elementos, localsize=(size_t)elementos;

    const char *srcptr[]={kernel_srt};

    cl_program prog = clCreateProgramWithSource(context, 1, srcptr, &srcsize, &error);

    error=clBuildProgram(prog, 0, NULL, "", NULL, NULL);

    cl_mem mem1;
    mem1=clCreateBuffer(context, CL_MEM_READ_WRITE, elementos * sizeof(int), NULL, &error);

    error=clEnqueueWriteBuffer(cq, mem1, CL_TRUE, 0,  elementos * sizeof(int), x, 0, NULL, &event1);

    cl_kernel k =clCreateKernel(prog, "reduce", &error);

    int compute_units  = get_max_compute_units(device);
    //int max_group_size = get_max_work_group_size(device);

    //Execução do kernel
    clSetKernelArg(k,0, sizeof(mem1), &mem1);

    //O vetor local será alocado com o número de elementos igual ao número de compute units,
    //pois será utilizado somente na segunda etapa do algoritmo, que é uma redução paralela;

    //compute_units = std::min(compute_units, elementos);

    clSetKernelArg(k,1, sizeof(int)*compute_units, NULL);
    clSetKernelArg(k,2, sizeof(elementos), &elementos);
    clSetKernelArg(k,3, sizeof(mem1), &mem1);

    worksize = (size_t)compute_units;
	
    error=clEnqueueNDRangeKernel(cq, k, 1, NULL, &worksize, &worksize, 0, NULL, &event2);

    error=clEnqueueReadBuffer(cq, mem1, CL_TRUE, 0,  elementos * sizeof(int), x, 0, NULL, &event3);

    clFinish(cq);

    clWaitForEvents(1 , &event3);

    cl_ulong time_start, time_end;

    double total_time=0;
    int global = elementos;

    clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_END,   sizeof(time_end),   &time_end,   NULL);

    total_time += time_end - time_start;

    clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_END,   sizeof(time_end),   &time_end, NULL);

    total_time += time_end - time_start;

    clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

    total_time += time_end - time_start;

    //Tempo em milisegundos
    total_time = total_time / 1000000.0;

    printf("%0.10f", total_time);
}

int soma_sequencial(int *x, int elementos){

    int aux=0,i;

    for(i=0;i<elementos;i++) aux+=x[i];

    return aux;
}

int main(int argc, char * argv[])
{
    const int elementos = atoi(argv[1]);

    //Inicialização
    int * x = new int[elementos];

    for(int i=0;i<elementos; ++i) x[i] = 1;

    int valor = soma_sequencial(x, elementos);

    soma_paralela(x, elementos);

    //Impressão do array
    //for(int i=0;i<elementos;++i) cout << '[' << x[i] << ']'; cout<< endl;

    if(x[0] == valor){
        //cout << endl << x[0] << endl;
    }
    else cout << "Soma incorreta. (" << x[0] << ") em vez de (" << valor<< ")" <<endl;

    delete[] x;

    return 0;
}
