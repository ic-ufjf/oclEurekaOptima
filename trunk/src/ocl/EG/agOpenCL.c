#include "agOpenCL.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include "gramatica.h"

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

#define TAMANHO_DATABASE 5

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

cl_uint max_compute_units;

float tempoTotal = 0;

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

size_t globalWorkSizeIteracao[1],localWorkSizeIteracao[1];
size_t datasize;

cl_mem bufferA, bufferB, bufferC, bufferCounter, bufferPais_local,
bufferFilhos_local, bufferProgramas, bufferGramatica, bufferDatabase;

//Eventos utilizados para medir o tempo de execução do kernel
//e das trocas de memória
cl_event event1, event2, event3;

size_t preferred_workgroup_size_multiple;


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

    cout << "max comput units:" << ncores << endl;

    const cl_device_partition_property subdevice_properties[] =
    { CL_DEVICE_PARTITION_BY_COUNTS,
        ncores, 0, CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0 };

    cl_device_id device_ids[2];

    numSubDevices=2;

    status = clCreateSubDevices(devices[0], subdevice_properties, numSubDevices, devices, NULL);

    device = devices[0];

    if(status != CL_SUCCESS){
        cout << "Erro ao criar os subdevices. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

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


void initializeOpenCL(int cores, int kernel){

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

    if(cores!=0){

        CriaSubDevices();

    }

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
                                "#define TAMANHO_GRAMATICA " + ToString(5) + "\n" +
                                "#define TAMANHO_DATABASE " + ToString(TAMANHO_DATABASE) + "\n" +
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
                            1,
				devices,
				"-I ./include -I /usr/include -I include/Random123 -I include/vsmc",
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

	bufferA = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
	bufferC = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);

    //bufferProgramas = clCreateBuffer(context, CL_MEM_READ_WRITE, TAMANHO_POPULACAO*sizeof(t_prog), NULL, &status);
    bufferGramatica = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(t_regra)*5, NULL, &status);
    bufferDatabase  = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*5*TAMANHO_DATABASE, NULL, &status);


    if(status != CL_SUCCESS){
        cout << "Erro ao enviar os aleatorios. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    bufferB = clCreateBuffer(context, CL_MEM_READ_WRITE , datasize, NULL, &status);

    if(kernelAG == KERNEL_2_POR_WORK_GROUP2)
        bufferCounter = clCreateBuffer(context, CL_MEM_READ_WRITE , TAMANHO_POPULACAO * sizeof(r123array4x32) * 4, NULL, &status);

    else
        bufferCounter = clCreateBuffer(context, CL_MEM_READ_WRITE , TAMANHO_POPULACAO * sizeof(r123array4x32), NULL, &status);


    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

	//---------------------------------------------
	// 7: Criação do kernel
	//---------------------------------------------

    kernelInicializacao = clCreateKernel(program, "inicializa_populacao", &status);

    if(CPU){

        status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS,
        sizeof(cl_uint), &max_compute_units, NULL);

        preferred_workgroup_size_multiple = (size_t)max_compute_units;

        //cout << "Preferred work group size:" << preferred_workgroup_size_multiple << endl;
    }
    else{

        //Obtém o tamanho do qual o groupsize deve ser múltiplo
        status = clGetKernelWorkGroupInfo(kernelInicializacao, device,
                                          CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                          sizeof(size_t),
                                          &preferred_workgroup_size_multiple,
                                          NULL);

        //cout << "Preferred work group size:" << preferred_workgroup_size_multiple << endl;

        preferred_workgroup_size_multiple = 4;
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
        cout << "Kernel invalido" << endl;
        exit(EXIT_FAILURE);
    }

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o kernel. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    //Cria o kernel de avaliação

    kernelAvaliacao = clCreateKernel(program, "avaliacao", &status);

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o kernel de avaliacao. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    if(CPU){
        kernelSubstituicao = clCreateKernel(program, "substituicao", &status);
    }
    else{
        kernelSubstituicao = clCreateKernel(program, "substituicao_gpu", &status);
    }

    if(status != CL_SUCCESS){
        cout << "Erro ao criar o kernel 'substituicao'. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

}

void exibe_melhor(individuo * melhor, t_regra * gramatica){

    /*if(melhor->aptidao==0){
        printf("Geracoes para encontrar o melhor: \t %d\n", geracao);

        int i;

        short genotipo_binario[TAMANHO_INDIVIDUO];

        exit(EXIT_SUCCESS);
    }
    */

    printf("---------------------------------");
    printf("\nGeracao %d: \n", geracao);
    printf("\nMelhor da geracao: %d: %f\n", geracao, melhor->aptidao);

    t_item_programa programa[TAMANHO_MAX_PROGRAMA];
    short fenotipo[DIMENSOES_PROBLEMA];

    obtem_fenotipo_individuo2(melhor, fenotipo);

    Decodifica(gramatica, fenotipo, programa);
    ImprimeIndividuo(programa);
}

