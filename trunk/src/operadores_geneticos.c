#include<stdlib.h>
#include "parametros_ag.h"
#include <stdio.h>

void mutacao(char* individuo, double chance){

    int i;

    for(i=0;i<TAMANHO_INDIVIDUO;i++) {

        //gera um número entre 0 e 1
        float aleatorio = (float)rand()/RAND_MAX;

        if (aleatorio<chance) {
           individuo[i] = individuo[i] == '1' ? '0' : '1';
        }
    }
}

void crossover_um_ponto(char *pai1, char*pai2, char*filho1, char*filho2){

    //gera número entre 0 e TAMANHO_INDIVIDUO-1
    int ponto = rand() % (TAMANHO_INDIVIDUO);

    int i;

    for(i=0;i<=ponto;i++){
       filho1[i] = pai1[i];
       filho2[i] = pai2[i];
    }

     for(i=ponto;i<=TAMANHO_INDIVIDUO;i++){
       filho1[i] = pai2[i];
       filho2[i] = pai1[i];
    }
}

void recombinacao(char *pai1, char*pai2, char*filho1, char*filho2, double chance){

    //gera um número entre 0 e 1
    double aleatorio = (float)rand()/RAND_MAX;


    if (aleatorio<chance) {
        crossover_um_ponto(pai1, pai2,filho1, filho2);
    }
    else{
        filho1 = pai1;
        filho2 = pai2;
    }
}
