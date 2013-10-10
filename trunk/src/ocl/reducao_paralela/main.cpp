#define __CL_ENABLE_EXCEPTIONS
#include <iostream>
#include <fstream>
#include<sstream>
#include<string>
#include <CL/cl.hpp>
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

int max_compute_units(cl::Device device){

    return device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

}

void ExibeInformacoesDispositivo(cl::Device device){

     cout << "-----------------------------------------------------------" << endl;
     cout<<"\nPlatform: "<< device.getInfo<CL_DEVICE_NAME>() << endl;
     cout<<"Max Compute units: "<< device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
     cout<<"Max Work group size: "<< device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl;
     cout << "\n-----------------------------------------------------------\n" << endl;
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

    error=clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &devices);

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

     if (error != CL_SUCCESS) {
        printf("\n Error number %d", error);
    }

     cl_mem mem1;
     mem1=clCreateBuffer(context, CL_MEM_READ_WRITE, elementos * sizeof(int), NULL, &error);

     //mem1=clCreateBuffer(context, CL_MEM_READ_WRITE, elementos , NULL, &error);

     error=clEnqueueWriteBuffer(cq, mem1, CL_TRUE, 0,  elementos * sizeof(int), x, 0, NULL, &event1);

     cl_kernel k =clCreateKernel(prog, "soma", &error);

     error = clSetKernelArg(k, 0,  sizeof(mem1), &mem1);
     error = clSetKernelArg(k, 1,  sizeof(mem1), NULL);

     error=clEnqueueNDRangeKernel(cq, k, 1, NULL, &worksize, &worksize, 0, NULL, &event2);

     error=clEnqueueReadBuffer(cq, mem1, CL_TRUE, 0,  elementos * sizeof(int), x, 0, NULL, &event3);

     clFinish(cq);

     clWaitForEvents(1 , &event3);

     cl_ulong time_start, time_end;

     double total_time=0;
     int global = elementos;

     clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
     clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

     total_time += time_end - time_start;

     clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
     clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

     total_time += time_end - time_start;

     clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
     clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

     total_time += time_end - time_start;

     //printf("\nExecution time in seconds = %0.10f s\n", (total_time / 1000000.0) );

     //Tempo em milisegundos
     total_time = total_time/ 1000000.0;

     printf("%0.10f", total_time);

     //ExibeInformacoesDispositivo(dispositivos[0]);
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


    const int elementos = atoi(argv[1]);

   // cout << "--------------------------------" << endl;

   // cout<< "Tamanho: " << elementos;

    //Inicialização
    int * x = new int[elementos];

    for(int i=0;i<elementos; ++i) x[i] = i+1;

    int valor_seq = soma_seq(x,elementos);

    soma_paralela(x, elementos);

    //Impressão do array
    //for(int i=0;i<elementos;++i) cout << '[' << x[i] << ']'; cout<< endl;

    //cout << "Soma: " << x[0] << ", soma sequencial = "  << valor_seq << endl;

    //if(x[0] != valor_seq) cout << "Soma incorreta" << endl;



    delete[] x;

    return 0;
}
