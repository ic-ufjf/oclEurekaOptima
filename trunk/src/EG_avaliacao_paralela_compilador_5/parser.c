#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "utils.h"

t_elemento naoTerminais[20];

t_elemento operadores_binarios[20] = {
								{T_SOMA,  "+"},
								{T_SUB,  "-"},
								{T_MUL,  "*"},
								{T_DIV,  "/"}
							 };

t_elemento operadores_unarios[20] = {
								{T_SEN, "sen"},
								{T_COS, "cos"},
								{T_SQRT, "sqrt"},
								{T_MENOS, "menos"}
							 };

t_elemento variaveis[20];

//Ultimo não terminal inserido
int idNaoTerminal = -1;
int idVariavel    = -1;

int GetOperadorBinario(char s[]){

	int i;

	for(i=0;i<20;i++){
		if(!strcmp(s, operadores_binarios[i].str)) return operadores_binarios[i].id;
	}

	return -1;
}

int GetOperadorUnario(char s[]){

	int i;

	for(i=0;i<20;i++){
		if(!strcmp(s, operadores_unarios[i].str)) return operadores_unarios[i].id;
	}

	return -1;
}

int GetVariavel(char s[]){

	int i;

	for(i=0;i<20;i++){
		if(!strcmp(s, variaveis[i].str)) return variaveis[i].id;
	}

	return -1;
}

int GetNaoTerminal(char s[]){

	int i;

	for(i=0;i<=idNaoTerminal;i++){
		if(!strcmp(s, naoTerminais[i].str)) return naoTerminais[i].id;
	}

	/* Não terminal não encontrado */
	idNaoTerminal++;

	naoTerminais[idNaoTerminal].id = idNaoTerminal;
	strcpy(naoTerminais[idNaoTerminal].str, s);

	return idNaoTerminal;
}

short GetQtdVariaveis(){
	return idVariavel+1;
}


void LeVariaveis(char s[]){

	char * pntVariaveis, * saveptr;

	char aux[200];

	strcpy(aux, s);

	pntVariaveis = strtok_r(aux, "\t", &saveptr);

	do{
		idVariavel++;

		variaveis[idVariavel].id = idVariavel;

		strcpy(variaveis[idVariavel].str, pntVariaveis);

		#ifdef DEBUG
			//printf("%s", variaveis[idVariavel].str);
		#endif

		pntVariaveis = strtok_r( NULL, "\t", &saveptr);

		#ifdef DEBUG
			//if(pntVariaveis != NULL) printf("\t");
		#endif
	}
	while( pntVariaveis != NULL );

	idVariavel--;
}


void database_read_line(char s[], Database *bancoDeDados, int i){

	char * valorPtr, *saveptr;
	short j=0;

	valorPtr = strtok_r(s, "\t", &saveptr);

	while(j <= GetQtdVariaveis()){

        /*
            Armazenamento por linha e coluna: i*bancoDeDados->numVariaveis + j
            Armazenamento por coluna e linha (transposta): j*bancoDeDados->numRegistros + i
        */

        int idx = j * bancoDeDados->numRegistros + i;

		bancoDeDados->registros[idx] = atof(valorPtr);

		valorPtr = strtok_r(NULL, "\t", &saveptr);

		#ifdef DEBUG
			//printf("%f\t", bancoDeDados->registros[idx]);
		#endif
        j++;
	}

	#ifdef DEBUG
		//printf("\n");
	#endif
}

Database *database_read(char nomeArquivo[]){

    Database * bancoDeDados = (Database *) malloc(sizeof(Database));

    get_info_bancoDeDados(nomeArquivo, &bancoDeDados->numRegistros, &bancoDeDados->numVariaveis);

    int tamanhoBanco = sizeof(float)*(bancoDeDados->numRegistros)*(bancoDeDados->numVariaveis);

    bancoDeDados->registros = (float*) malloc(tamanhoBanco);

	FILE *arq = fopen(nomeArquivo, "r");

	char linha[200];

	fgets(linha,200, arq);

	#ifdef DEBUG
		//printf("Banco de dados: \n");
	#endif

	/* Processa primeira linha (possui os nomes das variáveis) */
    LeVariaveis(linha);

    int count=0;

	while(fgets(linha,200,arq) != NULL){
		database_read_line(linha, bancoDeDados, count++);
	}

	fclose(arq);
	
	/*
	printf("Dados armazenados (em ordem): \n");
	
	int i;
	for(i=0;i<bancoDeDados->numRegistros*bancoDeDados->numVariaveis;i++){
	    printf("%f\t", bancoDeDados->registros[i]);
	    if((i+1)%(bancoDeDados->numRegistros)==0)
	        printf("\n");
	}*/

	return bancoDeDados;
}


