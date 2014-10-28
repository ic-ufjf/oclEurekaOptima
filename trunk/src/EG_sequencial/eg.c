#include "eg.h"
#include "ag.h"
#include "parser.h"
#include "gramatica.h"
#include "utils.h"

void imprime_melhor(individuo * pop, t_regra * gramatica){

	int indice_melhor=0;
	int i;

	for(i=1;i< TAMANHO_POPULACAO; i++){
        if(pop[i].aptidao>pop[indice_melhor].aptidao){
            indice_melhor=i;
        }
	}

	t_item_programa programa[TAMANHO_MAX_PROGRAMA];

	Decodifica(gramatica, pop[indice_melhor].genotipo, programa);

	printf("\nMelhor:\n");
	ImprimeInfixa(programa);
	printf("\nAptidao: %.10f\n", pop[indice_melhor].aptidao);

}

void imprime_populacao(individuo * pop, t_regra * gramatica){

	int i,j;

    t_item_programa programa[TAMANHO_MAX_PROGRAMA];
    int invalidos=0;

	for(i=0;i<TAMANHO_POPULACAO;i++){

		printf("%d - ", i);

   	    int program_ctr = Decodifica(gramatica, &pop[i].genotipo, programa);
		ImprimeInfixa(programa);

		if(program_ctr==-1){
		    invalidos++;
		}

		printf("==> %f\n\n", pop[i].aptidao);
	}
	printf("Inválidos: %d\n", invalidos);
}

void avaliacao(individuo * population, t_regra * gramatica, Database * db){

    int i;

    int invalidos = 0;

    for(i=0; i < TAMANHO_POPULACAO; i++){

       //short fenotipo[DIMENSOES_PROBLEMA];
       t_item_programa programa[TAMANHO_MAX_PROGRAMA];

       //obtem_fenotipo_individuo(&population[i], fenotipo);
       int program_ctr = Decodifica(gramatica, &population[i].genotipo, &programa);

       //Programa inválido
       if(program_ctr==-1){
            population[i].aptidao = -999999999;
            invalidos++;
       }
       else{

            float erro=0;
            int k;

            for(k=0; k < db->numRegistros; k++){
                erro += pow(Avalia(&programa, db->registros, k), 2);
            }

            population[i].aptidao = erro*(-1);
       }
    }
}

void eg(individuo * pop, t_regra *gramatica, Database *dataBase){

	individuo newPop[TAMANHO_POPULACAO];

	int geracao=0;

	cria_populacao_inicial(pop);
	avaliacao(pop, gramatica, dataBase);
	sort(pop);

	while(geracao < NUMERO_DE_GERACOES){

	    printf("-------------------------------------\n");
	    printf("Geração %d:\n", geracao);
		imprime_melhor(pop, gramatica);

		cria_nova_populacao(pop, newPop);
    	avaliacao(newPop, gramatica, dataBase);
		substitui_populacao(pop, newPop);

		geracao++;
	}

    printf("-------------------------------------\n");
    printf("Geração %d:\n", geracao);
	imprime_melhor(pop, gramatica);
}
