#include<stdlib.h>
#include "parametros_ag.h"
#include <stdio.h>

void mutacao(short* individuo, double chance){

    int i;

    for(i=0;i<TAMANHO_INDIVIDUO;i++) {

        //gera um número entre 0 e 1
        float aleatorio = (float)rand()/RAND_MAX;

        if (aleatorio<chance) {
           individuo[i] = (individuo[i]+1)%2;
        }

    }
}

void crossover_um_ponto(short *pai1, short*pai2, short*filho1, short*filho2){

    int i;

    //gera número entre 0 e TAMANHO_INDIVIDUO-1
    int ponto = rand() % (TAMANHO_INDIVIDUO);

    for(i=0;i<=ponto;i++){
       filho1[i] = pai1[i];
       filho2[i] = pai2[i];
    }

     for(i=ponto;i<TAMANHO_INDIVIDUO;i++){
       filho1[i] = pai2[i];
       filho2[i] = pai1[i];
    }
}

void recombinacao(short *pai1, short*pai2, short*filho1, short*filho2, double chance){

    //gera um número entre 0 e 1
    double aleatorio = (float)rand()/RAND_MAX;

    if (aleatorio<chance) {
        crossover_um_ponto(pai1, pai2,filho1, filho2);
    }
    else{

         int j;

         for(j=0;j<TAMANHO_INDIVIDUO;j++){
           filho1[j] = pai1[j];
           filho2[j] = pai2[j];
        }

    }
}
