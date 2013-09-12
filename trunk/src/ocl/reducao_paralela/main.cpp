/*
* Implementaçao simples de um algoritmo para soma paralela dos elementos de um vetor.
*/

#define __CL_ENABLE_EXCEPTIONS
#include <iostream>
#include <CL/cl.hpp>
#include <vector>
#include <utility>
#include <cstdlib>
#include <math.h>

using namespace std;

const char * kernel_srt =

"__kernel void "
"soma_erro(__global int *E, __local int *EP){ "

"    int lo_id = get_global_id(0); "
"    int gr_id = get_group_id(0);  "
"    int s = get_local_size(0)/2;  "

"    EP[lo_id] = E[lo_id]; "
"    EP[lo_id+s] = E[lo_id+s]; "

"    for(;s>0;s/=2){ "
"       barrier(CLK_LOCAL_MEM_FENCE);"

"        if(lo_id < s) EP[lo_id] += EP[lo_id+s]; "
"    } "

"    if(lo_id==0) E[gr_id] = EP[0]; "

" } ";


void soma_paralela(int *x, const int elementos){

    //descobrir e escolher plataformas e dispositivos
    vector<cl::Platform> plataformas;
    vector<cl::Device> dispositivos;

    cl::Platform::get(&plataformas);

    plataformas[1].getDevices(CL_DEVICE_TYPE_ALL, &dispositivos);

    //Cria o contexto
    cl::Context contexto(dispositivos);

    cl::CommandQueue fila(contexto, dispositivos[0]);

    cl::Program::Sources fonte(1, make_pair(kernel_srt, strlen(kernel_srt)));
    cl::Program programa(contexto, fonte);

    programa.build(vector<cl::Device>());

    cl::Kernel kernel(programa, "soma_erro");

    cl::Buffer bufferE(contexto,  CL_MEM_READ_WRITE, elementos * sizeof(int));

    fila.enqueueWriteBuffer(bufferE, CL_TRUE, 0, elementos * sizeof(int), x);

    //Execução do kernel
    kernel.setArg(0, bufferE);
    kernel.setArg(1, sizeof(int)*elementos, NULL);

    fila.enqueueNDRangeKernel(kernel, cl::NDRange(), cl::NDRange(elementos), cl::NDRange(elementos));

    //bloqueia e espera a finalização da execução do kernel
    fila.finish();

    //Transferência dos resultados para o hospedeiro
    fila.enqueueReadBuffer(bufferE, CL_TRUE, 0, elementos * sizeof(int), x);

}


int main(int argc, char * argv[])
{
    const int elementos = atoi(argv[1]);

    //Inicialização
    int * x = new int[elementos];

    for(int i=0;i<elementos; ++i) x[i] = i+1;

    soma_paralela(x, elementos);

    //Impressão do array
    for(int i=0;i<elementos;++i) cout << '[' << x[i] << ']'; cout<< endl;

    cout << "Soma: " << x[0] <<endl;

    delete[] x;

    return 0;
}
