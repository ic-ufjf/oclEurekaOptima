#include "representacao.h"
#include "substituicao.cl"
#include "utils.cl"
#include "parser.cl"
#include "gramatica.cl"

#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

int rand2(__global int *aleatorios){

  int al;
	
  al = abs((int)atom_inc(&aleatorios[0]));
	
  return aleatorios[al];  
}

#define RAND_MAX 2147483647	 

float u_rand2(__global int * aleatorios){	
	float a = (float)rand2(aleatorios)/RAND_MAX;
	return a;
}


void obtem_fenotipo_individuo(short gray[], short fenotipo[]){

    int i, j=0;
    
    short genotipo_binario[TAMANHO_INDIVIDUO];

    gray_para_binario(gray, genotipo_binario);
   
    for(i=0; i<DIMENSOES_PROBLEMA; i++, j+=TAMANHO_VALOR){
       fenotipo[i] = binario_para_decimal(genotipo_binario, j, j+TAMANHO_VALOR);
    }
}

int funcao_de_avaliacao(individuo p, short fenotipo[]){    

    obtem_fenotipo_individuo(p.genotipo, fenotipo);

    int soma = 0;

    int i;

    for(i=0;i < DIMENSOES_PROBLEMA; i++){
        soma += (int)pow((float)fenotipo[i]-10, 2);
    }

    return soma*(-1);
}


int torneio(__global individuo *populacao, int indice_participante, cburng4x32 *rng) {	

    int vencedor = indice_participante % TAMANHO_POPULACAO;
    int i=0; int aleatorio = 0;

    while(i < TAMANHO_TORNEIO) {
	
 	aleatorio = rand(rng) % (TAMANHO_POPULACAO);
	
        if(populacao[aleatorio].aptidao > populacao[vencedor].aptidao){
            vencedor = aleatorio;
        }

        i++;
    }

    return vencedor;
}

void crossover_um_ponto(individuo *pais, individuo *filhos, cburng4x32 *rng){

   int i;

    //Gera número entre 0 e TAMANHO_INDIVIDUO-1
    int ponto = rand(rng) % (TAMANHO_INDIVIDUO);

    for(i=0;i<=ponto;i++){
       filhos[0].genotipo[i] = pais[0].genotipo[i];  
       filhos[1].genotipo[i] = pais[1].genotipo[i];      
    }

    for(i=ponto;i<TAMANHO_INDIVIDUO;i++){
       filhos[0].genotipo[i] = pais[1].genotipo[i];
       filhos[1].genotipo[i] = pais[0].genotipo[i];
 
   }
}



void crossover_um_ponto2(__local individuo *pais, __local individuo *filhos, cburng4x32 *rng, int lid, int ponto){

    int i;

    for(i=0;i<=ponto;i++){
       //filhos[0].genotipo[i] = pais[0].genotipo[i];  
       //filhos[1].genotipo[i] = pais[1].genotipo[i];      
       
       filhos[lid].genotipo[i] = pais[lid].genotipo[i];      
    }

    for(i=ponto;i<TAMANHO_INDIVIDUO;i++){
       //filhos[0].genotipo[i] = pais[1].genotipo[i];
       //filhos[1].genotipo[i] = pais[0].genotipo[i];

       filhos[lid].genotipo[i] = pais[1-lid].genotipo[i]; 
   }
}

void crossover_um_ponto3(__local individuo *pais, individuo *filhos, cburng4x32 *rng, int lid, int ponto){

    int i, indice1, indice2;

    if(lid % 2 ==0){
	indice1 = lid;
        indice2 = indice1+1;
    }
    else{
	indice1 = lid;
        indice2 = indice1-1;
    }
    
    for(i=0;i<=ponto; i++){       
       filhos[0].genotipo[i] = pais[indice1].genotipo[i];      
    }

    for(i=ponto;i<TAMANHO_INDIVIDUO;i++){
       filhos[0].genotipo[i] = pais[indice2].genotipo[i]; 
   }
}

