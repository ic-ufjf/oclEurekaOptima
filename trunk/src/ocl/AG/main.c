#define __CL_ENABLE_EXCEPTIONS

//System includes
#include <stdio.h>
#include <time.h>

#include "ag.h"
#include "agOpenCL.c"

//OpenCL includes
#include <CL/cl.h>

int main()
{
    cout << "Avaliacao paralela do AG utilizando OpenCL" << endl;

    //AG();

    individuo populacao[TAMANHO_POPULACAO];

    //avalia_populacao(populacao);

    //cria_populacao_inicial(populacao);

    //Avaliação sequencial
    /*clock_t start = clock();

    avalia_populacao(populacao);

    clock_t finish = clock();

    float t_sequencial = 1000.0f*(float(finish - start)/CLOCKS_PER_SEC);

    printf("Tempo sequencial: %0.10f ms\n", t_sequencial);
*/
    //Avaliação paralela
    //exibePopulacao(populacao);

    ag_paralelo(populacao);

    //exibePopulacao(populacao);

    //Verificação da avaliação paralela
    int i=0;

    for(i=0;i<TAMANHO_POPULACAO;i++){

       if(populacao[i].aptidao != funcao_de_avaliacao(&populacao[i])){
            cout << "Avaliacao " << i << " incorreta: " <<populacao[i].aptidao << " em vez de "
            << funcao_de_avaliacao(&populacao[i]) << endl;
            exit(EXIT_FAILURE);
       }
    }

    //cout << soma/TAMANHO_POPULACAO << endl;

    return 0;
}


