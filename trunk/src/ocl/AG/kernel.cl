#include "representacao.h"
#include "substituicao.cl"
#include "utils.cl"


void obtem_fenotipo_individuo(short gray[], short fenotipo[]){

    int i, j=0;

    gray_para_binario(gray, fenotipo);
   
    for(i=0; i<DIMENSOES_PROBLEMA; i++, j+=TAMANHO_VALOR){

       fenotipo[i] = binario_para_decimal(fenotipo, j, j+TAMANHO_VALOR);
    }
}


int funcao_de_avaliacao(individuo p, short fenotipo[]){    

    obtem_fenotipo_individuo(p.genotipo, fenotipo);

    int soma = 0;

    int i;

    for(i=0;i < DIMENSOES_PROBLEMA; i++){
        soma += (int)pow((float)fenotipo[i], 2);
    }

    return soma*(-1);
}

void inicializa_individuo(individuo pop, cburng4x32 *rng){

    for(int j=0; j< TAMANHO_INDIVIDUO; j++){
        pop.genotipo[j] = rand(rng) % 2;           
    }
}

int torneio(int indice_participante, individuo *populacao, cburng4x32 *rng) {	

    int vencedor = indice_participante % (TAMANHO_POPULACAO);
    int i, aleatorio = 0;
    
    while( i < TAMANHO_TORNEIO) {

        aleatorio = rand(rng) % (TAMANHO_POPULACAO);
		
        if(populacao[aleatorio].aptidao > populacao[vencedor].aptidao){
            vencedor = aleatorio;
        }

      i++;
    }

    return vencedor;
}

void crossover_um_ponto(individuo *pai1, individuo *pai2, individuo *filho1, individuo *filho2, cburng4x32 *rng){

    int i;

    //Gera número entre 0 e TAMANHO_INDIVIDUO-1
    int ponto = rand(rng) % (TAMANHO_INDIVIDUO);

    for(i=0;i<=ponto;i++){
       filho1->genotipo[i] = pai1->genotipo[i];  
       filho2->genotipo[i] = pai2->genotipo[i];      
    }

    for(i=ponto;i<TAMANHO_INDIVIDUO;i++){
       filho1->genotipo[i] = pai2->genotipo[i];
       filho2->genotipo[i] = pai1->genotipo[i];
    }
}

void recombinacao(individuo *pai1, individuo *pai2, individuo *filho1, individuo *filho2, float chance, cburng4x32 *rng){

    //gera um número entre 0 e 1
    float aleatorio = u_rand(rng);

    if (aleatorio<chance) {
        crossover_um_ponto(pai1, pai2, filho1, filho2, rng);
    }
    else{

        for(int j=0;j<TAMANHO_INDIVIDUO;j++){
           filho1->genotipo[j] = pai1->genotipo[j];
	   filho2->genotipo[j] = pai2->genotipo[j];
        }
    }
}


void mutacao(individuo *p, float chance, cburng4x32 *rng){

    for(int i=0;i<TAMANHO_INDIVIDUO;i++) {

        //gera um número entre 0 e 1
        float aleatorio = u_rand(rng);

        if (aleatorio<chance) {
           p->genotipo[i] = (p->genotipo[i] + 1) % 2;
        }
    }
}

__kernel void inicializa_populacao(__global individuo *pop){  
    
    int tid = get_global_id(0), 
        lid = get_local_id(0);

    int seed = tid;    

    //Inicializa RNG
    cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = seed;
    rng.ctr.v[0] = 0;

    for(int j=0; j < TAMANHO_INDIVIDUO; j++){
        pop[tid].genotipo[j] = rand(&rng) % 2;           
    }

    __private short fenotipo[DIMENSOES_PROBLEMA];

    pop[tid].aptidao = funcao_de_avaliacao(pop[tid], fenotipo);    
}


__kernel void iteracao(__global individuo *pop, const int geracao, int fixed_seed, __global individuo *newPop){    

    int tid  = get_global_id(0)*2, 
        lid  = get_local_id(0),
	seed = fixed_seed + tid;
	
    //Inicializa RNG
    cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = seed;
    rng.ctr.v[0] = tid;
    
    /*
        Seleção
    */

    individuo pai1, pai2, filho1, filho2;

    int indicePai1 = torneio(tid, &pop, &rng);
    int indicePai2 = torneio(tid+1, &pop, &rng);
	
    pai1 = pop[indicePai1];
    pai2 = pop[indicePai2];
	
    /*
        Recombinação
    */ 	

    recombinacao(&pai1, &pai2, &filho1, &filho2, TAXA_DE_RECOMBINACAO, &rng);	
  
    /*
       Mutação
    */
	
    mutacao(&filho1, TAXA_DE_MUTACAO, &rng);
    mutacao(&filho2, TAXA_DE_MUTACAO, &rng);
	
    /*
       Avaliação
    */

    __private short fenotipo[TAMANHO_INDIVIDUO];

    filho1.aptidao = funcao_de_avaliacao(filho1, fenotipo);
    filho2.aptidao = funcao_de_avaliacao(filho2, fenotipo);
	
    newPop[tid]   = filho1;
    newPop[tid+1] = filho2;
}