//System includes
#include <stdio.h>
#include <time.h>

#include "ag.h"
#include "agOpenCL.c"

//OpenCL includes
#include <CL/cl.h>

//getopt
#include <getopt.h>

#include "parser.h"
#include "gramatica.h"

void print_usage(){
    puts("-----------------------------------------------------------");
    printf("Usage: --database='file' [--cores=num] [--kernelAG=num]\n");
    puts("-----------------------------------------------------------");
}

int main(int argc, char * argv[])
{
    /* Variáveis */
	t_regra Gramatica[10];
    individuo * populacao = (individuo*) malloc(sizeof(individuo)*TAMANHO_POPULACAO);


    char arquivoBancoDeDados[50] = "";

    int pcores = 0, kernelAG = 0;
    char c;
    while (1)
    {
        static struct option long_options[] =
        {
           {"database",  required_argument, 0, 'd'},
           {"cores",     required_argument, 0, 'c'},
           {"kernelAG",  required_argument, 0, 'k'},
           {0,  0,  0,  0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "dc", long_options, &option_index);

        if (c == -1){
            break;
        }

        switch (c)
        {
            case 'd':
                printf("Banco de dados: %s\n", optarg);
                strcpy(arquivoBancoDeDados, optarg);
                break;

            case 'c':
                pcores = atoi(optarg);
                printf("Cores: %d\n", pcores);
                break;

            case 'k':
                kernelAG = atoi(optarg);
                printf("KernelAG: %d\n", kernelAG);
                break;

            case '?':
                print_usage();
                exit(0);
                break;

            default:
               print_usage();
               exit(0);
               break;
        }
    }

    if(strlen(arquivoBancoDeDados) == 0){
        print_usage();
        printf("Banco de dados não especificado. Utilizando o arquivo 'problems/2X^2.txt'\n");
        strcpy(arquivoBancoDeDados, "problems/2X^2.txt");
    }

    Database * dataBase = database_read(arquivoBancoDeDados);

    puts("-----------------------------------------------------------");
    printf("Tamanho do banco de dados:%d \t Número de variáveis:%d\n", dataBase->numRegistros, dataBase->numVariaveis);
    puts("-----------------------------------------------------------");

	LeGramatica("grammars/g1.txt", Gramatica);

	ag_paralelo(populacao, Gramatica, dataBase, pcores, kernelAG);

    free(dataBase->registros);
    free(dataBase);
    free(populacao);

    return 0;
}
