#include "representacao.h"

int binario_para_decimal(int binario[], int inicio, int fim){

    int i,n=1; int valorNumerico=0;

    for(i=fim-1; i>=inicio; i--, n=n<<1){
        valorNumerico += n*((int)binario[i]);
    }

    return valorNumerico;
}

void binario_para_inteiro(int binarios[], int inteiros[]){

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

void gray_para_binario(int gray[], int binarios[]){

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


void obtem_fenotipo_individuo(int gray[], int binario[], int fenotipo[]){

    int i, j=0;

    gray_para_binario(gray, binario);
   
    for(i=0; i<DIMENSOES_PROBLEMA; i++, j+=TAMANHO_VALOR){

       fenotipo[i] = binario_para_decimal(binario, j, j+TAMANHO_VALOR);
    }
}


int funcao_de_avaliacao(individuo p){

    obtem_fenotipo_individuo(p.genotipo, p.genotipo_binario, p.fenotipo);

    int soma = 0;

    int i;

    for(i=0;i < DIMENSOES_PROBLEMA; i++){
        soma += (int)pow((float)p.fenotipo[i], 2);
    }

    return soma*(-1);
}


__kernel void avaliacao(__global individuo *pop){

    int tid = get_global_id(0);
    pop[tid].aptidao = funcao_de_avaliacao(pop[tid]);	
}
