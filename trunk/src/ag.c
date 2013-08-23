#include<stdio.h>
#include "parametros_ag.h"
#include "operadores_geneticos.h"
#include "ag.h"

int n = 1;

char populacao[TAMANHO_POPULACAO][TAMANHO_INDIVIDUO+1];
char copia_populacao[TAMANHO_POPULACAO][TAMANHO_INDIVIDUO+1];

/*
 aptidao[i][0] = aptidao absoluta
 aptidao[i][1] = aptidao acumulada
*/
int aptidao[TAMANHO_POPULACAO][2];

int obtem_valor_numerico(char *individuo){

    int i,n=1, valorNumerico=0;

    for(i=TAMANHO_INDIVIDUO-1; i>=0; i--){

        if(individuo[i]=='1'){
            valorNumerico += n;
        }

        n = n*2;
    }

    return valorNumerico;
}

//f(x) = (x²)(-1)
int funcao_de_avaliacao(char *individuo){

    int valorNumerico = obtem_valor_numerico(individuo);

    return  (valorNumerico*valorNumerico)*(-1);
}

void cria_populacao_inicial(){

    int i,j;

    for(i=0; i< TAMANHO_POPULACAO; i++){

         for(j=0; j< TAMANHO_INDIVIDUO; j++){

            populacao[i][j] = rand() % 2 ? '1' : '0';
            //printf("%c", populacao[i][j] );
        }
        populacao[i][j] =  '\0';
        //printf("\n");
    }
}

void avalia_populacao(){

    int i;

    aptidao[0][0] = aptidao[0][1] = funcao_de_avaliacao(populacao[0]);

    for(i=1; i< TAMANHO_POPULACAO; i++){

        //aptidão absoluta
        aptidao[i][0] = funcao_de_avaliacao(populacao[i]);

        //aptidão acumulada
        aptidao[i][1] = aptidao[i-1][1] + aptidao[i][0];
    }
}

int soma_avaliacoes(){

    int aux = 0;
    int i;
    for(i=0;i<TAMANHO_POPULACAO;i++){
        aux += aptidao[i][0];
        //printf("%d \n", aux);
    }

    return aux;
}


int roleta() {
	int i;
	int somaAvaliacoes = soma_avaliacoes();

    //gera um número entre 0 e a soma das avaliações
	int limite = rand() % somaAvaliacoes;

    for(i=0; i<TAMANHO_POPULACAO; i++) {

        if(aptidao[i][1]>limite){
            //printf("limite=%d, selecionado=%d \n",limite, i);
            return i;
        }
	}

	return(0);
}

/*
    Adiciona o i-ésimo indivíduo na população
*/
void adiciona_individuo(char *populacao, char *individuo){

    int j;
    for(j=0;j<TAMANHO_INDIVIDUO;j++){
        populacao[j] = individuo[j];
    }
}

/*
    Cria uma nova geração, através dos passos:
    1)Seleção;
    2)Recombinação;
    3)Mutação;

    até se obter uma população de tamanho TAMANHO_POPULACAO
*/

void cria_nova_geracao(){

     int i,j;

     //Copia a geração atual para a matriz copia_populacao
     for(i=0;i<TAMANHO_POPULACAO;i++)
        for(j=0;j<=TAMANHO_INDIVIDUO;j++)
            copia_populacao[i][j] = populacao[i][j];

    char filho1[TAMANHO_INDIVIDUO];
    char filho2[TAMANHO_INDIVIDUO];


     for(i=0;i<TAMANHO_POPULACAO;i++) {

        //Seleção
		char *pai1 = copia_populacao[roleta()];
        char *pai2 = copia_populacao[roleta()];

        //Recombinação
        recombinacao(pai1, pai2, filho1, filho2, TAXA_DE_RECOMBINACAO);

        //Mutação
		mutacao(filho1, TAXA_DE_MUTACAO);
		mutacao(filho2, TAXA_DE_MUTACAO);

        //Adiciona os novos indivíduos na população
        adiciona_individuo(populacao[i], filho1);
        adiciona_individuo(populacao[++i], filho2);
	}

}

int obtem_mais_apto(){

    int mais_apto = 0;
    int i;

    for(i=0;i<TAMANHO_POPULACAO;i++){

        if(aptidao[i][0]>mais_apto){
            mais_apto = aptidao[i][0];
        }
    }

    return mais_apto;
}


void AG(){

    srand(3);

    cria_populacao_inicial();
            avalia_populacao();

    for(;;){

        n++;

        //avalia_populacao();

        if(CRITERIO_DE_PARADA){
            return;
        }
        else{

            cria_nova_geracao();

        }
    }
}
