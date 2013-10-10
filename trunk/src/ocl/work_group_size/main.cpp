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

void ExibeInformacoesDispositivo(cl::Device device){

    /* cout << "-----------------------------------------------------------" << endl;
     cout<<"\nPlatform: "<< device.getInfo<CL_DEVICE_NAME>() << endl;
     cout<<"Max Compute units: "<< device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
     cout << "\n-----------------------------------------------------------\n" << endl;
    */
}

void soma_paralela(int *x, const int elementos){

    //descobrir e escolher plataformas e dispositivos
    vector<cl::Platform> plataformas;
    vector<cl::Device> dispositivos;

    cl::Platform::get(&plataformas);

    plataformas[0].getDevices(CL_DEVICE_TYPE_ALL, &dispositivos);

    //Cria o contexto
    cl::Context contexto(dispositivos);

    cl::CommandQueue fila(contexto, dispositivos[0]);

    std::string kernel_string = LoadKernel();
    kernel_srt = kernel_string.c_str();

    cl::Program::Sources fonte(1, make_pair(kernel_srt, strlen(kernel_srt)));
    cl::Program programa(contexto, fonte);

    programa.build(vector<cl::Device>());

    cl::Kernel kernel(programa, "group_size");

    cl::Buffer bufferE(contexto,  CL_MEM_READ_WRITE, elementos * sizeof(int));

    fila.enqueueWriteBuffer(bufferE, CL_TRUE, 0, elementos * sizeof(int), x);

    //Execução do kernel
    kernel.setArg(0, bufferE);

    fila.enqueueNDRangeKernel(kernel,  cl::NDRange(), cl::NDRange(elementos), cl::NDRange(5));

    fila.finish();

    //Transferência dos resultados para o hospedeiro
    fila.enqueueReadBuffer(bufferE, CL_TRUE, 0, elementos * sizeof(int), x);

    ExibeInformacoesDispositivo(dispositivos[0]);
}

int main(int argc, char * argv[])
{
    const int elementos = atoi(argv[1]);

    //Inicialização
    int * x = new int[elementos];

    for(int i=0;i<elementos; ++i) x[i] = i+1;

    soma_paralela(x, elementos);

    //Impressão do array
    //for(int i=0;i<elementos;++i) cout << '[' << x[i] << ']'; cout<< endl;

    cout << "Elementos: "<< elementos<< ", " "work_group_size: " << x[0] <<", Grupos:" << elementos/x[0]  <<endl;

    delete[] x;

    return 0;
}
