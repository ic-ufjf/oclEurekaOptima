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

float fitness[TAMANHO_POPULACAO];

cl_mem bufferA, bufferProgramas, bufferFitness, bufferGramatica, bufferDatabase;

int tamanhoBancoDeDados;

//Eventos utilizados para medir o tempo de execução do kernel
//e das trocas de memória
cl_event event1, event2, event3;

size_t preferred_workgroup_size_multiple, max_local_size;

int num_workgroups;

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

size_t localWorkSize[1];
size_t globalWorkSize[1];


/*
  Obtém o tempo decorrido entre o início e o fim de um evento (em picosegundos)
*/
float getTempoDecorrido(cl_event event){

    cl_ulong time_start, time_end;

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);

    return time_end - time_start;
}

void compila_programa(t_prog * pop){
    
    cl_ulong max_constant_buffer_size;

    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong),
                                 &max_constant_buffer_size, NULL);

    std::string header_string ="#define DIMENSOES_PROBLEMA " + ToString(DIMENSOES_PROBLEMA)  + "\n" +
                                "#define TAMANHO_POPULACAO " + ToString(TAMANHO_POPULACAO) + "\n" +
                                "#define TAMANHO_VALOR " + ToString(TAMANHO_VALOR) + "\n" +
                                "#define TAMANHO_INDIVIDUO DIMENSOES_PROBLEMA*TAMANHO_VALOR  " + "\n" +                               
                                "#define TAMANHO_GRAMATICA " + ToString(5) + "\n" +
                                "#define TAMANHO_DATABASE " + ToString(tamanhoBancoDeDados) + "\n" +
                                "#define DATABASE(x,y) dataBase[x*NUM_VARIAVEIS + y] \n"+
                                "#define NUM_VARIAVEIS " + ToString(2) + "\n";
    
    long constant_size = tamanhoBancoDeDados * sizeof(cl_float);

    if(constant_size > max_constant_buffer_size )
        header_string += " #define Y_DOES_NOT_FIT_IN_CONSTANT_BUFFER \n ";

    header_string += " #define LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2 "
                      + ToString( next_power_of_2( localWorkSize[0]) ) + " \n ";

    if(tamanhoBancoDeDados % localWorkSize[0] != 0 )
        header_string += " #define NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE \n";

    if(is_power_of_2( localWorkSize[0] ) )
        header_string += " #define LOCAL_SIZE_IS_NOT_POWER_OF_2 \n ";
    
    char programaTexto[TAMANHO_MAX_PROGRAMA];          
    int k;                       
  
     for(k=0; k<TAMANHO_POPULACAO; k++){
    
         if(pop[k].programa[0].t.v[0] != -1){
            
            GetProgramaInfixo(pop[k].programa, &programaTexto[0]);
            
            std::string avaliacao_string = 
            
            ToString(" void avalia") +ToString(k) + ToString("(int tid, int lid, int gid, int local_size, __global float * dataBase, __local float * erros ){\n")+
            ToString("      erros[lid] = 0;  		  \n")+
            ToString("      #ifndef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE \n")+
            ToString(" 	    for( uint iter = 0; iter < TAMANHO_DATABASE/local_size; ++iter ){	    \n")+
            ToString(" 	  #else \n")+
            ToString(" 	  for( uint iter = 0; iter < ceil( TAMANHO_DATABASE / (float) local_size ); ++iter ){	    \n")+
            ToString("    if( iter * local_size + lid < TAMANHO_DATABASE){           \n")+
            ToString(" 	  #endif	 \n")+
            ToString(" 	  uint line = iter * local_size + lid; \n")+
            //ToString(" 	  float x1 = DATABASE(line, 0); \n")+
            ToString(" 	  #define x1 DATABASE(line, 0) \n") +
            ToString(" 	  float result = ") + ToString(programaTexto) + ToString(";\n")+
            ToString(" 	  if(isnan(result) || isinf(result))erros[lid] = MAXFLOAT;            \n")+
            ToString("    else erros[lid] += pown(fabs(result-DATABASE(line, NUM_VARIAVEIS-1)), 2);            \n")+
            ToString("    #ifdef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE \n")+
            ToString("    } \n")+
            ToString("    #endif\n")+
            ToString("    } \n")+ 
            ToString("    } \n");    

            header_string+= avaliacao_string;
        }
    }                           
                               
    std::string fitness_string = "void avalia(int tid, int lid, int gid, int local_size, __global float * dataBase, __local float * erros ){    \n   switch(gid){ \n";
              
    for(k=0; k<TAMANHO_POPULACAO; k++){
    
         if(pop[k].programa[0].t.v[0] != -1){
    
            fitness_string += ToString(" case "+ToString(k)+": \n");            
            fitness_string += ToString(" avalia" + ToString(k) +  "(tid,lid,gid, local_size, dataBase, erros); \n break; \n");            
        }
    }
        
    fitness_string+= "default: erros[0] = MAXFLOAT; break; \n } \n } \n";              
    
    std::string body_string   = LoadKernel("avaliacao.cl");
    std::string kernel_string = header_string  + fitness_string+ body_string;

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
    }

    printf("\nTempo de compilação: %lf\n", getRealTime()-start);

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
    
    tamanhoBancoDeDados = dataBase->numRegistros;

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

    //if(pcores>0) CriaSubDevices();

    device = devices[0];

    /* Consulta as propriedades do dispositivo */

    clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_compute_units, NULL);    
    clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_local_size, NULL);

    if(max_local_size>1024) max_local_size = 1024;    
    
    if(tamanhoBancoDeDados < max_local_size)
        localWorkSize[0] = tamanhoBancoDeDados;
    else
        localWorkSize[0] = max_local_size;        
  
    // Um indivíduo por work-group
    globalWorkSize[0] = localWorkSize[0] * TAMANHO_POPULACAO;   

    printf("local: %ld, global: %ld, grupos:%ld\n", localWorkSize[0], 
    globalWorkSize[0],  globalWorkSize[0]/localWorkSize[0]);       
    
    
    //-----------------------
    // 4: Criação da fila de execução
    //---------------------------------------------

    cmdQueue = clCreateCommandQueue(context,
									device,
                                    CL_QUEUE_PROFILING_ENABLE,
									&status);

    check_cl(status, "Erro ao criar a fila de execução");


    //---------------------------------------------
	// 6: Criação dos buffers de memória
	//---------------------------------------------
	
    
   	bufferFitness = clCreateBuffer(context, CL_MEM_READ_WRITE, TAMANHO_POPULACAO * sizeof(float), NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferFitness");

    bufferDatabase  = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float)*(dataBase->numRegistros)*(dataBase->numVariaveis), NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferDatabase");   
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
    
    status = clSetKernelArg(kernelAvaliacao,  2, sizeof(float)*localWorkSize[0],  NULL);
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
    

     check_cl(status, "Erro ao enfileirar o kernel para execucao");
    
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
    
    carrega_bancoDeDados(dataBase);
}

void avaliacao_paralela(individuo * pop, t_prog * programas){
    
    check(pop != NULL, "A população não pode ser nula");

    printf("Iniciando a avaliação paralela...\n");    
    printf("Compilando a população...\n");
    
    compila_programa(programas);
        
    avaliacao_kernel(fitness, bufferA);
    
    int i;
    for(i=0; i < TAMANHO_POPULACAO; i++){
        
        //Programa inválido
        if(programas[i].programa[0].t.v[0] == -1){
            pop[i].aptidao = -99999999999999;    
        }
        else{
            pop[i].aptidao = fitness[i];
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
