#include "eg.h"
#include "ag.h"
#include "eg_opencl.h"
#include "parser.h"
#include "gramatica.h"
#include "utils.h"

void imprime_melhor(individuo * pop, t_regra * gramatica){
	
	int indice_melhor;

	#ifdef ELITE
	if(ELITE>0){

		if(pop[0].aptidao > pop[ELITE].aptidao){
			indice_melhor =  0;
		}
		else{
			indice_melhor =  ELITE;	
		}

	}
	#else
		indice_melhor =  0;
	#endif

	t_item_programa programa[TAMANHO_MAX_PROGRAMA];
	short fenotipo[DIMENSOES_PROBLEMA];
	obtem_fenotipo_individuo(&pop[indice_melhor], fenotipo);
	Decodifica(gramatica, fenotipo, programa);

	printf("\nMelhor:\n");

	ImprimeInfixa(programa);
	
	printf("\nAptidao: %f\n", pop[indice_melhor].aptidao);	
}

void imprime_populacao(individuo * pop){

	int i,j;

	for(i=0;i<TAMANHO_POPULACAO;i++){
		for(j=0;j<TAMANHO_INDIVIDUO;j++)
			printf("%d",pop[i].genotipo[j]);
		printf("==> %f\n", pop[i].aptidao);
	}
}

void avaliacao(individuo * pop, t_prog * programas, t_regra * gramatica){

    int i;
    short fenotipo[DIMENSOES_PROBLEMA];    
    
    for(i=0; i < TAMANHO_POPULACAO; i++){
        
       obtem_fenotipo_individuo(&pop[i], fenotipo);
       int program_ctr = Decodifica(gramatica, fenotipo, programas[i].programa);       
       
       //Programa inválido
       if(program_ctr==-1){
            programas[i].programa[0].t.v[0]=-1;
       }
       
       //ImprimeInfixa(programas[i].programa);
    }
            
    avaliacao_paralela(pop, programas);    
}

void eg(individuo * pop, t_regra *gramatica, Database *dataBase){
	
	individuo newPop[TAMANHO_POPULACAO];
	t_prog programas[TAMANHO_POPULACAO];

	int geracao=1;
    
    opencl_init(dataBase);
    
    avaliacao_init(gramatica, dataBase);    
    
	cria_populacao_inicial(pop);	

	avaliacao(pop, programas, gramatica);

	sort(pop);

	while(geracao <= NUMERO_DE_GERACOES){

	    printf("-------------------------------------\n");
		printf("Geração %d:\n", geracao);		
		imprime_melhor(pop, gramatica);

		cria_nova_populacao(pop, newPop);
    	avaliacao(newPop, programas, gramatica);
		substitui_populacao(pop, newPop);

		geracao++;
	}	

    opencl_dispose();	
}
