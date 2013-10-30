#ifndef REPRESENTACAO_H_INCLUDED
#define REPRESENTACAO_H_INCLUDED

#include "parametros_ag.h"

typedef struct{

    int genotipo[TAMANHO_INDIVIDUO],
        genotipo_binario[TAMANHO_INDIVIDUO],
        fenotipo[DIMENSOES_PROBLEMA],
        aptidao;

} individuo;


#endif // REPRESENTACAO_H_INCLUDED
