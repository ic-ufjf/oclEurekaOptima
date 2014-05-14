#include "eg_opencl.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include "gramatica.h"
#include "utils.h"

#include <vector>
#include <utility>
#include <cstdlib>
#include <math.h>
#include <sys/time.h>
#include "representacao.h"
#include "parser.h"

#define check_cl(STATUS,M) if(STATUS!=CL_SUCCESS){ log_error(M); log_error_code(STATUS); log_arquivo(); opencl_dispose(); exit(EXIT_FAILURE); }

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

//Utilizado para tratamento de erros
cl_int status;

cl_device_id device;
cl_uint max_compute_units;

//Número de cores informado por parâmetro
int pcores;

//Indica se o dispositivo é cpu
short CPU;

cl_command_queue cmdQueue;
cl_context context = NULL;
cl_device_id * devices = NULL, * subDevices = NULL;
cl_uint numDevices = 0, numSubDevices=0;
cl_kernel kernelAvaliacao;
cl_program program;

size_t datasize;

cl_mem bufferA, bufferProgramas, bufferFitness, bufferGramatica, bufferDatabase;

int tamanhoBancoDeDados;

//Eventos utilizados para medir o tempo de execução do kernel
//e das trocas de memória
cl_event event1, event2, event3;

size_t preferred_workgroup_size_multiple, max_local_size;

float tempoTotal = 0;
int seed;

#ifdef PROFILING

float tempoTotalProcessamento=0;
float tempoTotalAvaliacao=0;
float tempoTotalSubstituicao=0;
float tempoTotalAG=0;
float tempoTotalTransfMemoria=0;
float tempoTransfMemoriaInicial=0;

#endif

/*
  Obtém o tempo decorrido entre o início e o fim de um evento (em picosegundos)
*/
float getTempoDecorrido(cl_event event){

    cl_ulong time_start, time_end;

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

    return time_end - time_start;
}

void CriaSubDevices(){

    /*//-----------------------
    // Criação dos subdevices
    //-----------------------
    status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_compute_units, NULL);

    cl_uint ncores = std::min((int)max_compute_units, (int)pcores);

    //cout << "max comput units:" << ncores << endl;

    const cl_device_partition_property subdevice_properties[] =
    { CL_DEVICE_PARTITION_BY_COUNTS,
        ncores, 0, CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0 };

    cl_device_id device_ids[1];

    numSubDevices=1;

    status = clCreateSubDevices(devices[0], subdevice_properties, numSubDevices, devices, NULL);

    device = devices[0];

    check_cl(status, "Erro ao criar os subdevices");*/
}

