#ifndef AG_H_INCLUDED
#define AG_H_INCLUDED

#include "representacao.h"

int funcao_de_avaliacao(individuo *i);

void cria_populacao_inicial(individuo * pop);

void avalia_populacao(individuo * pop);

int obtem_soma_avaliacoes(individuo * pop);

void torneio(int indice_participante, individuo *populacao, individuo *retorno);

int obtem_mais_apto(individuo * pop);

int compara_individuo(const void *a, const void *b);

void adiciona_individuo(individuo *p, individuo * pop, int indice);

void sort(individuo * pop);

void cria_nova_populacao(individuo * pop, individuo * newPop);

void substitui_populacao(individuo *pop, individuo * newPop);

#endif // AG_H_INCLUDED
