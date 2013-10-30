#define __CL_ENABLE_EXCEPTIONS
#include "agOpenCL.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <CL/cl.h>
#include <vector>
#include <utility>
#include <cstdlib>
#include <math.h>
#include "representacao.h"

using namespace std;

const char * kernel_srt;

std::string LoadKernel(std::string fileName)
{
    std::ifstream file(fileName.c_str());
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

template<class T> inline std::string ToString( const T& t )
{
   try {
      std::stringstream ss; ss << t; return ss.str();
   }
   catch( ... ) { return ""; }
}


//Utilizado para tratamento de erros
cl_int status;

cl_command_queue cmdQueue;
cl_context context = NULL;
cl_device_id * devices = NULL;
cl_uint numDevices = 0;
cl_kernel kernelAvaliacao;

size_t globalWorkSize[1], localWorkSize;
size_t datasize;

cl_mem bufferA;

//Eventos utilizados para medir o tempo de execução do kernel
//e das trocas de memória
cl_event event1, event2, event3;


/*
  Obtém o tempo decorrido entre o início e o fim de um evento (em picosegundos)
*/
float getTempoDecorrido(cl_event event){

    cl_ulong time_start, time_end;

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

    return time_end - time_start;
}


void initializeOpenCL(){

    //---------------------------------------------
	// 1: Descoberta e inicialização da(s) plataforma(s)
	//---------------------------------------------

	cl_uint numPlatforms = 0;
	cl_platform_id * platforms = NULL;

	status = clGetPlatformIDs(0, NULL, &numPlatforms);

	//Aloca espaço para cada plataforma
	platforms = (cl_platform_id*) malloc(numPlatforms*sizeof(cl_platform_id));

	//Obtém as plataformas
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);

    if(status != CL_SUCCESS){
        cout << "Erro ao tentar obter as plataformas. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 2: Descoberta e inicialização do(s) dispositivo(s)
	//---------------------------------------------

	//Obtém o número de dispositivos na plataforma de índice 0
	status = clGetDeviceIDs(platforms[0],
							CL_DEVICE_TYPE_ALL,
							0,
							NULL,
							&numDevices);

    if(status != CL_SUCCESS){
        cout << "Erro ao tentar obter os dispositivos. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    //Aloca espaço para cada dispositivo
	devices = (cl_device_id*) malloc(numDevices*sizeof(cl_device_id));

	//Obtém os dispositivos
	status = clGetDeviceIDs(platforms[0],
							CL_DEVICE_TYPE_ALL,
							numDevices,
							devices,
							NULL);

     if(status != CL_SUCCESS){
        cout << "Erro ao tentar obter os dispositivos. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 3: Criação do contexto de execução
	//---------------------------------------------


	//Cria o contexto de execução, associando os dispositivos
	context = clCreateContext(NULL,
							 numDevices,
						 	 devices,
						 	 NULL,
						 	 NULL,
						 	 &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o contexto de execução. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }


	//-----------------------
	// 4: Criaçaõ da fila de execução
	//---------------------------------------------

	cmdQueue = clCreateCommandQueue(context,
									devices[0],
                                    CL_QUEUE_PROFILING_ENABLE,
									&status);
   if(status != CL_SUCCESS){
        cout << "Erro ao criar a fila de execução. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }


    //---------------------------------------------
	// 5: Criação e compilação do programa
	//-------------

    std::string header_string = "#define NUMERO_DE_GERACOES " + ToString(NUMERO_DE_GERACOES) + " \n"+
                                "#define NUMERO_DE_GERACOES " + ToString(NUMERO_DE_GERACOES) + " \n"+
                                "#define DIMENSOES_PROBLEMA " + ToString(DIMENSOES_PROBLEMA)  + "\n" +
                                "#define TAMANHO_POPULACAO " + ToString(TAMANHO_POPULACAO) + "\n" +
                                "#define TAMANHO_VALOR " + ToString(TAMANHO_VALOR) + "\n" +
                                "#define TAMANHO_INDIVIDUO DIMENSOES_PROBLEMA*TAMANHO_VALOR  " + "\n" +
                                "#define TAXA_DE_MUTACAO " + ToString(TAXA_DE_MUTACAO) + "\n" +
                                "#define TAXA_DE_RECOMBINACAO " + ToString(TAXA_DE_RECOMBINACAO )+ "\n" +
                                "#define TAMANHO_TORNEIO " + ToString(TAMANHO_TORNEIO) + "\n" +
                                "#define ELITE " + ToString(ELITE) + "\n";

    std::string body_string   = LoadKernel("kernel_avaliacao.cl");
    std::string kernel_string = header_string + body_string ;

    kernel_srt = kernel_string.c_str();

    //cout << kernel_srt;

 	size_t programSize = (size_t)strlen(kernel_srt);

	//Cria o programa
	cl_program program = clCreateProgramWithSource(context,
												   1,
												   (const char **)&kernel_srt,
												   &programSize,
												   &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o programa. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//Compilação do programa
	status = clBuildProgram(program,
							  numDevices,
							  devices,
							  NULL,
							  NULL,
							  NULL);

    if(status != CL_SUCCESS){
        cout << "Erro ao compilar o programa. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------
	// 6: Criação dos buffers de memória
	//---------------------------------------------

    datasize = sizeof(individuo)*TAMANHO_POPULACAO;

	bufferA = clCreateBuffer(context, CL_MEM_READ_WRITE , datasize, NULL, &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 7: Criação do kernel
	//---------------------------------------------

	//Cria o kernel de avaliação
	kernelAvaliacao = clCreateKernel(program, "avaliacao", &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o kernel. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 8: Argumentos do kernel
	//---------------------------------------------

	//Associate the input and outputprintf("%0.10f", total_time); buffers with the kernel
	status = clSetKernelArg(kernelAvaliacao,
							0,
							sizeof(bufferA),
							&bufferA);

    if(status != CL_SUCCESS){
        cout << "Erro ao setar os parametros do kernel. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 9: Definição do global-size e local-size
	//---------------------------------------------

	globalWorkSize[0] = TAMANHO_POPULACAO;
	localWorkSize = 1;
}

/*
    Executa o kernel de avaliação
*/
void avaliacao_paralela(individuo * populacao){

	//---------------------------------------------
	// 10: Transferência dos dados do host para o dispositivo
	//---------------------------------------------

	//Transfere os dados da população para o bufferA
	status = clEnqueueWriteBuffer(cmdQueue,
								  bufferA,
								  CL_TRUE,
								  0,
								  datasize,
								  populacao,
								  0,
								  NULL,
								  &event1);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }


	//---------------------------------------------
	// 11: Enfileira o kernel para execução
	//---------------------------------------------

	status = clEnqueueNDRangeKernel(cmdQueue,
									kernelAvaliacao,
									1,
                                    NULL,
									globalWorkSize,
									NULL,
									0,
									NULL,
									&event2);

    if(status != CL_SUCCESS){
        cout << "Erro ao enfileirar o kernel para execucao. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//-------------------------------------------------
	// 12: Leitura dos dados após o término da execução
	//-------------------------------------------------

	clEnqueueReadBuffer(cmdQueue,
						bufferA,
						CL_TRUE,
						0,
						datasize,
						populacao,
						0,
						NULL,
						&event3);

    //Espera o término da fila de execução
	clFinish(cmdQueue);

    //-------------------------------------------------
	// 13: Calcula o tempo de execução do kernel
	//-------------------------------------------------

    double total_time = 0;

    total_time += getTempoDecorrido(event1);
    total_time += getTempoDecorrido(event2);
    total_time += getTempoDecorrido(event3);

    //Tempo em milisegundos
    total_time = total_time / 1000000.0;

    printf("Tempo de execucao do kernel: %0.10f ms\n", total_time);
}
