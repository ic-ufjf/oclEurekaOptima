#ifndef OPERADORES_GENETICOS_H_INCLUDED
#define OPERADORES_GENETICOS_H_INCLUDED

void mutacao(short* individuo, double chance);
void crossover_um_ponto(short *pai1, short*pai2, short*filho1, short*filho2);
void recombinacao(short *pai1, short *pai2, short *filho1, short *filho2, float chance);

#endif // OPERADORES_GENETICOS_H_INCLUDED
