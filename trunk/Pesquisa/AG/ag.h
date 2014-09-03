#ifndef AG_H_INCLUDED
#define AG_H_INCLUDED

#include "representacao.h"
#include <stdlib.h>

void obtem_fenotipo_individuo(individuo *i);

int funcao_de_avaliacao(individuo *i);

void cria_populacao_inicial();

void avalia_populacao();

int obtem_soma_avaliacoes();

int roleta();

void torneio(int indice_participante, individuo *populacao, individuo *retorno);

int obtem_mais_apto();

void AG();

int compara_individuo(const void *a, const void *b);

#endif // AG_H_INCLUDED
