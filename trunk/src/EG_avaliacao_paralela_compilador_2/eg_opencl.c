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

int tamanhoBancoDeDados, numVariaveis;

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
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,   sizeof(time_end), &time_end, NULL);

    return time_end - time_start;
}

void compila_programa(individuo * pop, int geracao, t_regra * gramatica ){
    
     if(geracao>1){    
        clReleaseKernel(kernelAvaliacao);
        clReleaseProgram(program);
     }
    
    
    int k;    
    cl_ulong max_constant_buffer_size;
    char programaTexto[10*TAMANHO_MAX_PROGRAMA];

    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong),
                                 &max_constant_buffer_size, NULL);

    std::string header_string = "#define DIMENSOES_PROBLEMA " + ToString(DIMENSOES_PROBLEMA)  + "\n" +
                                "#define TAMANHO_POPULACAO " + ToString(TAMANHO_POPULACAO) + "\n" +
                                "#define TAMANHO_VALOR " + ToString(TAMANHO_VALOR) + "\n" +
                                "#define TAMANHO_INDIVIDUO DIMENSOES_PROBLEMA*TAMANHO_VALOR  " + "\n" +                               
                                "#define TAMANHO_DATABASE " + ToString(tamanhoBancoDeDados) + "\n" +
                                "#define NUM_VARIAVEIS " + ToString(numVariaveis) + "\n"+
 //                             "#define DATABASE(row,column) dataBase[(row)*NUM_VARIAVEIS + (column)] \n";
                                "#define TAMANHO_BLOCO 500 \n" + 
                                "#define DATABASE(row,column) dataBase[(column)*TAMANHO_DATABASE + row] \n";
   
    long constant_size = tamanhoBancoDeDados * sizeof(cl_float);
    if(constant_size > max_constant_buffer_size )
        header_string += " #define Y_DOES_NOT_FIT_IN_CONSTANT_BUFFER \n ";

    header_string += "#define LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2 "
                      + ToString( next_power_of_2( localWorkSize[0]) ) + " \n ";

    if(tamanhoBancoDeDados % localWorkSize[0] != 0 )
        header_string += "#define NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE \n";

    if(is_power_of_2( localWorkSize[0] ) )
        header_string += "#define LOCAL_SIZE_IS_NOT_POWER_OF_2 \n ";    
   
   
    int bloco = 200;
    int iteracoes = ceil((float)TAMANHO_POPULACAO/(float)bloco);
    int l;   
   
   
    std::string body_string   = "";
   
    //Obtém o valor de cada variável no registro
    for(k=1; k < numVariaveis; k++){        
        //body_string += "float x" + ToString(k) + " = DATABASE(line, "+ ToString(k-1)+ "); \n";        
        header_string += "#define x" + ToString(k) + " (DATABASE(line, "+ ToString(k-1)+ ")) \n";                        
    }   
    
    //std::string fitness_string = ToString("float funcaoobjetivo(int p, __global float * dataBase, int line){ \n");      
    //cout << fitness_string << endl;    
          
    t_prog programa;
    short fenotipo[DIMENSOES_PROBLEMA];    
    
    for(l=0; l < iteracoes;l++){
    
        int limiteInferior = bloco*l;
        int limiteSuperior = std::min((l+1)*bloco, TAMANHO_POPULACAO);
        
        body_string += "\n float bloco" + ToString(l) + "(int gid, __global float * dataBase, int line){\n";        
        
        body_string += ToString("switch(gid){ \n");
        
        int li;
        for(li=limiteInferior; li < limiteSuperior; li++){
            
            obtem_fenotipo_individuo(&pop[li], fenotipo);
            int program_ctr = Decodifica(gramatica, fenotipo, programa.programa);       
            
            if(program_ctr != -1){
    
                body_string += ToString(" case "+ToString(li)+": \n");                        
               
                GetProgramaInfixo(programa.programa, &programaTexto[0]);
                body_string += ToString(" return  " + ToString(programaTexto) +  "; break; \n");            
            }                        
        }
        
        body_string += "default: return MAXFLOAT; }\n } \n";
    }
    
     
    body_string += ToString("float funcaoobjetivo(int gid, __global float * dataBase, int line){ \n");
    
    body_string += "int bloco = floor((float)gid/TAMANHO_BLOCO);\n";

    body_string += "switch(bloco){ \n";
  
    for(l=0; l < iteracoes; l++){
        body_string += "case "+ToString(l)+": return bloco"+ToString(l)+"(gid, dataBase, line); \n";        
    }   
        
    body_string += "} \n } \n";
    
    body_string += ToString("__kernel void avaliacao_gpu") + 
    ToString(geracao) + ToString("(") + LoadKernel("avaliacao1.cl");
    
    
    /*body_string += ToString("switch(gid){ \n");

    for(k=0; k<TAMANHO_POPULACAO; k++){
    
         if(pop[k].programa[0].t.v[0] != -1){
    
            body_string += ToString(" case "+ToString(k)+": \n");
           
            GetProgramaInfixo(pop[k].programa, &programaTexto[0]);
            body_string += ToString(" result = " + ToString(programaTexto) +  "; break; \n");            
         }
    }*/
           
    //Caso o programa seja inválido, será selecionado o caso default, que retorna MAXFLOAT
    //body_string+= "default: result = MAXFLOAT; break; \n } \n ";

    
    body_string += LoadKernel("avaliacao2.cl");
    
    std::string kernel_string = header_string + body_string;

    kernel_srt = kernel_string.c_str();
    
    //cout << kernel_srt << endl;    
    
 	size_t programSize = (size_t)strlen(kernel_srt);
 	
