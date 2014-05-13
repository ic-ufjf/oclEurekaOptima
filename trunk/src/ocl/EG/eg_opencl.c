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

#define NUM 30000

#ifndef INT64_C
    #define INT64_C(c) (c ## LL)
    #define UINT64_C(c) (c ## ULL)
#endif

#define KERNEL_2_POR_WORK_ITEM 0
#define KERNEL_2_POR_WORK_GROUP 1
#define KERNEL_N_POR_WORK_GROUP 2
#define KERNEL_2_POR_WORK_ITEM_GRUPO_FIXO 3
#define KERNEL_1_POR_WORK_ITEM 4
#define KERNEL_2_POR_WORK_GROUP2 5

#include "Random123/array.h"
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

int geracao = 1;

//Utilizado para tratamento de erros
cl_int status;

cl_device_id device;
cl_uint max_compute_units;


//Número de cores informado por parâmetro
int pcores;

//Indica se o dispositivo é cpu
short CPU;

//Kernel selecionado para as iterações do AG
int kernelAG;

int geracaoMelhor = 0;

cl_command_queue cmdQueue;
cl_context context = NULL;
cl_device_id * devices = NULL, * subDevices = NULL;
cl_uint numDevices = 0, numSubDevices=0;
cl_kernel kernelIteracao, kernelAvaliacao, kernelInicializacao, kernelSubstituicao;
cl_program program;

size_t globalWorkSizeIteracao[1],localWorkSizeIteracao[1];
size_t datasize;

cl_mem bufferA, bufferB, bufferC, bufferCounter, bufferProgramas, bufferGramatica, bufferDatabase;

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

    //-----------------------
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

    check_cl(status, "Erro ao criar os subdevices");
}

int binario_para_decimal2(short *binarios, int inicio, int fim){

    int i,n=1; int valorNumerico=0;

    for(i=fim-1; i>=inicio; i--, n=n<<1){

        valorNumerico += n*(binarios[i]);
    }

    return valorNumerico;
}

void gray_para_binario2(short *gray, short*binarios){

    int i,j;

    for(i=0; i< TAMANHO_INDIVIDUO; i++){
        binarios[i] = gray[i];
    }

    int start;
    int end = 0;
    for (j = 0; j < DIMENSOES_PROBLEMA; j++) {
        start = end;
        end += TAMANHO_VALOR;
        for (i = start + 1; i < end; i++) {
            binarios[i] = binarios[i - 1] ^ binarios[i];
        }
    }
}

void obtem_fenotipo_individuo2(individuo *p, short fenotipo[]){

    int i, j=0;

    short genotipo_binario[TAMANHO_INDIVIDUO];

    gray_para_binario2(p->genotipo, genotipo_binario);
    //binario_para_inteiro(p->genotipo_binario, p->fenotipo);

    for(i=0; i<DIMENSOES_PROBLEMA; i++, j+=TAMANHO_VALOR){

       fenotipo[i] = binario_para_decimal2(genotipo_binario, j, j+TAMANHO_VALOR);

    }
}


