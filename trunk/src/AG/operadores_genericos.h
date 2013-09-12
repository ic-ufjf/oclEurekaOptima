#ifndef OPERADORES_GENETICOS_H_INCLUDED
#define OPERADORES_GENETICOS_H_INCLUDED

void mutacao(char* individuo, double chance);
void crossover_um_ponto(char *pai1, char*pai2, char*filho1, char*filho2);
void recombinacao(char *pai1, char *pai2, char *filho1, char *filho2, float chance);

#endif // OPERADORES_GENETICOS_H_INCLUDED