// 	cout << "Program size: " << strlen(kernel_srt) << endl << endl;
 	

	//Cria o programa
	
	//if(geracao==1)
    	program = clCreateProgramWithSource(context,
							   1,
							   (const char **)&kernel_srt,
							   &programSize,
							   &status);
    

    check_cl(status, "Erro ao criar o programa");

    double start  = getRealTime();

    printf("Iniciando compilação\n");

	//Compilação do programa    
    //if(geracao==1)
	status = clBuildProgram(program,
                            1,
				            devices,
				            //"-cl-opt-disable",
				            //"",
				            //-cl-nv-maxrregcount=<20>
				            "-cl-nv-verbose",
				            NULL,
				            NULL);
    
    printf("Terminou a compilação\n");

    #ifndef SHOW_BUILD_LOG_OPENCL
        if(status != CL_SUCCESS){   
    #endif

        // Exibe os erros de compilação
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog), buildLog, NULL);

        printf("Build log: \n %s \n", buildLog);       
    
    #ifndef SHOW_BUILD_LOG_OPENCL
        clReleaseProgram(program);
        exit(1);
                
    }
    #endif
    
    #ifdef SHOW_PROGRAM_BINARIES
    
        size_t nb_devices = {1};    

        status = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(size_t), &nb_devices, NULL);
        size_t *np = new size_t[nb_devices];
        status = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t)*nb_devices, np, NULL);
        char** bn = new char* [nb_devices];
        for(int i =0; i < nb_devices;i++)  bn[i] = new char[np[i]]; 
      
        status = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(unsigned char *)*nb_devices, bn, NULL);
      
        printf("%s\n", bn[0]);
    
    #endif

    printf("\nTempo de compilação: %lf\n", getRealTime()-start);

    check_cl(status, "Erro ao compilar o programa");    
    
    //---------------------------------------------
	// 7: Criação do kernel
	//---------------------------------------------
       
    char nomeKernel[30];
    
    sprintf(nomeKernel, "avaliacao_gpu%d", geracao);

	kernelAvaliacao = clCreateKernel(program, nomeKernel, &status);
    check_cl(status, "Erro ao criar kernel 'avaliacao'");
}

void opencl_init(Database *dataBase){

    #ifdef cpu
        CPU = 1;
    #else
        CPU = 0;
    #endif
    
    tamanhoBancoDeDados = dataBase->numRegistros;
    numVariaveis = dataBase->numVariaveis;

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
    
    //---------------------------------------------
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

void avaliacao_paralela(individuo * pop, t_prog * programas, int geracao, t_regra * gramatica){
    
    check(pop != NULL, "A população não pode ser nula");

    //printf("Iniciando a avaliação paralela...\n");    
    //printf("Compilando a população...\n");
    
    printf("1.1\n");
    
    compila_programa(pop, geracao, gramatica);
    
    printf("1.2\n");
    
    avaliacao_kernel(fitness, bufferA);
    
    printf("1.3\n");
    
    int i;
    for(i=0; i < TAMANHO_POPULACAO; i++){        
        pop[i].aptidao = fitness[i];        
        //printf("%d => %f\n", i, fitness[i]);
    }
}

void opencl_dispose(){

    #ifdef PROFILING
        //printf("Tempos\nAvaliação:\tProcessamento\tTransf memoria\tTransf memoria inicial\tProc+Memoria\n");
        printf("%.10f\t", tempoTotalAvaliacao);
        /* printf("%.10f\t", tempoTotalProcessamento);
        printf("%.10f\t", tempoTotalTransfMemoria);
        printf("%.10f\t", tempoTransfMemoriaInicial);
        printf("%.10f\n", tempoTotalProcessamento + tempoTotalTransfMemoria);*/
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
