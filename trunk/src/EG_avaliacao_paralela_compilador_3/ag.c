#include <stdio.h>
#include "representacao.h"
#include "parametros_ag.h"
#include "operadores_geneticos.h"
#include "ag.h"
#include <math.h>
#include <stdlib.h>

void cria_populacao_inicial(individuo * pop){

    int i,j;

    for(i=0; i < TAMANHO_POPULACAO; i++){

         for(j=0; j< TAMANHO_INDIVIDUO; j++){
            pop[i].genotipo[j] = rand() % 2;
        }
   }
}

void torneio(int indice_participante, individuo *populacao, individuo *retorno) {

    individuo vencedor = populacao[indice_participante];
	int i, aleatorio = 0;

    for(i=0; i < TAMANHO_TORNEIO; i++) {

        aleatorio = rand() % TAMANHO_POPULACAO;

        if(populacao[aleatorio].aptidao > vencedor.aptidao){
            vencedor = populacao[aleatorio];
        }
    }

    for(i=0;i< TAMANHO_INDIVIDUO;i++){
        retorno->genotipo[i] = vencedor.genotipo[i];
    }

}

/*
    Adiciona o i-ésimo indivíduo na população
*/
void adiciona_individuo(individuo *p, individuo * pop, int indice){

    int j;
    for(j=0;j<TAMANHO_INDIVIDUO;j++){
        pop[indice].genotipo[j] = p->genotipo[j];
    }

    pop[indice].aptidao = p->aptidao;
}

/*
 Função utilizada pelo qsort para ordenar a população segundo a aptidão.
*/
int compara_individuo(const void* a, const void* b){

    individuo* p1 = (individuo*)a;
    individuo* p2 = (individuo*)b;

    return p1->aptidao < p2->aptidao;
}

void sort(individuo * pop){	
	qsort(pop, TAMANHO_POPULACAO, sizeof(individuo), (int(*)(const void*, const void*))compara_individuo);
}

void cria_nova_populacao(individuo * pop, individuo * newPop){

	int i;

	for(i=0;i<TAMANHO_POPULACAO-1;i++) {

		individuo parents[2], offspring[2];

        //Seleção
		torneio(i,   pop, &parents[0]);
		torneio(i+1, pop, &parents[1]);

	 	//Recombinação
    	recombinacao(&parents[0],  &parents[1], &offspring[0], &offspring[1], TAXA_DE_RECOMBINACAO);

	 	//Mutação
		mutacao(&offspring[0], TAXA_DE_MUTACAO);
		mutacao(&offspring[1], TAXA_DE_MUTACAO);

	  	adiciona_individuo(&offspring[0],newPop, i);
    	adiciona_individuo(&offspring[1],newPop, i+1);

		i++;
	 }
}

void substitui_populacao(individuo *pop, individuo * newPop){
	
	sort(pop);
	sort(newPop);

	/* Mantém elite */
	int j = 0, l, i;
	for(i = ELITE; i < TAMANHO_POPULACAO;i++,j++){

		for(l=0;l<TAMANHO_INDIVIDUO;l++){
			pop[i].genotipo[l] = newPop[j].genotipo[l];
		}
		pop[i].aptidao = newPop[j].aptidao;
	 }
}



