#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <stdio.h>

#define IS_TERMINAL(T) T[0] != '<'
#define IS_OPERADOR_ARITMETICO(C)( C == '+' || C == '-' || C == '*' || C == '/')

#define FIM_PROGRAMA -1
#define TAMANHO_MAX_PROGRAMA 128

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
	type_simbolo simbolos[50];
} t_escolha;

typedef struct{
	type_simbolo simbolo;
	int num_escolhas;
	t_escolha escolhas[20];
} t_regra;

typedef struct{
	t_item_programa programa[TAMANHO_MAX_PROGRAMA];
}t_prog;

typedef struct{

    int numRegistros;
    int numVariaveis;
    float * registros;

}Database;


typedef struct NoExpressao{
    char expr[TAMANHO_MAX_PROGRAMA];
    struct NoExpressao * proximo;
} No;

No * EmpilhaExpressao(No * pilha, char * expressao);

/* Protótipo das funções */

char * GetSimboloNT(char * origem);
type_simbolo GetSimboloParser(char * s);

int GetNaoTerminal(char s[]);
int GetOperadorBinario(char s[]);
int GetOperadorUnario(char s[]);
int GetVariavel(char s[]);

void LeVariaveis(char s[]);

short GetQtdVariaveis();
void GetNomeElemento(type_simbolo *s, char *nome);
void GetNomeElemento2(int type, int value, char *nome);

Database *database_read(char nomeArquivo[]);

float Avalia(t_item_programa programa[], float registro[]);

void ImprimePosfixa(t_item_programa * programa);
void ImprimeInfixa(t_item_programa * programa);


#endif // PARSER_H_INCLUDED