void compila_programa(char * funcaoObjetivo){
    
    cl_ulong max_constant_buffer_size;

    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong),
                                 &max_constant_buffer_size, NULL);

     std::string header_string ="#define DIMENSOES_PROBLEMA " + ToString(DIMENSOES_PROBLEMA)  + "\n" +
                                "#define TAMANHO_POPULACAO " + ToString(TAMANHO_POPULACAO) + "\n" +
                                "#define TAMANHO_VALOR " + ToString(TAMANHO_VALOR) + "\n" +
                                "#define TAMANHO_INDIVIDUO DIMENSOES_PROBLEMA*TAMANHO_VALOR  " + "\n" +                               
                                "#define TAMANHO_GRAMATICA " + ToString(5) + "\n" +
                                "#define TAMANHO_DATABASE " + ToString(tamanhoBancoDeDados) + "\n" +
                                "#define NUM_VARIAVEIS " + ToString(2) + "\n" + 
                                "#define FUNCAO_OBJETIVO(x1) " + (funcaoObjetivo) + "\n";

    long constant_size = sizeof(t_regra)*5 + (tamanhoBancoDeDados * sizeof(cl_float));

    if(constant_size > max_constant_buffer_size )
        header_string += " #define Y_DOES_NOT_FIT_IN_CONSTANT_BUFFER \n ";

    header_string += " #define LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2 "
                      + ToString( next_power_of_2( max_local_size) ) + " \n ";

    if(tamanhoBancoDeDados % max_local_size != 0 )
        header_string += " #define NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE \n";

    if( is_power_of_2( max_local_size ) )
        header_string += " #define LOCAL_SIZE_IS_NOT_POWER_OF_2 \n ";

    std::string body_string   = LoadKernel("avaliacao.cl");
    std::string kernel_string = header_string + body_string;

    kernel_srt = kernel_string.c_str();

 	size_t programSize = (size_t)strlen(kernel_srt);

	//Cria o programa
	program = clCreateProgramWithSource(context,
							   1,
							   (const char **)&kernel_srt,
							   &programSize,
							   &status);

    check_cl(status, "Erro ao criar o programa");

    double start  = getRealTime();

	//Compilação do programa
	status = clBuildProgram(program,
                            1,
				            devices,
				            "",
				            NULL,
				            NULL);

    if(status != CL_SUCCESS){

        // Exibe os erros de compilação
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog), buildLog, NULL);

        printf("Erros no kernel: \n %s \n", buildLog);

        clReleaseProgram(program);
    }

    //printf("\nTempo de compilação: %lf\n", getRealTime()-start);

    check_cl(status, "Erro ao compilar o programa");    
    
     //---------------------------------------------
	// 7: Criação do kernel
	//---------------------------------------------

	//Cria o kernel de avaliação

    if(CPU){
        kernelAvaliacao = clCreateKernel(program, "avaliacao", &status);
    }
    else{
        kernelAvaliacao = clCreateKernel(program, "avaliacao_gpu", &status);
    }

    check_cl(status, "Erro ao criar kernel 'avaliacao'");    
    

}

