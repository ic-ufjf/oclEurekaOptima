#ifndef REPRESENTACAO_H_INCLUDED
#define REPRESENTACAO_H_INCLUDED

#include "parametros_ag.h"

typedef struct Individuo individuo;

struct Individuo{

    short genotipo[TAMANHO_INDIVIDUO];
    short genotipo_binario[TAMANHO_INDIVIDUO];
    long fenotipo[DIMENSOES_PROBLEMA];
    long aptidao;
};


#endif // REPRESENTACAO_H_INCLUDED
