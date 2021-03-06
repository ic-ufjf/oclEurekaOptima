/*
 * parser.h
 *
 *  Created on: 02/02/2014
 *      Author: igorsr
 */

#include <stdio.h>

#ifndef PARSER_H_
#define PARSER_H_

#define IS_TERMINAL(T) T[0] != '<'
#define IS_OPERADOR_ARITMETICO(C)( C == '+' || C == '-' || C == '*' || C == '/')

#define FIM_PROGRAMA -1
#define TAMANHO_MAX_PROGRAMA 1000

typedef enum { DEFAULT, NAOTERMINAL, OPERADOR_BINARIO, OPERADOR_UNARIO, NUMERO_INTEIRO, NUMERO_COM_PONTO, VARIAVEL} TipoSimbolo;

typedef enum { T_SOMA, T_SUB, T_MUL, T_DIV} OperadoresBinarios;
typedef enum { T_SEN, T_COS, T_SQRT, T_MENOS } OperadoresUnarios;

/* Definição das estruturas de dados */

typedef struct {
	float v[2];
} type_simbolo;

typedef struct {
	int id;
	char str[10];
} t_elemento;

typedef struct{
	type_simbolo t;
	int proximo;
} t_item_programa;

typedef struct{
	int num_simbolos;
	type_simbolo simbolos[10];
} t_escolha;

typedef struct{
	type_simbolo simbolo;
	int num_escolhas;
	t_escolha escolhas[20];
} t_regra;

/* Protótipo das funções */

char * GetSimboloNT(char * origem);
type_simbolo GetSimboloParser(char * s);

int GetNaoTerminal(char s[]);
int GetOperadorBinario(char s[]);
int GetOperadorUnario(char s[]);
int GetVariavel(char s[]);

void LeVariaveis(char s[]);

int LeBancoDeDados(char arq[], float bancoDeDados[][5]);
void ProcessaLinhaBD(char s[], int indice, float bancoDeDados[][5]);

short GetQtdVariaveis();

void GetNomeElemento(type_simbolo *s, char *nome);

float Avalia(t_item_programa programa[], float registro[]);


#endif /* PARSER_H_ */
