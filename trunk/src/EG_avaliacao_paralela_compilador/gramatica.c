#include "gramatica.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

int id_regra;

/*
 * Procura na gramática a regra relacionada ao não terminal indicado
 */
int GetRegra(t_regra * gramatica, type_simbolo simbolo){

	int i;

	for(i=0;i<=id_regra;i++){
		if(gramatica[i].simbolo.v[1] == simbolo.v[1]) return i;
	}

	return -1;
}

/*
 * Processa uma derivação de um não terminal.
 * Procura por símbolos (não terminais ou terminais) e adiciona à regra um type_simbolo para cada um.
 */
void ProcessaEscolha(char * escolha, t_escolha * nova_escolha ){

    #undef DEBUG

	nova_escolha->num_simbolos=0;

	char * elemento, *saveptr;

	elemento = strtok_r(escolha, DELIMITADOR_TERMINAIS, &saveptr);

	/* Encontra cada elemento da derivação*/
	while(elemento != NULL) {

		elemento = trim(elemento);

		if(strlen(elemento)!=0) {

			type_simbolo novo_simbolo = GetSimboloParser(elemento);

			if(novo_simbolo.v[0]==-1){
				 printf("Gramatica inválida. (Simbolo terminal %s desconhecido).", elemento);
				 exit (EXIT_FAILURE);
			}
			else{

				char *nomeElemento = (char*)malloc(20*sizeof(char));
				GetNomeElemento(&novo_simbolo, nomeElemento);

                #ifdef DEBUG
				    printf("Simbolo encontrado: %s (%d,%f) \n", nomeElemento, (int)novo_simbolo.v[0], novo_simbolo.v[1]);
                #endif

			}

			nova_escolha->simbolos[nova_escolha->num_simbolos].v[0] = novo_simbolo.v[0];
			nova_escolha->simbolos[nova_escolha->num_simbolos].v[1] = novo_simbolo.v[1];
			nova_escolha->num_simbolos++;
		}

		elemento = strtok_r(NULL, DELIMITADOR_TERMINAIS, &saveptr);
	}
}

void LeGramatica(char nomeArquivo[], t_regra  * Gramatica){

#undef DEBUG

	/* Estrutura para ler o arquivo da gramática  */
	FILE *fptr = NULL;

	char linha[200]="", aux[200] = "";

	//Abre o arquivo contendo a gramática para leitura
	fptr = fopen(nomeArquivo, "r");

	#ifdef DEBUG
		printf("Gramática: \n");
	#endif

	id_regra = -1;
	int quantidade_escolhas = 0;

	while(fgets(linha, 200, fptr) != NULL){

		#ifdef DEBUG
			printf("Linha - %s\n", linha);
		#endif

		//Verifica se a linha é o início de uma regra
		char * busca_regra = strstr(linha, DELIMITADOR_REGRAS);

		if(busca_regra != NULL){

			id_regra++;

			quantidade_escolhas = 0;

			strcpy(aux, linha);

			char * pnt, * pntEscolhas;

			pnt = strtok(aux, DELIMITADOR_REGRAS);

			char * token = GetSimboloNT(pnt);

			#ifdef DEBUG
				printf("Regra: %s\n", token);
			#endif

			/* Insere o não terminal na tabela de símbolos da gramática */
			type_simbolo s = GetSimboloParser(token);

			Gramatica[id_regra].simbolo.v[0] = s.v[0];
			Gramatica[id_regra].simbolo.v[1] = s.v[1];

			pnt = strtok(NULL, DELIMITADOR_REGRAS);

			#ifdef DEBUG
				printf("Derivações: %s\n", pnt);
			#endif

			pntEscolhas = strtok(pnt, DELIMITADOR_ESCOLHAS);

			while(pntEscolhas!= NULL )
			{
				ProcessaEscolha(pntEscolhas, &Gramatica[id_regra].escolhas[quantidade_escolhas]);

				if(Gramatica[id_regra].escolhas[quantidade_escolhas].num_simbolos>0){
					quantidade_escolhas++;
				}

				pntEscolhas = strtok( NULL, DELIMITADOR_ESCOLHAS);
			}

			Gramatica[id_regra].num_escolhas = quantidade_escolhas;
		}
		else{

			/*Contém derivações da última regra lida */

			strcpy(aux, linha);

			#ifdef DEBUG
				printf("Derivações: %s\n", aux);
			#endif

			char *pntEscolhas = strtok(aux, DELIMITADOR_ESCOLHAS);

			while(pntEscolhas != NULL )
			{
				ProcessaEscolha(pntEscolhas, &Gramatica[id_regra].escolhas[quantidade_escolhas]);

				if(Gramatica[id_regra].escolhas[quantidade_escolhas].num_simbolos>0){
					quantidade_escolhas++;
				}

				pntEscolhas = strtok( NULL, DELIMITADOR_ESCOLHAS );
			}

			Gramatica[id_regra].num_escolhas = quantidade_escolhas;
		}
	}
}

int Decodifica(t_regra * Gramatica, short * fenotipo, t_item_programa * programa){

    #undef DEBUG

	int m, fenotipo_ctr = 0;

	/* Inicializa o programa com o símbolo inicial */
	type_simbolo inicial = GetSimboloParser("<expr>");
	programa[0].t.v[0] = inicial.v[0];
	programa[0].t.v[1] = inicial.v[1];
	programa[0].proximo = FIM_PROGRAMA;

	int program_ctr=1;

	#ifdef DEBUG
		ImprimeIndividuo(programa);
	#endif

	fenotipo_ctr = 0;

	//Mapeamento
	while(1){

		/* Verifica se todo o genótipo foi utilizado */
		if(fenotipo_ctr == DIMENSOES_PROBLEMA-1){
			program_ctr = -1;
			break;
		}

		/* Procura primeiro não terminal à esquerda */
		int i=0;

		while((int)programa[i].t.v[0] != NAOTERMINAL && programa[i].proximo != -1) {

			//i++;
			i = programa[i].proximo;

		}

		/* Verifica se há somente terminais */
		if((int)programa[i].t.v[0] != NAOTERMINAL) break;

		int idRegra = GetRegra(Gramatica, programa[i].t);

		int opcao = fenotipo[fenotipo_ctr++] % Gramatica[idRegra].num_escolhas;

		int proximoAnterior = programa[i].proximo;

		/* Substitui pelo primeiro simbolo */

		type_simbolo auxs = Gramatica[idRegra].escolhas[opcao].simbolos[0];
		programa[i].t.v[0] = auxs.v[0];
		programa[i].t.v[1] = auxs.v[1];

		if(Gramatica[idRegra].escolhas[opcao].num_simbolos>1){

			programa[i].proximo = program_ctr;

			for(m=1;m < Gramatica[idRegra].escolhas[opcao].num_simbolos;m++){

				programa[program_ctr].t = Gramatica[idRegra].escolhas[opcao].simbolos[m];
				programa[program_ctr].proximo = program_ctr+1;

				program_ctr++;
			}

			programa[program_ctr-1].proximo = proximoAnterior;
		}

		#ifdef DEBUG
			ImprimeIndividuo(programa);
		#endif
	}

	#ifdef DEBUG
		printf("\nIndividuo:\n");
		ImprimeIndividuo(programa);
	#endif

	return program_ctr;
}
