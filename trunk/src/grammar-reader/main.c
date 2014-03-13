/*
 * main.c
 *
 *  Created on: Mar/2014
 *      Author: igor
 *
 *  Implementação da leitura da gramática, decodificação do programa a partir do genótipo (vetor de inteiros) e
 *  avaliação utilizando interpretador.
 */

/*
 * includes
 */

#define DEBUG TRUE

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "parser.h"
#include "gramatica.h"


void InicializaFenotipo( int * fenotipo){

	srand(99413);

	int init;
	for(init=1;init < D_FENOTIPO; init++) fenotipo[init] = rand();
}

int main(){

	/* Variáveis */
	t_regra Gramatica[10];
	t_item_programa programa[TAMANHO_MAX_PROGRAMA];
	int fenotipo[D_FENOTIPO] = {0,1,0};
	float dataBase[20][5];
	int dataBaseSize, i;

	dataBaseSize = LeBancoDeDados("problems/table1.txt", dataBase);

	LeGramatica("grammars/g1.txt", Gramatica);

	InicializaFenotipo(fenotipo);

	Decodifica(Gramatica, fenotipo, programa);

    float erro = 0;

	for(i=0; i< dataBaseSize;i++){
		printf("Registro %d\n", i);
		erro += Avalia(programa, dataBase[i]);
	}

	return 0;
}
