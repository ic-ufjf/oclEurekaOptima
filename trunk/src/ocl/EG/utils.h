#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include<stdio.h>
#include<stdlib.h>

#define DEBUG TRUE

#define log_arquivo() printf("Arquivo: %s, linha: %d\n",__FILE__, __LINE__)
#define log_error(M) printf("%s\n", M)
#define log_error_code(CODE) printf("Codigo: (%d)\n", CODE)

#define check(A,M) if(!A){log_error(M); log_arquivo();  exit(EXIT_FAILURE);}

char *trim(char *s);

void get_info_bancoDeDados(char *nomeArquivo, int *tamanho, int *qtdVariaveis);

float ** cria_matriz_float(int m, int n);

#endif // UTILS_H_INCLUDED
