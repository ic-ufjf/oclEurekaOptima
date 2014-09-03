#ifndef REPRESENTACAO_H_INCLUDED
#define REPRESENTACAO_H_INCLUDED

#include "parametros_ag.h"

typedef struct Individuo individuo;

struct Individuo{

    short genotipo[TAMANHO_INDIVIDUO];
    short genotipo_binario[TAMANHO_INDIVIDUO];
    int fenotipo[DIMENSOES_PROBLEMA];
    int aptidao;
};


#endif // REPRESENTACAO_H_INCLUDED