void opencl_init(int cores, int kernel, Database *dataBase){

    kernelAG = kernel;
    pcores   = cores;

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

	//cout << "Número de plataformas:" << numPlatforms << endl;

	//Aloca espaço para cada plataforma
	platforms = (cl_platform_id*) malloc(numPlatforms*sizeof(cl_platform_id));

	//Obtém as plataformas
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);

    check_cl(status, "Erro ao obter as plataformas disponiveis");

    //printf("\nPlataformas: %d\n", numPlatforms);

   	//---------------------------------------------
	// 2: Descoberta e inicialização do(s) dispositivo(s)
	//---------------------------------------------

	//Obtém o número de dispositivos na plataforma de índice 0
	status = clGetDeviceIDs(platforms[1],
							CL_DEVICE_TYPE_ALL,
							0,
							NULL,
							&numDevices);

    check_cl(status, "Erro ao obter os dispositivos");


    //Aloca espaço para cada dispositivo
	devices = (cl_device_id*) malloc(numDevices*sizeof(cl_device_id));

	//Obtém os dispositivos
	status = clGetDeviceIDs(platforms[1],
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


    if(pcores>0)
        CriaSubDevices();

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

    cl_ulong max_constant_buffer_size;

    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong),
                                 &max_constant_buffer_size, NULL);

    //printf("Max constant buffer size: %ld \n", (long)max_constant_buffer_size);

    std::string header_string = "#define NUMERO_DE_GERACOES " + ToString(NUMERO_DE_GERACOES) + " \n"+
                                "#define DIMENSOES_PROBLEMA " + ToString(DIMENSOES_PROBLEMA)  + "\n" +
                                "#define TAMANHO_POPULACAO " + ToString(TAMANHO_POPULACAO) + "\n" +
                                "#define TAMANHO_VALOR " + ToString(TAMANHO_VALOR) + "\n" +
                                "#define TAMANHO_INDIVIDUO DIMENSOES_PROBLEMA*TAMANHO_VALOR  " + "\n" +
                                "#define TAXA_DE_MUTACAO " + ToString(TAXA_DE_MUTACAO) + "\n" +
                                "#define TAXA_DE_RECOMBINACAO " + ToString(TAXA_DE_RECOMBINACAO )+ "\n" +
                                "#define TAMANHO_TORNEIO " + ToString(TAMANHO_TORNEIO) + "\n" +
                                "#define TAMANHO_GRAMATICA " + ToString(5) + "\n" +
                                "#define TAMANHO_DATABASE " + ToString(dataBase->numRegistros) + "\n" +
                                "#define NUM_VARIAVEIS " + ToString(dataBase->numVariaveis) + "\n" +
                                "#define ELITE " + ToString(ELITE) + "\n"+
                                "#define IH " + ToString(getRealTime())+"\n";


    long constant_size = sizeof(t_regra)*5 + (dataBase->numRegistros * sizeof(cl_float));

    if(constant_size > max_constant_buffer_size )
        header_string += " #define Y_DOES_NOT_FIT_IN_CONSTANT_BUFFER \n ";

    header_string += " #define LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2 "
                      + ToString( next_power_of_2( max_local_size) ) + " \n ";

    if(dataBase->numRegistros % max_local_size != 0 )
        header_string += " #define NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE \n";

    if( is_power_of_2( max_local_size ) )
        header_string += " #define LOCAL_SIZE_IS_NOT_POWER_OF_2 \n ";

    std::string body_string   = LoadKernel("kernel.cl");
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
				"-I ./ -I ./include -I include/Random123 -I include/vsmc",
				NULL,
				NULL);

    if(status != CL_SUCCESS){

        // Determine the reason for the error
        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog), buildLog, NULL);

        printf("Erros no kernel: \n %s \n", buildLog);

        clReleaseProgram(program);
    }

    //printf("\nTempo de compilação: %lf\n", getRealTime()-start);

    check_cl(status, "Erro ao compilar o programa");

    //---------------------------------------------
	// 6: Criação dos buffers de memória
	//---------------------------------------------

    datasize = sizeof(individuo)*TAMANHO_POPULACAO;

	bufferA = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferA");

    bufferB = clCreateBuffer(context, CL_MEM_READ_WRITE , datasize, NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferB");

	bufferC = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
	check_cl(status, "Erro ao criar buffer de memoria bufferC");

    //bufferProgramas = clCreateBuffer(context, CL_MEM_READ_WRITE, TAMANHO_POPULACAO*sizeof(t_prog), NULL, &status);
    bufferGramatica = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(t_regra)*5, NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferGramatica");

    bufferDatabase  = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*(dataBase->numRegistros)*(dataBase->numVariaveis), NULL, &status);
    check_cl(status, "Erro ao criar buffer de memoria bufferDatabase");

    if(kernelAG == KERNEL_2_POR_WORK_GROUP2){
        bufferCounter = clCreateBuffer(context, CL_MEM_READ_WRITE , TAMANHO_POPULACAO * sizeof(r123array4x32) * 4, NULL, &status);
    }
    else{
        bufferCounter = clCreateBuffer(context, CL_MEM_READ_WRITE , TAMANHO_POPULACAO * sizeof(r123array4x32), NULL, &status);
    }

    check_cl(status, "Erro ao criar buffer de memoria bufferCounter");

    //---------------------------------------------
	// 7: Criação do kernel
	//---------------------------------------------

    kernelInicializacao = clCreateKernel(program, "inicializa_populacao", &status);

    check_cl(status, "Erro ao criar kernel kernelInicializacao");

    if(CPU){

        status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint),
                                 &max_compute_units, NULL);

        preferred_workgroup_size_multiple = (size_t)max_compute_units;

    }
    else{

        //Obtém o tamanho  do qual o groupsize deve ser múltiplo
        status = clGetKernelWorkGroupInfo(kernelInicializacao, device,
                                          CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                          sizeof(size_t),
                                          &preferred_workgroup_size_multiple,
                                          NULL);
    }

	//Cria o kernel de mutação/recombinação

	if(kernelAG==KERNEL_2_POR_WORK_ITEM){

        kernelIteracao = clCreateKernel(program, "iteracao_2_por_work_item", &status);

        globalWorkSizeIteracao[0] = TAMANHO_POPULACAO/2;

    }
    else if(kernelAG==KERNEL_1_POR_WORK_ITEM){

        kernelIteracao = clCreateKernel(program, "iteracao_1_por_work_item", &status);
        globalWorkSizeIteracao[0] = TAMANHO_POPULACAO;

    }
	else if(kernelAG==KERNEL_2_POR_WORK_GROUP){

        kernelIteracao = clCreateKernel(program, "iteracao_2_por_work_group", &status);

        globalWorkSizeIteracao[0] = TAMANHO_POPULACAO;
        localWorkSizeIteracao[0]  = 2;
    }

    else if(kernelAG==KERNEL_2_POR_WORK_GROUP2){

        kernelIteracao = clCreateKernel(program, "iteracao_2_por_work_group2", &status);

         localWorkSizeIteracao[0]  = preferred_workgroup_size_multiple;
        globalWorkSizeIteracao[0] = ceil((TAMANHO_POPULACAO/2)*localWorkSizeIteracao[0]);
    }

    else if(kernelAG==KERNEL_N_POR_WORK_GROUP){

        kernelIteracao = clCreateKernel(program, "iteracao_n_por_work_group", &status);

        localWorkSizeIteracao[0]  = preferred_workgroup_size_multiple;
        globalWorkSizeIteracao[0] = ceil((float)TAMANHO_POPULACAO/localWorkSizeIteracao[0])*localWorkSizeIteracao[0];

    }

    else if(kernelAG == KERNEL_2_POR_WORK_ITEM_GRUPO_FIXO){

        kernelIteracao = clCreateKernel(program, "iteracao_2_por_work_item_grupo_fixo", &status);

        float numberWorkItens = TAMANHO_POPULACAO/2;

        localWorkSizeIteracao[0]  = preferred_workgroup_size_multiple;
        globalWorkSizeIteracao[0] = ceil((float)numberWorkItens/localWorkSizeIteracao[0])*localWorkSizeIteracao[0];
    }

    else{
        log_error("Kernel inválido");
        exit(EXIT_FAILURE);
    }
    check_cl(status, "Erro ao criar kernel 'iteracao'");

    //Cria o kernel de avaliação

    if(CPU){
        kernelAvaliacao = clCreateKernel(program, "avaliacao", &status);
    }
    else{
        kernelAvaliacao = clCreateKernel(program, "avaliacao_gpu", &status);
    }

    check_cl(status, "Erro ao criar kernel 'avaliacao'");

    kernelSubstituicao = clCreateKernel(program, "substituicao", &status);
    check_cl(status, "Erro ao criar kernel 'substituicao'");
}

