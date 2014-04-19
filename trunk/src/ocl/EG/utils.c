#include <ctype.h>
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include "utils.h"

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

/*
    Obtém o número de registros e variáveis do banco de dados
*/

void get_info_bancoDeDados(char *nomeArquivo, int *tamanho, int *qtdVariaveis){

    (*tamanho) = 0;
    (*qtdVariaveis) = 0;

    check(nomeArquivo != NULL, "Banco de dados inválido");

    FILE * fp = fopen(nomeArquivo, "r");

    check(fp != NULL, "Banco de dados inválido");

    int linhas = 0, ch;

    while(!feof(fp))
    {
        ch = fgetc(fp);
        if(ch == '\n')
        {
            (*tamanho)++;
        }
    }

    rewind(fp);

    (*tamanho)--;

    char linha[200], *saveptr;
	fgets(linha,200, fp);

    char * pntVariaveis = strtok_r(linha, "\t", &saveptr);

    while( pntVariaveis != NULL){
		(*qtdVariaveis)++;
 		 pntVariaveis = strtok_r( NULL, "\t", &saveptr);
	}

	fclose(fp);
}


float ** cria_matriz_float(int m, int n){

    int i;

    float **matriz = (float**) malloc(sizeof(float*) * m);

    printf("%d linhas, %d colunas \n", m,n);

    for (i = 0; i < m; i++)
    {
        matriz[i] = (float*) malloc(sizeof(float) * n);
    }

    return matriz;
}
