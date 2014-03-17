//System includes
#include <stdio.h>
#include <time.h>

#include "ag.h"
#include "agOpenCL.c"

//OpenCL includes
#include <CL/cl.h>
#define __CL_ENABLE_EXCEPTIONS

//getopt
#include <unistd.h>

#include "parser.h"
#include "gramatica.h"

void InicializaFenotipo( int * fenotipo){

	srand(99413);

	int init;
	for(init=1;init < D_FENOTIPO; init++) fenotipo[init] = rand();
}

int main(int argc, char * argv[])
{
    /* Variáveis */
    individuo populacao[TAMANHO_POPULACAO];

    int pcores = 0,kernelAG = 0;
    char c;

    while ((c = getopt (argc, argv, "p:k:")) != -1){

        switch (c)
        {
            case 'p':
                pcores = atoi(optarg);
                break;
            case 'k':
                kernelAG = atoi(optarg);
                break;

            default:
                break;
        }
    }

    if(pcores>0){
      //cout << "Cores:" << pcores << endl;
    }
    else {
        pcores=0;
    }

    ag_paralelo(populacao, pcores, kernelAG);

    //Verificação da avaliação paralela
    int i=0;

    for(i=0;i<TAMANHO_POPULACAO;i++){

       if(populacao[i].aptidao != funcao_de_avaliacao(&populacao[i])){
            cout << "Avaliacao " << i << " incorreta: " <<populacao[i].aptidao << " em vez de "
            << funcao_de_avaliacao(&populacao[i]) << endl;
            exit(EXIT_FAILURE);
       }
    }

    return 0;
}