void crossover_um_ponto4(individuo *pais, individuo *filhos, cburng4x32 *rng){

    
   int i;

    //Gera número entre 0 e TAMANHO_INDIVIDUO-1
    int ponto = rand(rng) % (TAMANHO_INDIVIDUO);

    for(i=0;i<=ponto;i++){
       filhos[0].genotipo[i] = pais[0].genotipo[i];  
    }

    for(i=ponto;i<TAMANHO_INDIVIDUO;i++){
       filhos[0].genotipo[i] = pais[1].genotipo[i];
   }
}

void recombinacao(individuo *pais, individuo *filhos, float chance, cburng4x32 *rng){

    //Gera um número entre 0 e 1
    float aleatorio = u_rand(rng);

    if (aleatorio<chance) {
        crossover_um_ponto(pais, filhos, rng);
    }
    else{
       filhos[0] = pais[0];
       filhos[1] = pais[1];
    }
}



void recombinacao2(__local individuo *pais, 
		   __local individuo *filhos, 			
 		   __local int * recombinar, cburng4x32 *rng, 
		   int lid, 
		   __local int *pontoCrossOver){

    if (recombinar==1) {
        crossover_um_ponto2(pais, filhos, rng, lid, pontoCrossOver);
    }
    else{
       filhos[lid] = pais[lid];
       //filhos[1-lid] = pais[1-lid];
    }
}

void recombinacao3(__local individuo *pais, 
		   individuo *filhos, 			
 		   __local int * recombinar, 
		   cburng4x32 *rng, 
		   int lid, 
		   __local int *pontosCrossOver){

    if (recombinar==1) {
        crossover_um_ponto3(pais, filhos, rng, lid, pontosCrossOver);
    }
    else{
       filhos[0] = pais[lid];
    }
}

void recombinacao4(individuo *pais, individuo *filhos, float chance, cburng4x32 *rng){

    //Gera um número entre 0 e 1
    float aleatorio = u_rand(rng);

    if (aleatorio<chance) {
       crossover_um_ponto4(pais, filhos, rng);
    }
    else{
       filhos[0] = pais[0];       
    }
}

void mutacao(individuo *p, float chance, cburng4x32 *rng){

    for(int i=0;i<TAMANHO_INDIVIDUO;i++) {
	
        //gera um número entre 0 e 1
        float aleatorio = u_rand(rng);

        if (aleatorio<chance) {
           p->genotipo[i] = (p->genotipo[i]+1) % 2;
        }
    }
}

