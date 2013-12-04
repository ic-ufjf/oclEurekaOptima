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
#include <sys/time.h>
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

double getRealTime()
{
    struct timeval tv;
    gettimeofday(&tv,0);
    return (double)tv.tv_sec + 1.0e-6*(double)tv.tv_usec;
}

template<class T> inline std::string ToString( const T& t )
{
   try {
      std::stringstream ss; ss << t; return ss.str();
   }
   catch( ... ) { return ""; }
}


int geracao = 0;

//Utilizado para tratamento de erros
cl_int status;

cl_device_id device;

cl_command_queue cmdQueue;
cl_context context = NULL;
cl_device_id * devices = NULL;
cl_uint numDevices = 0;
cl_kernel kernelIteracao, kernelInicializacao, kernelSubstituicao;

size_t globalWorkSize[1], localWorkSize;
size_t datasize;

cl_mem bufferA, bufferB, bufferC;

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

    device = devices[0];

	cmdQueue = clCreateCommandQueue(context,
									device,
                                    CL_QUEUE_PROFILING_ENABLE,
									&status);
    if(status != CL_SUCCESS){
        cout << "Erro ao criar a fila de execução. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }


    //---------------------------------------------
    // 5: Criação e compilação do programa
    //---------------------------------------------

    std::string header_string = "#define NUMERO_DE_GERACOES " + ToString(NUMERO_DE_GERACOES) + " \n"+
                                "#define DIMENSOES_PROBLEMA " + ToString(DIMENSOES_PROBLEMA)  + "\n" +
                                "#define TAMANHO_POPULACAO " + ToString(TAMANHO_POPULACAO) + "\n" +
                                "#define TAMANHO_VALOR " + ToString(TAMANHO_VALOR) + "\n" +
                                "#define TAMANHO_INDIVIDUO DIMENSOES_PROBLEMA*TAMANHO_VALOR  " + "\n" +
                                "#define TAXA_DE_MUTACAO " + ToString(TAXA_DE_MUTACAO) + "\n" +
                                "#define TAXA_DE_RECOMBINACAO " + ToString(TAXA_DE_RECOMBINACAO )+ "\n" +
                                "#define TAMANHO_TORNEIO " + ToString(TAMANHO_TORNEIO) + "\n" +
                                "#define ELITE " + ToString(ELITE) + "\n";

    std::string body_string   = LoadKernel("kernel.cl");
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
							"-I /usr/include", //Random123
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

    //cout << "Datasize:" << datasize << endl;

	bufferA = clCreateBuffer(context, CL_MEM_READ_WRITE , datasize, NULL, &status);
	bufferC = clCreateBuffer(context, CL_MEM_READ_WRITE , datasize, NULL, &status);


    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    bufferB = clCreateBuffer(context, CL_MEM_READ_WRITE , datasize, NULL, &status);


    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 7: Criação do kernel
	//---------------------------------------------

	//Cria o kernel de avaliação
	kernelIteracao = clCreateKernel(program, "iteracao", &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o kernel. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    kernelInicializacao = clCreateKernel(program, "inicializa_populacao", &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o kernel inicializacao. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    kernelSubstituicao = clCreateKernel(program, "substituicao", &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o kernel 'substituicao'. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 8: Argumentos do kernel
	//---------------------------------------------


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


void exibe_melhor(individuo * melhor){

    printf("---------------------------------");
    printf("\nGeracao %d: \n", geracao);

    printf("\nMelhor da geracao: %d: %d\n", geracao, melhor->aptidao);
}


void substituicao(individuo *pop){

    status = clSetKernelArg(kernelSubstituicao, 0, sizeof(bufferA), &bufferA);
    status = clSetKernelArg(kernelSubstituicao, 1, sizeof(bufferB), &bufferB);
    status = clSetKernelArg(kernelSubstituicao, 2, sizeof(bufferC), &bufferC);

    #ifdef PROFILING

    double start = getRealTime();

    #endif

    clEnqueueNDRangeKernel(cmdQueue,
                           kernelSubstituicao,
                           1,
                           NULL,
                           globalWorkSize,
                           NULL,
                           0,
                           NULL,
                           NULL);

   /*
    Troca os buffers A e C, de forma que a população gerada na etapa de substituição
    passe a ser a população atual
   */
   cl_mem aux = bufferA;
   bufferA = bufferC;
   bufferC = aux;

   individuo melhor1[1], melhor2[1];

   size_t offset = {sizeof(individuo) * ELITE};

   clEnqueueReadBuffer(cmdQueue, bufferA, CL_TRUE, 0,
						datasize, pop,
						0, NULL, &event3);


   clEnqueueReadBuffer(cmdQueue, bufferA, CL_TRUE, 0,
						sizeof(individuo), melhor1,
						0, NULL, &event3);

   clEnqueueReadBuffer(cmdQueue, bufferA, CL_TRUE, offset,
						sizeof(individuo), melhor2,
						0, NULL, &event3);


   clFinish(cmdQueue);


   /*printf("Melhor1: %d \n", melhor1[0].aptidao);

   for(int c=0;c< TAMANHO_INDIVIDUO;c++){
        printf(" %d", melhor1[0].genotipo[c]);
   }

   printf("\nMelhor2: %d \n", melhor2[0].aptidao);

    for(int c=0;c< TAMANHO_INDIVIDUO;c++){
        printf(" %d", melhor2[0].genotipo[c]);
   }
   */

   if(melhor1[0].aptidao > melhor2[0].aptidao){
        exibe_melhor(melhor1);
   }
   else{
         exibe_melhor(melhor2);
   }

   #ifdef PROFILING

   double end = getRealTime();
   printf("\nTempo de substituicao: %.10f  \n", end-start);

   #endif
}



/*
    Executa o kernel de avaliação
*/
void iteracao(individuo * populacao){

    int seed = geracao * TAMANHO_POPULACAO + rand();

    status = clSetKernelArg(kernelIteracao,
							0,
							sizeof(bufferA),
							&bufferA);

    status = clSetKernelArg(kernelIteracao,
							1,
							sizeof(geracao),
							&geracao);


    status = clSetKernelArg(kernelIteracao,
							2,
							sizeof(seed),
							&seed);

    status = clSetKernelArg(kernelIteracao,
							3,
							sizeof(bufferB),
							&bufferB);

	//---------------------------------------------
	// 10: Transferência dos dados do host para o dispositivo
	//---------------------------------------------

	//Transfere os dados da população para o bufferA
	/*status = clEnqueueWriteBuffer(cmdQueue,
								  bufferA,
								  CL_TRUE,
								  0,
								  datasize,
								  populacao,
								  0,
								  NULL,
								  &event1);

    */


    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 11: Enfileira o kernel para execução
	//---------------------------------------------

    size_t global_size[1] = {TAMANHO_POPULACAO/2};

	status = clEnqueueNDRangeKernel(cmdQueue,
									kernelIteracao,
									1,
                                    NULL,
									global_size,
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

	/*clEnqueueReadBuffer(cmdQueue,
						bufferB,
						CL_TRUE,
						0,
						datasize,
						//sizeof(individuo),
						newPop,
						0,
						NULL,
						&event3);
    */

    //Espera o término da fila de execução
	clFinish(cmdQueue);

    substituicao(populacao);
}

void exibe_geracao(individuo * pop){

    printf("---------------------------------");
    printf("\nGeracao %d: \n", geracao);

    int mais_apto = obtem_mais_apto(pop);

    printf("\nMelhor da geracao: %d: %d\n", geracao, mais_apto);
}


void inicializa_populacao(individuo * pop){

    int seed = rand();

    status = clSetKernelArg(kernelInicializacao,
							0,
							sizeof(bufferA),
							&bufferA);

    status = clSetKernelArg(kernelInicializacao,
							1,
							sizeof(seed),
							&seed);

    //Transfere os dados da população para o bufferA
	status = clEnqueueWriteBuffer(cmdQueue,
								  bufferA,
								  CL_TRUE,
								  0,
								  datasize,
								  pop,
								  0,
								  NULL,
								  &event1);


    status = clEnqueueNDRangeKernel(cmdQueue,
									kernelInicializacao,
									1,
                                    NULL,
									globalWorkSize,
									NULL,
									0,
									NULL,
									&event2);


    /*clEnqueueReadBuffer(cmdQueue,
						bufferA,
						CL_TRUE,
						0,
						datasize,
						pop,
						0,
						NULL,
						&event3);
     */

    //Espera o término da fila de execução
	clFinish(cmdQueue);
}

void exibePopulacao(individuo * populacao){

    int i,j;

    printf("\n");

    for(i=0; i < TAMANHO_POPULACAO; i++){

        for(j=0; j< TAMANHO_INDIVIDUO; j++){
            printf(" %d ", populacao[i].genotipo[j]);
        }
        printf("---> %d  \n", populacao[i].aptidao);
    }
}

void ag_paralelo(individuo * pop){

    initializeOpenCL();

    inicializa_populacao(pop);
    geracao++;

    while(geracao < NUMERO_DE_GERACOES){

        #ifdef PROFILING
        double start = getRealTime();

        #endif

        //exibe_geracao(pop);
        iteracao(pop);

        //exibePopulacao(pop);

        geracao++;

        #ifdef PROFILING

        double end = getRealTime();
        printf("\nTempo de execucao da iteracao do AG: %.10f  \n", end-start);

        #endif
    }
}
