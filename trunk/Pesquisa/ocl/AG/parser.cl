
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

float OperaBinario(float a, float b, float x){

    if(x == T_SOMA)
        return a+b;
    if(x == T_SUB)
	return a-b;
    if(x == T_MUL)
	return a*b;
    if(x == T_DIV){
	if(b!=0) return a/b;
	else return 1;
    }

    return 0;
}

float OperaUnario(float a, float x){

    if(x == T_SEN)
	return (float)sin(a);
    if(x == T_COS)
	return (float)cos(a);
    if(x == T_SQRT)
	return (float)sqrt(a);
    if(x == T_MENOS){
	return a*(-1);
    }

    return 0;
}

float Avalia(t_item_programa programa[], float registro[]) {

   float pilha[TAMANHO_MAX_PROGRAMA];
   int topo = -1;
   float erro = 0;

   short indiceY = 5;

   int i=0;

   while(i != FIM_PROGRAMA){

	   switch((int)programa[i].t.v[0])
   	   {
	   	   case NUMERO_INTEIRO:
	   		   pilha[++topo] = programa[i].t.v[1];
	   		   break;
	   	   case NUMERO_COM_PONTO:
	   		   pilha[++topo] = programa[i].t.v[1];
			   break;
	   	   case VARIAVEL:
	   		   pilha[++topo] = registro[(int)programa[i].t.v[1]];
	   		   break;
	   	   case OPERADOR_BINARIO:
	   		   pilha[topo-1] = OperaBinario(pilha[topo-1], pilha[topo], programa[i].t.v[1]);
	   		   topo--;
	   		   break;
	   	   case OPERADOR_UNARIO:
			   pilha[topo] = OperaUnario(pilha[topo], programa[i].t.v[1]);
			   break;
	   }

	   i = programa[i].proximo;
   }

   //Erro absoluto
   return fabs( pilha[topo] - registro[indiceY] );
}




