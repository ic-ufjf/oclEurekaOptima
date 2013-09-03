#include<stdio.h>
#include "representacao.h"
#include "parametros_ag.h"
#include "operadores_geneticos.h"
#include "ag.h"
#include <math.h>
#include <stdlib.h>

individuo populacao[TAMANHO_POPULACAO];
individuo copia_populacao[TAMANHO_POPULACAO];

int n = 1;

long binario_para_decimal(individuo *p, int inicio, int fim){

    int i,n=1; long valorNumerico=0;

    for(i=fim-1; i>=inicio; i--, n=n<<1){

        valorNumerico += n*((int)p->genotipo_binario[i]);

    }

    return valorNumerico;
}

void binario_para_inteiro(short *binarios, long *inteiros){

    int start,n,i, j;

    int end = 0;
    for (j = 0; j < TAMANHO_INDIVIDUO; j++) {
        inteiros[j] = 0;
        start = end;
        end += TAMANHO_VALOR;
        n=1;
        for (i = start; i < end; i++, n=n<<1) {
            inteiros[j] += binarios[i] * n;
        }
    }
}


void gray_para_binario(short *gray, short *binarios){

        int i,j;

        for(i=0; i< TAMANHO_INDIVIDUO; i++){
            binarios[i] = gray[i];
        }

        int start;
        int end = 0;
        for (j = 0; j < DIMENSOES_PROBLEMA; j++) {
            start = end;
            end += TAMANHO_VALOR;
            for (i = start + 1; i < end; i++) {
                binarios[i] = binarios[i - 1] ^ binarios[i];
            }
        }
}

void obtem_fenotipo_individuo(individuo *p){

    int i, j=0;

    gray_para_binario(p->genotipo, p->genotipo_binario);
    //binario_para_inteiro(p->genotipo_binario, p->fenotipo);

    for(i=0; i<DIMENSOES_PROBLEMA; i++, j+=TAMANHO_VALOR){

       p->fenotipo[i] = binario_para_decimal(p, j, j+TAMANHO_VALOR);

    }
}

int funcao_de_avaliacao(individuo *p){

    obtem_fenotipo_individuo(p);

    int soma = 0;

    int i;

    for(i=0;i < DIMENSOES_PROBLEMA; i++){
        soma += pow(p->fenotipo[i],2);
    }

    return soma * (-1);
}

void cria_populacao_inicial(){

    int i,j;

    for(i=0; i < TAMANHO_POPULACAO; i++){

         for(j=0; j< TAMANHO_INDIVIDUO; j++){

            populacao[i].genotipo[j] = rand() % 2;
        }
    }
}

void avalia_populacao(){

    int i;

    for(i=0; i < TAMANHO_POPULACAO; i++){

        //Avaliação do indivíduo
        populacao[i].aptidao = funcao_de_avaliacao(&populacao[i]);

    }
}

int soma_avaliacoes(){

    int aux = 0;
    int i;
    for(i=0;i<TAMANHO_POPULACAO;i++){
        aux += populacao[i].aptidao;
    }

    return aux;
}

/*

int roleta() {
	int i;
	int somaAvaliacoes = soma_avaliacoes();

    //gera um número entre 0 e a soma das avaliações
	int limite = rand() % somaAvaliacoes;

    for(i=0; i<TAMANHO_POPULACAO; i++) {

        if(aptidao[i][1]>limite){
            return i;
        }
	}

	return (0);
}


*/


void torneio(int indice_participante, individuo *populacao, individuo *retorno) {

    individuo vencedor = populacao[indice_participante];
	int i, aleatorio = 0;

    //printf("----------------------Torneio %d--------------------------\n\n", indice_participante);

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
void adiciona_individuo(individuo *individuo, int indice){

    int j;
    for(j=0;j<TAMANHO_INDIVIDUO;j++){
        copia_populacao[indice].genotipo[j] = individuo->genotipo[j];
    }

    copia_populacao[indice].aptidao = individuo->aptidao;
}

/*
    Cria uma nova geração, através dos passos:
    1)Seleção;
    2)Recombinação;
    3)Mutação;

    até se obter uma população de tamanho TAMANHO_POPULACAO
*/

individuo pai1,pai2,filho1,filho2;


int compara_individuo(const void* a, const void* b){

    individuo* p1 = (individuo*)a;
    individuo* p2 = (individuo*)b;

    return p1->aptidao < p2->aptidao;
}


void cria_nova_geracao(){

     int i;

     for(i=0;i<TAMANHO_POPULACAO-1;i++) {

        //Seleção
		torneio(i,   populacao, &pai1);
        torneio(i+1, populacao, &pai2);

        //Recombinação
        recombinacao(&pai1, &pai2, &filho1, &filho2, TAXA_DE_RECOMBINACAO);

        //Mutação
		mutacao(&filho1, TAXA_DE_MUTACAO);
		mutacao(&filho2, TAXA_DE_MUTACAO);


        filho1.aptidao = funcao_de_avaliacao(&filho1);
        filho2.aptidao = funcao_de_avaliacao(&filho2);

        adiciona_individuo(&filho1,i);
        adiciona_individuo(&filho2,++i);
	 }

	 //Ordena a geração atual
     qsort(populacao, TAMANHO_POPULACAO, sizeof(individuo), (int(*)(const void*, const void*))compara_individuo);

     //Ordena a nova geração
     qsort(copia_populacao, TAMANHO_POPULACAO, sizeof(individuo), (int(*)(const void*, const void*))compara_individuo);

     //Mantém a elite e substitui o restante pelos melhores da nova geração
     int j = 0, l;
     for(i = ELITE; i < TAMANHO_POPULACAO;i++,j++){

        for(l=0;l<TAMANHO_INDIVIDUO;l++){
            populacao[i].genotipo[l] = copia_populacao[j].genotipo[l];
        }

     }
}


long obtem_mais_apto(){

    long mais_apto = populacao[0].aptidao;
    int i;

    for(i=1;i<TAMANHO_POPULACAO;i++){

        if(populacao[i].aptidao > mais_apto){
            mais_apto = populacao[i].aptidao;
        }
    }

    return mais_apto;
}

void exibe_dados_geracao(){

    printf("---------------------------------");
    printf("\nGeracao %d: \n", n);

    long mais_apto = obtem_mais_apto();


    printf("\nMelhor da geracao: %d: %ld\n", n, mais_apto);
    printf("---------------------------------");
}


void AG(){

    srand(3);
    cria_populacao_inicial();

    for(;;){

        avalia_populacao();
        exibe_dados_geracao();

        if(CRITERIO_DE_PARADA){
            return;
        }
        else{
            cria_nova_geracao();
        }

        n++;
    }
}