void mutacao2(__local individuo *p, float chance, cburng4x32 *rng){

    for(int i=0;i<TAMANHO_INDIVIDUO;i++) {
	
        //gera um número entre 0 e 1
        float aleatorio = u_rand(rng);

        if (aleatorio<chance) {
           p->genotipo[i] = (p->genotipo[i]+1) % 2;
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

/*
 KERNEL 0:
 Utiliza TAMANHO_POPULACAO/2 itens de trabalho, cada 1 responsável por selecionar 2 pais
 e gerar 2 filhos, realizando crossover e mutação.
*/
__kernel void iteracao_2_por_work_item(__global individuo *pop, 
			const int geracao, 
			int fixed_seed, 
			__global individuo *newPop,
			__global struct r123array4x32 *counter)
{    

    int tid  = get_global_id(0)*2, 
        lid  = get_local_id(0),
	seed = fixed_seed + tid;

    //Inicializa RNG  
    cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = seed;
    rng.ctr = counter[tid];
 
    /*
	Seleção
    */

    individuo pais[2], filhos[2]; 	

    pais[0] = pop[torneio(pop, tid, &rng)];
    pais[1] = pop[torneio(pop, tid+1, &rng)];

    /*
	Recombinação
    */
 
    recombinacao(pais, filhos, TAXA_DE_RECOMBINACAO, &rng);	
  
    /*
	Mutação
    */
	
    mutacao(&filhos[0], TAXA_DE_MUTACAO, &rng);
    mutacao(&filhos[1], TAXA_DE_MUTACAO, &rng);
	
    /*
       Avaliação
    */

    __private short fenotipo[DIMENSOES_PROBLEMA];

    filhos[0].aptidao = funcao_de_avaliacao(filhos[0], fenotipo);
    filhos[1].aptidao = funcao_de_avaliacao(filhos[1], fenotipo);
    		

    newPop[tid]   = filhos[0];
    newPop[tid+1] = filhos[1]; 

    counter[tid] = rng.ctr;
}

/*
  KERNEL 1:

 Utiliza TAMANHO_POPULACAO itens de trabalho, e 2 itens por unidade de computação. O item de trabalho 0 é responsável por
 selecionar os 2 pais; logo em seguida há um sincronismo, para que o item 1 tenha acesso (via memória local) aos indivíduos selecionados.
 Em seguida cada item realiza sua parte do crossover e a mutação do indivíduo resultante.

*/
__kernel void iteracao_2_por_work_group(__global individuo *pop, 
			const int geracao, 
			int fixed_seed, 
			__global individuo *newPop,
			__global struct r123array4x32 *counter)
{    

    int tid  = get_global_id(0),
        lid  = get_local_id(0),
	seed = fixed_seed + tid;

    //Inicializa RNG  
    cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = seed;
    rng.ctr = counter[tid];

    /*
	Seleção
    */
    __local individuo pais[2], filhos[2];
    __local int recombinar, pontoCrossOver; 

    if(lid == 0){

    	pais[0] = pop[torneio(pop, tid, &rng)];
    	pais[1] = pop[torneio(pop, tid+1, &rng)];

	float aleatorio = u_rand(&rng);
      
        pontoCrossOver = rand(&rng)  % (TAMANHO_INDIVIDUO);

	recombinar = aleatorio < TAXA_DE_RECOMBINACAO;	
    }       

    barrier(CLK_LOCAL_MEM_FENCE);    

    /*
        Recombinação
    */
 
    recombinacao2(pais, filhos, recombinar, &rng, lid, pontoCrossOver);

    /*
	Mutação
    */
	
    mutacao2(&filhos[lid], TAXA_DE_MUTACAO, &rng);
	
    /*
       Avaliação
    */

    __private short fenotipo[DIMENSOES_PROBLEMA];

    filhos[lid].aptidao = funcao_de_avaliacao(filhos[lid], fenotipo);
    		
    newPop[tid]  = filhos[lid];
 
    counter[tid] = rng.ctr;
}


/*
  KERNEL 2**:

 Semelhante ao KERNEL 1, porém utilizando N itens de trabalho por unidade de computação.
 Neste caso, os itens de trabalho pares (tid % 2 == 0) realizam a seleção dos 2 pais, e também 
 a decisão da ocorrência do crossover. É feito então um sincronismo para que todos os itens tenham acesso (via memória local)
 aos indivíduos selecionados.
*/

__kernel void iteracao_n_por_work_group(__global individuo *pop, 
			const int geracao, 
			int fixed_seed, 
			__global individuo *newPop,
			__global struct r123array4x32 *counter,
			__local individuo *pais, 
			__local int *recombinar, 
			__local int *pontosCrossover)
{    

    int tid  = get_global_id(0),
        lid  = get_local_id(0),
	seed = fixed_seed + tid;
    
    if(tid >= TAMANHO_POPULACAO) return;

    //Inicializa RNG  
    cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = seed;
    rng.ctr = counter[tid];

    /*
	Seleção
    */

    int index = (int)lid/2;

    if(lid % 2 == 0){

    	pais[lid]    = pop[torneio(pop, tid, &rng)];
    	pais[lid+1]  = pop[torneio(pop, tid+1, &rng)];

	/*
           Recombinação
        */

	float aleatorio = u_rand(&rng);

        pontosCrossover[index] = rand(&rng)  % (TAMANHO_INDIVIDUO);

	recombinar[index] = aleatorio < TAXA_DE_RECOMBINACAO;       
    }       

    barrier(CLK_LOCAL_MEM_FENCE);    

    individuo filhos[1];
	
    recombinacao3(pais, filhos, recombinar[index], &rng, lid, pontosCrossover[index]);

    /*
	Mutação
    */
	
    mutacao(&filhos[0], TAXA_DE_MUTACAO, &rng);
	
    /*
       Avaliação
    */

    //short fenotipo[DIMENSOES_PROBLEMA];
    //filhos[0].aptidao = funcao_de_avaliacao(filhos[0], fenotipo);
    		
    newPop[tid] = filhos[0];
    
    counter[tid] = rng.ctr;
}