void avaliacao(individuo *pop, t_regra * gramatica){

    cl_event eventoAvaliacao;

    status = clSetKernelArg(kernelAvaliacao,  0, sizeof(bufferB), &bufferB);
    status = clSetKernelArg(kernelAvaliacao,  1, sizeof(bufferGramatica), &bufferGramatica);
    //status = clSetKernelArg(kernelAvaliacao,  2, sizeof(bufferProgramas), &bufferProgramas);
    status = clSetKernelArg(kernelAvaliacao,  2, sizeof(bufferDatabase),  &bufferDatabase);

    size_t localWorkSize[1] = {1};
    size_t globalWorkSize[1];

    globalWorkSize[0] = ceil((float)TAMANHO_POPULACAO/localWorkSize[0])*localWorkSize[0];

    clEnqueueNDRangeKernel(cmdQueue,
                           kernelAvaliacao,
                           1,
                           NULL,
                           globalWorkSize,
                           localWorkSize,
                           0,
                           NULL,
                           &eventoAvaliacao);

     clEnqueueReadBuffer(cmdQueue, bufferB, CL_TRUE, 0,
						 datasize, pop, 0, NULL, &event3);

     clFinish(cmdQueue);

     #ifdef PROFILING

        float tempoAvaliacao = getTempoDecorrido(eventoAvaliacao) / 1000000000.0 ;
        tempoTotal += tempoAvaliacao;
        printf("Tempo de avaliacao: \t %.10f\n", tempoAvaliacao);

     #endif

     return;

     int i,j;

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

        ImprimeIndividuo(programa);

        printf("\n");
     }


}

void substituicao(individuo *pop, t_regra * gramatica){

    cl_event eventoSubstituicao;

    status = clSetKernelArg(kernelSubstituicao,  0, sizeof(bufferA), &bufferA);
    status = clSetKernelArg(kernelSubstituicao,  1, sizeof(bufferB), &bufferB);
    status = clSetKernelArg(kernelSubstituicao,  2, sizeof(bufferC), &bufferC);

    if(CPU){

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

    }
    else{

        size_t localWorkSize[1] = {preferred_workgroup_size_multiple};
        size_t globalWorkSize[1];

        globalWorkSize[0] = ceil((float)TAMANHO_POPULACAO/localWorkSize[0])*localWorkSize[0];

        cout << "Local: " << localWorkSize[0] << ", global:" << globalWorkSize[0] << endl;

        status = clSetKernelArg(kernelSubstituicao,  3, sizeof(int)*4, NULL);
        status = clSetKernelArg(kernelSubstituicao,  4, sizeof(int)*4, NULL);

        clEnqueueNDRangeKernel(cmdQueue,
                               kernelSubstituicao,
                               1,
                               NULL,
                               globalWorkSize,
                               localWorkSize,
                               0,
                               NULL,
                               &eventoSubstituicao);
    }

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

   clEnqueueReadBuffer(cmdQueue, bufferA, CL_TRUE, 0,
						sizeof(individuo), melhor1,
						0, NULL, &event3);

   clEnqueueReadBuffer(cmdQueue, bufferA, CL_TRUE, offset,
						sizeof(individuo), melhor2,
						0, NULL, &event3);

   clFinish(cmdQueue);


   if(melhor1[0].aptidao > melhor2[0].aptidao){
       exibe_melhor(melhor1, gramatica);
   }
   else{
       exibe_melhor(melhor2, gramatica);
   }

   #ifdef PROFILING

    float tempoSubs = getTempoDecorrido(eventoSubstituicao) / 1000000000.0 ;
    tempoTotal += tempoSubs;
    printf("Tempo de substituicao: \t %.10f\n", tempoSubs);

   #endif
}

/*
    Executa o kernel de iteracao
*/

