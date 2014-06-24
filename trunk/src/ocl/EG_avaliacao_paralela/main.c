//System includes
#include <stdio.h>
#include <time.h>

#include "ag.h"
#include "eg.h"

//getopt
#include <getopt.h>

#include "parser.h"
#include "gramatica.h"
#include "utils.h"

void print_usage(){
    puts("-------------------------------------------------------------------------------");
    printf("Usage: --database='file' [--grammar='file'] [--cores=num] [--kernelAG=num]\n");
    puts("-------------------------------------------------------------------------------");
}

int main(int argc, char * argv[])
{
    #ifdef PROFILING
        desabilita_cache_compilacao();    
    #endif

    /* Variáveis */
    t_regra Gramatica[10];
    individuo * populacao = (individuo*) malloc(sizeof(individuo)*TAMANHO_POPULACAO);

    char arquivoBancoDeDados[50] = "", arquivoGramatica[50] = "grammars/g1.txt";

	srand(time(NULL));

    int pcores = 0, kernelAG = 2;
    char c;
    while (1)
    {
        static struct option long_options[] =
        {
           {"database",  required_argument, 0, 'd'},
           {"grammar",  required_argument, 0, 'g'},
           {"cores",     required_argument, 0, 'c'},
           {"kernelAG",  required_argument, 0, 'k'},
           {0,  0,  0,  0}
        };

        int option_index = 0;

        c = getopt_long(argc, argv, "dc", long_options, &option_index);

        if (c == -1){
            break;
        }

        switch (c)
        {
            case 'd':                
                strcpy(arquivoBancoDeDados, optarg);
                break;
                
           case 'g':                
                strcpy(arquivoGramatica, optarg);
                break;

            case 'c':
                pcores = atoi(optarg);
                break;

            case 'k':
                kernelAG = atoi(optarg);
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
        printf("Banco de dados não especificado. Utilizando o arquivo 'problems/2X^2.txt'\n");
        strcpy(arquivoBancoDeDados, "problems/2X^2.txt");
    }

    Database * dataBase = database_read(arquivoBancoDeDados);

    /*puts("-----------------------------------------------------------");
    printf("Tamanho do banco de dados:%d \t Número de variáveis:%d\n", dataBase->numRegistros, dataBase->numVariaveis);
    puts("-----------------------------------------------------------");*/
    
    LeGramatica(arquivoGramatica, Gramatica);

    double inicio = getTime();

    eg(populacao, Gramatica, dataBase);

    double fim = getTime()-inicio;
    
    printf("%lf\n", fim);

    free(dataBase->registros);
    free(dataBase);
    free(populacao);
    
    return 0;
}