void GetNomeElemento(type_simbolo *s, char *nome){

	switch((int)s->v[0]){

		case NAOTERMINAL:
			 strcpy(nome, naoTerminais[(int)s->v[1]].str);
			break;

		case OPERADOR_BINARIO:
			strcpy(nome, operadores_binarios[(int)s->v[1]].str);
			break;

		case OPERADOR_UNARIO:
			strcpy(nome, operadores_unarios[(int)s->v[1]].str);
			break;

		case VARIAVEL:
			strcpy(nome, variaveis[(int)s->v[1]].str);
			break;

		case NUMERO_INTEIRO:
			sprintf(nome, "%d", (int)s->v[1]);
			break;

		case NUMERO_COM_PONTO:
			sprintf(nome, "%f", s->v[1]);
            strcat(nome, "f");
            //printf("número com ponto flutuante: %s\n",nome);
			break;

		default:
			break;
	}
}

char * GetSimboloNT(char * origem){

    if(strlen(origem)>0){

        char *token = (char*)malloc(sizeof(char)*(strlen(origem)+1));

        int i,j=0;

        for(i=0;i<strlen(origem);i++) {
            if(!isspace(origem[i])){
                token[j++] = origem[i];
            }
        }

        token[i] = '\0';
        return token;
	}

	return origem;
}

type_simbolo GetSimboloParser(char * s){

    #undef DEBUG

	int j=0;
	type_simbolo simbolo;

	#ifdef DEBUG
		printf("Elemento: %s\n", s);
	#endif

	//short teste = strlen(s);

	if(s[0]=='<' ){ //&& s[strlen(s)-1]=='>'){

		simbolo.v[0] = NAOTERMINAL;

		//Obtem id do não terminal
		simbolo.v[1] = GetNaoTerminal(s);

		#ifdef DEBUG
			printf("Não terminal: %d \n", (int)simbolo.v[1]);
		#endif
	}

	else if(isdigit(s[0])){

		while(isdigit(s[j])) j++;

		if(s[j]!= '.'){
			simbolo.v[0] = NUMERO_INTEIRO;
			simbolo.v[1] = atoi(s);

			#ifdef DEBUG
				puts("Numero inteiro");
			#endif
		}
		else{

			j++;

			while(isdigit(s[j])) j++;

			simbolo.v[0] = NUMERO_COM_PONTO;
			simbolo.v[1] = atof(s);

			#ifdef DEBUG
				puts("Numero com ponto");
			#endif
		}
	}
	else if(GetOperadorBinario(s)!=-1){

		simbolo.v[0] = OPERADOR_BINARIO;
		simbolo.v[1] = GetOperadorBinario(s);

		#ifdef DEBUG
			printf("Operador binário: %d \n",(int)simbolo.v[1]);
		#endif
	}

	else if(GetOperadorUnario(s)!=-1){

		simbolo.v[0] = OPERADOR_UNARIO;
		simbolo.v[1] = GetOperadorUnario(s);

		#ifdef DEBUG
			printf("Operador unário: %d \n",(int)simbolo.v[1]);
		#endif
	}

	else if(GetVariavel(s)!=-1){

		simbolo.v[0] = VARIAVEL;
		simbolo.v[1] = GetVariavel(s);
		#ifdef DEBUG
			printf("Variavel: %d \n", (int)simbolo.v[1]);
		#endif
	}
	else{
		/* Elemento inválido */
		simbolo.v[0] = -1;
		simbolo.v[1] = 0;
	}

	#ifdef DEBUG
		puts("");
	#endif

	return simbolo;
}

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
   int topo   = -1;
   float erro = 0;

   short indiceY = GetQtdVariaveis();

   int i=0;

   while(i != FIM_PROGRAMA){

	   switch((int)programa[i].t.v[0])
	   {
	   	   case NUMERO_INTEIRO:
	   		   //printf(" (num) ");
	   		   pilha[++topo] = programa[i].t.v[1];
	   		   break;
	   	   case NUMERO_COM_PONTO:
	   		   //printf(" (num) ");
	   		   pilha[++topo] = programa[i].t.v[1];
			   break;
	   	   case VARIAVEL:
	   		   //printf(" (Variável) ");
	   		   pilha[++topo] = registro[(int)programa[i].t.v[1]];
	   		   break;
	   	   case OPERADOR_BINARIO:
	   		   //printf(" (op) ");
	   		   pilha[topo-1] = OperaBinario(pilha[topo-1], pilha[topo], programa[i].t.v[1]);
	   		   topo--;
	   		   break;
	   	   case OPERADOR_UNARIO:
			   //printf(" (op unario) ");
			   pilha[topo] = OperaUnario(pilha[topo], programa[i].t.v[1]);
			   break;
	   }

	   //printf("Topo (%d): %f\n",topo, pilha[topo]);
	   i = programa[i].proximo;
   }

   #ifdef DEBUG
   	   printf("Avaliacao: %f \t Y = %f \t erro = %f\n", pilha[topo], registro[indiceY], fabs( pilha[topo] - registro[indiceY] ));
   #endif

   //Erro absoluto
   return fabs( pilha[topo] - registro[indiceY] );
}