void opencl_init(Database *dataBase){

    #ifdef cpu
        CPU = 1;
    #else
        CPU = 0;
    #endif

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

    check_cl(status, "Erro ao obter as plataformas disponiveis");

   	//---------------------------------------------
	// 2: Descoberta e inicialização do(s) dispositivo(s)
	//---------------------------------------------

	//Obtém o número de dispositivos na plataforma de índice 0
	status = clGetDeviceIDs(platforms[0],
							CL_DEVICE_TYPE_ALL,
							0,
							NULL,
							&numDevices);

    check_cl(status, "Erro ao obter os dispositivos");
    

    //Aloca espaço para cada dispositivo
	devices = (cl_device_id*) malloc(numDevices*sizeof(cl_device_id));

	//Obtém os dispositivos
	status = clGetDeviceIDs(platforms[0],
							CL_DEVICE_TYPE_ALL,
							numDevices,
							devices,
							NULL);
							
    check_cl(status, "Erro ao obter os dispositivos");

    //---------------------------------------------
	// 3: Criação do contexto de execução
	//---------------------------------------------

	//Cria o contexto de execução, associando os dispositivos
	context = clCreateContext(NULL,
							 1,
						 	 devices,
						 	 NULL,
						 	 NULL,
						 	 &status);
    check_cl(status, "Erro ao criar o contexto de execução");

    clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_local_size, NULL);

    //if(pcores>0) CriaSubDevices();

    device = devices[0];

    /* Consulta as propriedades do dispositivo */

    clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_local_size, NULL);

    //-----------------------
    // 4: Criaçaõ da fila de execução
    //---------------------------------------------

    cmdQueue = clCreateCommandQueue(context,
									device,
                                    CL_QUEUE_PROFILING_ENABLE,
									&status);

    check_cl(status, "Erro ao criar a fila de execução");


    //-----------------------------------=----------
    // 5: Criação e compilação do programa
    //----------------------------------------------

    //compila_programa(dataBase, "2*(x1*x1)");

    //---------------------------------------------
	// 6: Criação dos buffers de memória
	//---------------------------------------------

    datasize = sizeof(individuo)*TAMANHO_POPULACAO;

	bufferA = clCreateBuffer(context, CL_MEM_READ_WRITE, TAMANHO_POPULACAO * sizeof(t_prog), NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferA");
    
   	bufferFitness = clCreateBuffer(context, CL_MEM_READ_WRITE, TAMANHO_POPULACAO * sizeof(float), NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferFitness");

    //bufferProgramas = clCreateBuffer(context, CL_MEM_READ_WRITE, TAMANHO_POPULACAO*sizeof(t_prog), NULL, &status);
    bufferGramatica = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(t_regra)*5, NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferGramatica");

    bufferDatabase  = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*(dataBase->numRegistros)*(dataBase->numVariaveis), NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferDatabase");   
}


void carrega_gramatica(t_regra * gramatica){

    cl_event eventoCargaGramatica;

    status = clEnqueueWriteBuffer(cmdQueue,
                                      bufferGramatica,
                                      CL_FALSE,
                                      0,
                                      sizeof(t_regra)*5,
                                      gramatica,
                                      0,
                                      NULL,
                                      &eventoCargaGramatica);
    check_cl(status, "Erro ao carregar o buffer da gramatica");

    //Espera o término da fila de execução
    status = clFinish(cmdQueue);
    check_cl(status, "Erro ao carregar o buffer da gramatica");

    #ifdef PROFILING

    float tempoCarga = getTempoDecorrido(eventoCargaGramatica) / 1000000000.0 ;
    tempoTotal += tempoCarga;
    tempoTotalTransfMemoria += tempoCarga;
    tempoTransfMemoriaInicial += tempoCarga;

    #endif
}


void carrega_bancoDeDados(Database *dataBase){

    cl_event eventoCargaBanco;

    status = clEnqueueWriteBuffer(cmdQueue,
                                  bufferDatabase,
                                  CL_FALSE,
                                  0,
                                  sizeof(float)*(dataBase->numRegistros)*(dataBase->numVariaveis),
                                  dataBase->registros,
                                  0,
                                  NULL,
                                  &eventoCargaBanco);

    check_cl(status, "Erro ao carregar o buffer do banco de dados");

    //Espera o término da fila de execução
    status = clFinish(cmdQueue);
    check_cl(status, "Erro ao carregar o buffer do banco de dados");

    #ifdef PROFILING

    float tempoCarga = getTempoDecorrido(eventoCargaBanco) / 1000000000.0 ;
    tempoTotal += tempoCarga;
    tempoTotalTransfMemoria += tempoCarga;
    tempoTransfMemoriaInicial += tempoCarga;

    #endif
}


void avaliacao_kernel(float * fitness, cl_mem bufferPop){

    cl_event eventoAvaliacao, eventoLeitura;
    
    status = clSetKernelArg(kernelAvaliacao,  0, sizeof(bufferFitness), &bufferFitness);
    check_cl(status, "Erro ao adicionar argumento ao kernel");

    status = clSetKernelArg(kernelAvaliacao,  1, sizeof(bufferDatabase),  &bufferDatabase);
    check_cl(status, "Erro ao adicionar argumento ao kernel");    
    
    if(!CPU)
    {
        size_t localWorkSize[1];
        size_t globalWorkSize[1];

        if(tamanhoBancoDeDados < max_local_size)
            localWorkSize[0] = tamanhoBancoDeDados;
        else
            localWorkSize[0] = max_local_size;

        // Um indivíduo por work-group
        globalWorkSize[0] = localWorkSize[0] * TAMANHO_POPULACAO;

        status = clSetKernelArg(kernelAvaliacao,  3, sizeof(float)*localWorkSize[0],  NULL);
        check_cl(status, "Erro ao adicionar argumento ao kernel");

        status = clEnqueueNDRangeKernel(cmdQueue,
                               kernelAvaliacao,
                               1,
                               NULL,
                               globalWorkSize,
                               localWorkSize,
                               0,
                               NULL,
                               &eventoAvaliacao);
     }
     else{

        size_t localWorkSize[1] = {1};
        size_t globalWorkSize[1];

        globalWorkSize[0] = ceil((float)TAMANHO_POPULACAO/localWorkSize[0])*localWorkSize[0];

        status = clEnqueueNDRangeKernel(cmdQueue,
                               kernelAvaliacao,
                               1,
                               NULL,
                               globalWorkSize,
                               localWorkSize,
                               0,
                               NULL,
                               &eventoAvaliacao);
     }

     check_cl(status, "Erro ao enfileirar o kernel para execucao");
   
     clFinish(cmdQueue);
     
     status = clEnqueueReadBuffer(cmdQueue, bufferFitness, CL_TRUE, 0, sizeof(float)*TAMANHO_POPULACAO, 
                                    fitness, 0, NULL, &eventoLeitura);

     check_cl(status, "Erro ao enfileirar leitura de buffer de memoria");    
     
     clFinish(cmdQueue);        

     #ifdef PROFILING

        float tempoAvaliacao = getTempoDecorrido(eventoAvaliacao) / 1000000000.0 ;
        tempoTotal += tempoAvaliacao;

        tempoTotalAvaliacao += tempoAvaliacao;
        tempoTotalProcessamento += tempoAvaliacao;

        float tempoLeitura = getTempoDecorrido(eventoLeitura) / 1000000000.0 ;
        tempoTotal += tempoLeitura;
        tempoTotalTransfMemoria += tempoLeitura;

     #endif
}

void avaliacao_init(t_regra *gramatica, Database *dataBase){
    
    check(gramatica != NULL, "A gramática não pode ser nula");
    check(dataBase  != NULL, "o banco de dados não pode ser nulo");
    
    tamanhoBancoDeDados = dataBase->numRegistros;
    
    carrega_gramatica(gramatica);
    carrega_bancoDeDados(dataBase);
}

void avaliacao_paralela(individuo * pop, t_prog * programas){

    check(pop != NULL, "A população não pode ser nula");
  
    float fitness[TAMANHO_POPULACAO];
  
    //avaliacao_kernel(programas, fitness, bufferA);
    
    int i;
    char programaTexto[TAMANHO_MAX_PROGRAMA];
    
    printf("Iniciando a avaliação paralela\n");
    
    for(i=0; i < TAMANHO_POPULACAO; i++){
    
        printf("Compilando o programa %d\n", i);
    
        //Programa inválido
        if(programas[i].programa[0].t.v[0] == -1){
            pop[i].aptidao = 999999999999;    
            printf("Programa inválido\n");    
        }
        else{
    
            GetProgramaInfixo(programas[i].programa, &programaTexto[0]);        
            
            printf("Programa: %s\n", programaTexto);
            
            printf("%d - %f\n", i, fitness[i]);  
            
            compila_programa(programaTexto);
            
            avaliacao_kernel(fitness, bufferA);
               
            pop[i].aptidao = fitness[0];
            
            printf("Aptidao: %f\n", fitness[0]);  
        }
    }
    
    /*#ifdef PROFILING

      //printf("Tempos\nAvaliação:\tProcessamento\tTransf memoria\tTransf memoria inicial\tProc+Memoria\n");

      printf("%.10f\t", tempoTotalAvaliacao);
      printf("%.10f\t", tempoTotalProcessamento);
      printf("%.10f\t", tempoTotalTransfMemoria);
      printf("%.10f\t", tempoTransfMemoriaInicial);
      printf("%.10f\n", tempoTotalProcessamento + tempoTotalTransfMemoria);
      //printf("kernel substituicao: \t %.10f\n", tempoTotalSubstituicao);
      //printf("kernel ag: \t %.10f\n", tempoTotalAG);

   #endif*/
}

void opencl_dispose(){

#ifdef PROFILING

      //printf("Tempos\nAvaliação:\tProcessamento\tTransf memoria\tTransf memoria inicial\tProc+Memoria\n");

      printf("%.10f\t", tempoTotalAvaliacao);
      printf("%.10f\t", tempoTotalProcessamento);
      printf("%.10f\t", tempoTotalTransfMemoria);
      printf("%.10f\t", tempoTransfMemoriaInicial);
      printf("%.10f\n", tempoTotalProcessamento + tempoTotalTransfMemoria);
      //printf("kernel substituicao: \t %.10f\n", tempoTotalSubstituicao);
      //printf("kernel ag: \t %.10f\n", tempoTotalAG);

   #endif

    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferFitness);
    clReleaseMemObject(bufferGramatica);
    clReleaseMemObject(bufferDatabase);
    clReleaseKernel(kernelAvaliacao);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseContext(context);    
}
