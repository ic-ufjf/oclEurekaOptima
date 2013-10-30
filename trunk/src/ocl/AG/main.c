#include <stdio.h>
#include "ag.h"
#include "agOpenCL.c"

//OpenCL includes
#include <CL/cl.h>

int main()
{
    //AG();
    individuo populacao[TAMANHO_POPULACAO];

    cria_populacao_inicial(populacao);

    initializeOpenCL();

    cout << "Avaliacao paralela do AG utilizando OpenCL" << endl;

    avaliacao_paralela(populacao);

    //Verificação da avaliação
    int i=0;

    for(;i<TAMANHO_POPULACAO;i++){

        if(populacao[i].aptidao != funcao_de_avaliacao(&populacao[i])){
            cout << "Avaliacao incorreta: " <<populacao[i].aptidao << " em vez de "
            << funcao_de_avaliacao(&populacao[i]) << endl;
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