void iteracao(individuo * populacao, t_regra * gramatica){

    cl_event eventIteracao;

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

    status = clSetKernelArg(kernelIteracao,
                            4,
                            sizeof(bufferCounter),
                            &bufferCounter);


    if(kernelAG == KERNEL_N_POR_WORK_GROUP){

        status = clSetKernelArg(kernelIteracao,
                                  5,
                                  localWorkSizeIteracao[0]*sizeof(individuo),
                                  NULL);

        status = clSetKernelArg(kernelIteracao,
                                  6,
                                  (localWorkSizeIteracao[0])*sizeof(int),
                                  NULL);

        status = clSetKernelArg(kernelIteracao,
                                  7,
                                  (localWorkSizeIteracao[0])*sizeof(int),
                                  NULL);
    }

    if(status != CL_SUCCESS){
        cout << "Erro ao criar buffer de memoria. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
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

    if(status != CL_SUCCESS){
        cout << "Erro ao enfileirar o kernel para execucao. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }

    //Espera o término da fila de execução
    clFinish(cmdQueue);

    #ifdef PROFILING

    float tempoIteracao = getTempoDecorrido(eventIteracao) / 1000000000.0 ;
    tempoTotal += tempoIteracao;

    #endif

    avaliacao(populacao, gramatica);

    substituicao(populacao, gramatica);
}

void carrega_gramatica(t_regra * gramatica){

      cl_event eventoInicializacao;

      //Transfere os e da população para o bufferA
      status = clEnqueueWriteBuffer(cmdQueue,
                                      bufferGramatica,
                                      CL_TRUE,
                                      0,
                                      sizeof(t_regra)*5,
                                      gramatica,
                                      0,
                                      NULL,
                                      &eventoInicializacao);

   //Espera o término da fila de execução
   clFinish(cmdQueue);

   if(status != CL_SUCCESS){
        cout << "Erro carregar a gramatica. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
   }

}


void carrega_bancoDeDados(float dataBase[][5]){

    cl_event eventoInicializacao;

    //Transfere os e da população para o bufferA
    status = clEnqueueWriteBuffer(cmdQueue,
                                      bufferDatabase,
                                      CL_TRUE,
                                      0,
                                      sizeof(float)*TAMANHO_DATABASE*5,
                                      dataBase,
                                      0,
                                      NULL,
                                      &eventoInicializacao);

   //Espera o término da fila de execução
   clFinish(cmdQueue);

   if(status != CL_SUCCESS){
        cout << "Erro carregar a gramatica. (Erro " << status << ")" << endl;
        exit(EXIT_FAILURE);
    }


}

void inicializa_populacao(individuo * pop){

    int seed = rand();

    cl_event eventoInicializacao;

    status = clSetKernelArg(kernelInicializacao,
							0,
							sizeof(bufferA),
							&bufferA);

    status = clSetKernelArg(kernelInicializacao,
							1,
							sizeof(seed),
							&seed);

    //Transfere os e da população para o bufferA
    status = clEnqueueWriteBuffer(cmdQueue,
								  bufferA,
								  CL_TRUE,
								  0,
								  datasize,
								  pop,
								  0,
								  NULL,
								  &event1);


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


    clEnqueueReadBuffer(cmdQueue,
						bufferA,
						CL_TRUE,
						0,
						datasize,
						pop,
						0,
						NULL,
						&event3);


    //Espera o término da fila de execução
	clFinish(cmdQueue);

	#ifdef PROFILING

      float tempoInicializacao = getTempoDecorrido(eventoInicializacao) / 1000000000.0 ;
      tempoTotal += tempoInicializacao;

    #endif

}

void exibePopulacao(individuo * populacao){

    int i,j;

    printf("\n");

    for(i=0; i < TAMANHO_POPULACAO; i++){

        for(j=0; j< TAMANHO_INDIVIDUO; j++){
               // printf(" %d ", populacao[i].genotipo[j]);
        }
        printf("%d ---> \t %d \n",i, populacao[i].aptidao);
    }
}



void ag_paralelo(individuo * pop, t_regra *gramatica, float dataBase[][5], int pcores, int kernelAG){

    initializeOpenCL(pcores, kernelAG);

    inicializa_populacao(pop);
    carrega_gramatica(gramatica);
    carrega_bancoDeDados(dataBase);

    while(geracao < NUMERO_DE_GERACOES){

        iteracao(pop, gramatica);

        //exibePopulacao(pop);

        int j;
        for(j=0;j<0-TAMANHO_POPULACAO;j++){

           if(pop[j].aptidao != funcao_de_avaliacao(&pop[j])){
                cout << "Avaliacao " << j << " incorreta: " <<pop[j].aptidao << " em vez de "
                << funcao_de_avaliacao(&pop[j]) << endl;
                exit(EXIT_FAILURE);
           }
        }

        geracao++;
    }

   #ifdef PROFILING

      printf("Tempo total: \t %.10f\n", tempoTotal);

   #endif
}