void exibe_melhor(individuo * melhor, t_regra * gramatica){

    check(melhor != 0, "Indivíduo não pode ser nulo");
    check(gramatica != 0, "Gramática não pode ser nula");

    /*if(melhor->aptidao==0){
        printf("Geracoes para encontrar o melhor: \t %d\n", geracao);

        int i;

        short genotipo_binario[TAMANHO_INDIVIDUO];

        exit(EXIT_SUCCESS);
    }
    */

    printf("---------------------------------");
    printf("\nGeracao %d: \n", geracao);
    printf("\nMelhor da geracao: %d: %.10f\n", geracao, melhor->aptidao);

    t_item_programa programa[TAMANHO_MAX_PROGRAMA];
    short fenotipo[DIMENSOES_PROBLEMA];

    obtem_fenotipo_individuo2(melhor, fenotipo);

    Decodifica(gramatica, fenotipo, programa);

    printf("\n");
    ImprimePosfixa(programa);

    printf("\nEm ordem infixa:");

    ImprimeInfixa(programa);
    printf("\n");
}

void exibePopulacao(individuo * populacao){

    int i,j;

    printf("\n");

    for(i=0; i < TAMANHO_POPULACAO; i++){

        for(j=0; j < TAMANHO_INDIVIDUO; j++){
               printf(" %d ", populacao[i].genotipo[j]);
        }
        printf("%d \t %f \n",i, populacao[i].aptidao);
    }
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


void inicializa_populacao(individuo * pop){

    //srand(12);

    seed = rand();

    /*int i,j;

    for(i=0; i < TAMANHO_POPULACAO; i++){

         for(j=0; j< TAMANHO_INDIVIDUO; j++){
            pop[i].genotipo[j] = 0;
         }
    }*/

    cl_event eventoInicializacao;

    status = clSetKernelArg(kernelInicializacao,
							0,
							sizeof(bufferA),
							&bufferA);
    check_cl(status, "Erro ao adicionar 0 argumento ao kernel");


    status = clSetKernelArg(kernelInicializacao,
							1,
							sizeof(seed),
							&seed);
    check_cl(status, "Erro ao adicionar 1 argumento ao kernel");

    /*//Transfere a população para o bufferA
    status = clEnqueueWriteBuffer(cmdQueue,
								  bufferA,
								  CL_FALSE,
								  0,
								  datasize,
								  pop,
								  0,
								  NULL,
								  &event1);
     check_cl(status, "Erro ao enfileirar escrita do buffer de memoria");

     status = clEnqueueWriteBuffer(cmdQueue,
								  bufferB,
								  CL_FALSE,
								  0,
								  datasize,
								  pop,
								  0,
								  NULL,
								  &event1);
    check_cl(status, "Erro ao enfileirar escrita do buffer de memoria");

    status = clEnqueueWriteBuffer(cmdQueue,
								  bufferC,
								  CL_FALSE,
								  0,
								  datasize,
								  pop,
								  0,
								  NULL,
								  &event1);
    check_cl(status, "Erro ao enfileirar escrita do buffer de memoria");
    */

    size_t globalWorkSize[1] = {TAMANHO_POPULACAO};

    status = clEnqueueNDRangeKernel(cmdQueue,
									kernelInicializacao,
									1,
                                    NULL,
									globalWorkSize,
									NULL,
									0,
									NULL,
									&eventoInicializacao);
    check_cl(status, "Erro ao enfileirar o kernel para execucao");


    /*status = clEnqueueReadBuffer(cmdQueue,
						bufferA,
						CL_TRUE,
						0,
						datasize,
						pop,
						0,
						NULL,
						&event3);
    check_cl(status, "Erro ao enfileirar leitura do buffer de memoria");*/


    //Espera o término da fila de execução
	clFinish(cmdQueue);

	#ifdef PROFILING

      float tempoInicializacao = getTempoDecorrido(eventoInicializacao) / 1000000000.0 ;
      tempoTotal += tempoInicializacao;
      tempoTotalProcessamento += tempoInicializacao;

    #endif
}

void avaliacao(individuo *pop, t_regra * gramatica, cl_mem bufferPop){

    cl_event eventoAvaliacao;

    status = clSetKernelArg(kernelAvaliacao,  0, sizeof(bufferPop), &bufferPop);
    check_cl(status, "Erro ao adicionar argumento ao kernel");

    status = clSetKernelArg(kernelAvaliacao,  1, sizeof(bufferGramatica), &bufferGramatica);
    check_cl(status, "Erro ao adicionar argumento ao kernel");

    status = clSetKernelArg(kernelAvaliacao,  2, sizeof(bufferDatabase),  &bufferDatabase);
    check_cl(status, "Erro ao adicionar argumento ao kernel");

    if(!CPU)
    {
        size_t localWorkSize[1];
        size_t globalWorkSize[1];

        //printf("max local size: %d\n", (int)max_local_size);

        if(tamanhoBancoDeDados < max_local_size)
            localWorkSize[0] = tamanhoBancoDeDados;
        else
            localWorkSize[0] = max_local_size;

        // Um indivíduo por work-group
        globalWorkSize[0] = localWorkSize[0] * TAMANHO_POPULACAO;

        //printf("Local: %ld, Global: %ld, Grupos: %ld\n",localWorkSize[0], globalWorkSize[0], globalWorkSize[0]/localWorkSize[0]);

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

     /* status = clEnqueueReadBuffer(cmdQueue, bufferPop, CL_TRUE, 0,
						 datasize, pop, 0, NULL, &event3);

     check_cl(status, "Erro ao enfileirar leitura de buffer de memoria");*/


     clFinish(cmdQueue);

     #ifdef PROFILING

        float tempoAvaliacao = getTempoDecorrido(eventoAvaliacao) / 1000000000.0 ;
        tempoTotal += tempoAvaliacao;

        //printf("Tempo de avaliacao: \t %.10f\n", tempoAvaliacao);

        tempoTotalAvaliacao += tempoAvaliacao;
        tempoTotalProcessamento += tempoAvaliacao;

     #endif

     //Imprime todos os indivíduos

     /*int i,j;

     for(i=0; i < TAMANHO_POPULACAO; i++){

        printf("Individuo %d:\n", i);

        printf("\nAptidao: %f\n", pop[i].aptidao );

        // for(j=0;j<TAMANHO_INDIVIDUO;j++){
           // printf("%d", pop[i].genotipo[j]);
        // }

        t_item_programa programa[TAMANHO_MAX_PROGRAMA];

        short fenotipo[DIMENSOES_PROBLEMA];

        obtem_fenotipo_individuo2(&pop[i], fenotipo);

        //printf("Fenotipo:");
        //for(j=0;j<DIMENSOES_PROBLEMA;j++){
         //   printf("%d,", fenotipo[j]);
        //

        Decodifica(gramatica, fenotipo, programa);

        ImprimePosfixa(programa);
        puts("Em ordem infixa:");
        ImprimeInfixa(programa);

        printf("\n");
     }*/
}

void substituicao(individuo *pop, t_regra * gramatica){

    cl_event eventoSubstituicao;

    status = clSetKernelArg(kernelSubstituicao,  0, sizeof(bufferA), &bufferA);
    check_cl(status, "Erro ao adicionar argumento 0 ao kernel");

    status = clSetKernelArg(kernelSubstituicao,  1, sizeof(bufferB), &bufferB);
    check_cl(status, "Erro ao adicionar argumento 1 ao kernel");

    status = clSetKernelArg(kernelSubstituicao,  2, sizeof(bufferC), &bufferC);
    check_cl(status, "Erro ao adicionar argumento 2 ao kernel");

    size_t globalWorkSize[1] = {TAMANHO_POPULACAO};

    clEnqueueNDRangeKernel(cmdQueue,
                               kernelSubstituicao,
                                1,
                               NULL,
                               globalWorkSize,
                               NULL,
                               0,
                               NULL,
                               &eventoSubstituicao);

    check_cl(status, "Erro ao enfileirar o kernel para execucao");

    /*
    Troca os buffers A e C, de forma que a população gerada na etapa de substituição
    passe a ser a população atual
    */

    cl_mem aux = bufferA;
    bufferA = bufferC;
    bufferC = aux;

    individuo melhor1[1], melhor2[1];

    size_t offset = {sizeof(individuo) * ELITE};

    /*clEnqueueReadBuffer(cmdQueue, bufferA, CL_TRUE, 0,
                        datasize, pop,
                        0, NULL, &event3);
    */

    cl_event eventoTroca1, eventoTroca2;

    check_cl(status, "Erro ao enfileirar leitura de buffer de memória");

    clEnqueueReadBuffer(cmdQueue, bufferA, CL_FALSE, 0,
                        sizeof(individuo), melhor1,
                        0, NULL, &eventoTroca1);
    check_cl(status, "Erro ao enfileirar leitura de buffer de memória");


    clEnqueueReadBuffer(cmdQueue, bufferA, CL_FALSE, offset,
                        sizeof(individuo), melhor2,
                        0, NULL, &eventoTroca2);
    check_cl(status, "Erro ao enfileirar leitura de buffer de memória");

    clFinish(cmdQueue);

    if(melhor1[0].aptidao > melhor2[0].aptidao){
       exibe_melhor(melhor1, gramatica);
    }
    else{
       exibe_melhor(melhor2, gramatica);
    }

    #ifdef PROFILING

    float tempoSubs = getTempoDecorrido(eventoSubstituicao) / 1000000000.0 ;
    //tempoTotal += tempoSubs;
    tempoTotalProcessamento += tempoSubs;

    //printf("Tempo de substituicao: \t %.10f\n", tempoSubs);

    float troca1 =  getTempoDecorrido(eventoTroca1) / 1000000000.0;
    float troca2 =  getTempoDecorrido(eventoTroca2) / 1000000000.0;

    tempoTotalTransfMemoria+=troca1+troca2;

   #endif
}

void iteracao(individuo * populacao, t_regra * gramatica){

    cl_event eventIteracao;


    status = clSetKernelArg(kernelIteracao,
                            0,
                            sizeof(bufferA),
                            &bufferA);
    check_cl(status, "Erro ao adicionar argumento ao kernel");

    status = clSetKernelArg(kernelIteracao,
                            1,
                            sizeof(geracao),
                            &geracao);
    check_cl(status, "Erro ao adicionar argumento ao kernel");

    status = clSetKernelArg(kernelIteracao,
                            2,
                            sizeof(seed),
                            &seed);
    check_cl(status, "Erro ao adicionar argumento ao kernel");


    status =  clSetKernelArg(kernelIteracao,
                            3,
                            sizeof(bufferB),
                            &bufferB);
    check_cl(status, "Erro ao adicionar argumento ao kernel");


    status = clSetKernelArg(kernelIteracao,
                            4,
                            sizeof(bufferCounter),
                            &bufferCounter);
    check_cl(status, "Erro ao adicionar argumento ao kernel");

    if(kernelAG == KERNEL_N_POR_WORK_GROUP){

        status = clSetKernelArg(kernelIteracao,
                                  5,
                                  localWorkSizeIteracao[0]*sizeof(individuo),
                                  NULL);
        check_cl(status, "Erro ao adicionar argumento ao kernel");

        status = clSetKernelArg(kernelIteracao,
                                  6,
                                  (localWorkSizeIteracao[0])*sizeof(int),
                                  NULL);
        check_cl(status, "Erro ao adicionar argumento ao kernel");

        status = clSetKernelArg(kernelIteracao,
                                  7,
                                  (localWorkSizeIteracao[0])*sizeof(int),
                                  NULL);
        check_cl(status, "Erro ao adicionar argumento ao kernel");
    }

    //---------------------------------------------
    // 11: Enfileira o kernel para execução
    //---------------------------------------------

    if(kernelAG == KERNEL_2_POR_WORK_ITEM || kernelAG == KERNEL_1_POR_WORK_ITEM){

        status = clEnqueueNDRangeKernel(cmdQueue,
                                    kernelIteracao,
                                    1,
                                    NULL,
                                    globalWorkSizeIteracao,
                                    NULL,
                                    0,
                                    NULL,
                                    &eventIteracao);

    }
    else{

         status = clEnqueueNDRangeKernel(cmdQueue,
                                    kernelIteracao,
                                    1,
                                    NULL,
                                    globalWorkSizeIteracao,
                                    localWorkSizeIteracao,
                                    0,
                                    NULL,
                                    &eventIteracao);

    }

    check_cl(status, "Erro ao enfileirar o kernel para execucao");

    //Espera o término da fila de execução
    clFinish(cmdQueue);

    #ifdef PROFILING

    float tempoIteracao = getTempoDecorrido(eventIteracao) / 1000000000.0 ;
    tempoTotal += tempoIteracao;
    tempoTotalProcessamento += tempoIteracao;
    tempoTotalAG+=tempoIteracao;

    #endif

    //Avalia a nova geração
    avaliacao(populacao, gramatica, bufferB);

    //Política de substituição dos indivíduos da geração
    substituicao(populacao, gramatica);
}

void eg_paralela(individuo * pop, t_regra *gramatica, Database *dataBase, int pcores, int kernelAG){

    check(pop    != NULL, "A população não pode ser nula");
    check(gramatica != NULL, "A gramática não pode ser nula");
    check(dataBase != NULL, "o banco de dados não pode ser nulo");

    tamanhoBancoDeDados = dataBase->numRegistros;

    opencl_init(pcores, kernelAG, dataBase);

    carrega_gramatica(gramatica);
    carrega_bancoDeDados(dataBase);
    inicializa_populacao(pop);

    avaliacao(pop, gramatica, bufferA);

    while(geracao <= NUMERO_DE_GERACOES){

        iteracao(pop, gramatica);
        geracao++;
    }

    opencl_dispose();

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
}

void opencl_dispose(){

    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseMemObject(bufferGramatica);
    clReleaseMemObject(bufferDatabase);
    clReleaseKernel(kernelAvaliacao);
    clReleaseKernel(kernelInicializacao);
    clReleaseKernel(kernelIteracao);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseContext(context);
}
