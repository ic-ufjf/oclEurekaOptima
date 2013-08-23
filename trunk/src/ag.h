#ifndef AG_H_INCLUDED
#define AG_H_INCLUDED

#include <stdlib.h>

int obtem_valor_numerico(char *individuo);

int funcao_de_avaliacao(char *individuo);

void cria_populacao_inicial();

void avalia_populacao();

int obtem_soma_avaliacoes();

int roleta();

int obtem_mais_apto();

void AG();

#endif // AG_H_INCLUDED