No * EmpilhaExpressao(No * pilha, char * expressao){

    No * novo = (No *) malloc(sizeof(No));
    novo->proximo = pilha;
    strcpy(novo->expr, expressao);
    
    return novo;
}

void ImprimePosfixa(t_item_programa * programa){

	char nomeElemento[20];

	int idx = 0;

	while(idx != FIM_PROGRAMA){

		GetNomeElemento(&programa[idx].t, nomeElemento);
		printf("(%s) ", nomeElemento);
		idx = programa[idx].proximo;
	}

	printf("\n");
}

void ImprimeInfixa(t_item_programa *programa){

   char text[TAMANHO_MAX_PROGRAMA*10];
  
   GetProgramaInfixo(programa, text);
    
   puts(text);

}

void GetProgramaInfixo(t_item_programa *programa, char * textoPrograma){

   No * p = NULL;

   int i=0;

   char aux1[TAMANHO_MAX_PROGRAMA*10];
   char aux2[TAMANHO_MAX_PROGRAMA*10];

   while(i != FIM_PROGRAMA){

	   switch((int)programa[i].t.v[0])
	   {
	   	   case NUMERO_INTEIRO:

	   		   GetNomeElemento(&programa[i].t, aux2);
	   		   p = EmpilhaExpressao(p, aux2);

	   		   break;

	   	   case NUMERO_COM_PONTO:

               GetNomeElemento(&programa[i].t, aux2);
	   		   p = EmpilhaExpressao(p, aux2);

			   break;
	   	   case VARIAVEL:

               GetNomeElemento(&programa[i].t, aux2);
	   		   p = EmpilhaExpressao(p, aux2);

	   		   break;

	   	   case OPERADOR_BINARIO:

               //strcpy(aux1, "((float)( ");
               strcpy(aux1, "( ");
	   		   strcat(aux1,  p->proximo->expr);

	   		   GetNomeElemento(&programa[i].t, aux2);

               strcat(aux1,  " ");
               strcat(aux1, aux2);

               strcat(aux1,  " ");
               strcat(aux1,  p->expr);
               strcat(aux1,  " ");
               
               /*
                Inclusão de uma operação 'neutra' para impedir que seja utilizada a função fma do OpenCL;
               */
               
               /* 
               if(programa[i].t.v[1] == T_MUL || programa[i].t.v[1] == T_DIV){               
                   strcat(aux1,  "+(0.0)");
               }     
               */          
               
               strcat(aux1,  ")");
               
               No * aux;

               aux = p;
               p = p->proximo;
               free(aux);

               strcpy(p->expr, aux1);

	   		   break;

	   	   case OPERADOR_UNARIO:

			   strcpy(aux1, "");

			   GetNomeElemento(&programa[i].t, aux1);

              // strcat(aux1, "((float)(");
               strcat(aux1, "(");
               strcat(aux1,  p->expr);
               strcat(aux1, ")");

               strcpy(p->expr, aux1);

			   break;
	   }

	   i = programa[i].proximo;
   }
    
   strcpy(textoPrograma, p->expr);

//   puts(p->expr);

   free(p);

}
