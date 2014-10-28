#define DATABASE(x,y) dataBase[x*NUM_VARIAVEIS + y]

#include "representacao.h"
#include "substituicao.cl"
#include "utils.cl"
#include "parser.cl"
#include "gramatica.cl"
#include "avaliacao.cl"

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

    if (aleatorio<=chance) {
       crossover_um_ponto4(pais, filhos, rng);
    }
    else{
       filhos[0] = pais[0];       
    }
}

void mutacao(individuo *p, float chance, cburng4x32 *rng){

    int mutacoes = (int)(TAMANHO_INDIVIDUO * chance);
    int aleatorio=0;
    
    for(int i=0;i<mutacoes;i++) {
	
        aleatorio = rand(rng) % TAMANHO_INDIVIDUO;       
        
        if(p->genotipo[aleatorio])
            p->genotipo[aleatorio] = 0;
        else p->genotipo[aleatorio] = 1;
    }
}

void mutacao2(__local individuo *p, float chance, cburng4x32 *rng){
    
    int mutacoes = (int)(TAMANHO_INDIVIDUO * chance);
    int aleatorio=0;
    
    for(int i=0;i<mutacoes;i++) {
	
        aleatorio = rand(rng) % TAMANHO_INDIVIDUO;       
        
        if(p->genotipo[aleatorio])
            p->genotipo[aleatorio] = 0;
        else p->genotipo[aleatorio] = 1;
    }
}

__kernel void inicializa_populacao(__global individuo *pop, const int seed){  
    
    int tid = get_global_id(0),
        lid = get_local_id(0);

    int lseed = seed+tid;

    //Inicializa RNG
    cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = lseed;
    rng.ctr.v[0] = 0;
    rng.ctr.v[1] = 0;

    for(int j=0; j < TAMANHO_INDIVIDUO; j++){
        pop[tid].genotipo[j] = rand(&rng) % 2;         
    }
}

__kernel void inicializa_populacao2(__global individuo *pop, 
                                    __global int *D_seeds){  
    
    int tid = get_global_id(0),
        lid = get_local_id(0),
        gid = get_group_id(0);

    int seed = D_seeds[tid];

    /*   //Inicializa RNG
        cburng4x32 rng;
        cburng4x32_init(&rng);
        rng.key.v[0] = lseed;
        rng.ctr.v[1] = 0;
    */
    

    /*
        Vantagem: granularidade mais fina e acesso coalescido à memória global.
    */    
    pop[gid].genotipo[lid] = rand2(&seed) % 2;    
    
    D_seeds[tid] = seed;
}

__kernel void selecao(__global individuo *pop, 
//			const int geracao, 
			__global individuo *newPop,
			__global int *D_seeds){
												
			
    int tid  = get_global_id(0)*2, 
        lid  = get_local_id(0),
        gid  = get_group_id(0);

    int seed = D_seeds[tid];

    //Inicializa RNG  
    /*cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = seed;
    
    if(geracao <= 1){
    	 rng.ctr.v[0] = rng.ctr.v[1] = 0;
    }
    else{
    	rng.ctr = counter[tid];
    } */
 
    /*
        Seleção
    */
    
    //individuo pais[2];
    
    /* DOCUMENTAR ALTERAÇÃO NO TORNEIO E NA SELEÇÃO (speedup de 1.29) */
    
    __local int indicePais[2];    
    __local int melhores1[TAMANHO_TORNEIO];
    __local float aptidoes1[TAMANHO_TORNEIO];
    __local int melhores2[TAMANHO_TORNEIO];
    __local float aptidoes2[TAMANHO_TORNEIO];
    
    if(lid < TAMANHO_TORNEIO){
    
        int aleatorio = 0;
        
        aleatorio = rand2(&seed) % TAMANHO_POPULACAO;
	    
	    melhores1[lid] = aleatorio;
        aptidoes1[lid] = pop[aleatorio].aptidao;
        
        aleatorio = rand2(&seed) % TAMANHO_POPULACAO;
        
        melhores2[lid] = aleatorio;
        aptidoes2[lid] = pop[aleatorio].aptidao;     
    }    

    barrier(CLK_LOCAL_MEM_FENCE);  
    
    if(lid == 0){      
      
        int idmelhor1=0, idmelhor2=0;
        
        for(int i=1; i < TAMANHO_TORNEIO; i++){
        
            if(aptidoes1[i] > aptidoes1[idmelhor1]){
                idmelhor1 = i;
            }
            
            if(aptidoes2[i] > aptidoes2[idmelhor2]){
                idmelhor2 = i;
            }
        }  
        
        indicePais[0] = melhores1[idmelhor1];
        indicePais[1] = melhores2[idmelhor2];
    }   
    
    /*if(lid == 0){        
        indicePais[0] = torneio(pop, gid, &rng);
        indicePais[1] = torneio(pop, gid+1, &rng);        
    }*/
    
    barrier(CLK_LOCAL_MEM_FENCE);

    //pais[0] = pop[torneio(pop, tid, &rng)];
    //pais[1] = pop[torneio(pop, tid+1, &rng)];
    
    newPop[gid].genotipo[2*lid]   = pop[indicePais[0]].genotipo[2*lid];
    newPop[gid].genotipo[2*lid+1] = pop[indicePais[1]].genotipo[2*lid+1];        
			
    //newPop[tid]   = pais[0];
    //newPop[tid+1] = pais[1];
    
    //if(lid==0)
    //counter[tid] = rng.ctr;
        
    D_seeds[tid] = seed;
}

__kernel void crossOverEMutacao(__global individuo *pop, 
		//	const int geracao, 
			__global individuo *newPop,
			__global int * D_seeds)
{
     int tid  = get_global_id(0)*2, 
        lid   = get_local_id(0),
        gid   = get_group_id(0);
        
    int seed  = D_seeds[tid];

    /*//Inicializa RNG  
    cburng4x32 rng;
    cburng4x32_init(&rng);
    rng.key.v[0] = seed;
    
    if(geracao <= 1){
    	 rng.ctr.v[0] = rng.ctr.v[1] = 0;
    }
    else{
    	rng.ctr = counter[tid];
    }*/
    
    /* Cada grupo de trabalho realiza o crossover entre os elementos pop[2*gid]
     e pop[2*gid+1];
     Cada item de trabalho é responsável por um bit do cromossomo;
     A decisão de realizar o xover e o ponto de corte são tomadas pelo item 0. 
    */

    __local int crossOver;
    __local int ponto;
    
    if(lid == 0){
        //Gera um número entre 0 e 1
        float aleatorio = u_rand2(&seed);
        if(aleatorio <= (float)TAXA_DE_RECOMBINACAO){
            crossOver = 1;            
            ponto = rand(&seed) % TAMANHO_INDIVIDUO;            
        }        
        else{
            crossOver = 0;			
        }
    }    
    
    barrier(CLK_LOCAL_MEM_FENCE);
    
    int bitFilho1, bitFilho2;    
    
    if(crossOver){
    
        if(lid > ponto){           
            bitFilho1 = newPop[2*gid+1].genotipo[lid];
            bitFilho2 = newPop[2*gid].genotipo[lid];
        }
        else{
            bitFilho1 = newPop[2*gid].genotipo[lid];
            bitFilho2 = newPop[2*gid+1].genotipo[lid];
        }
    } 
    else{
        bitFilho1 = newPop[2*gid].genotipo[lid];
        bitFilho2 = newPop[2*gid+1].genotipo[lid];
    }
        
    /* MUTAÇÃO */
    
    /* TODO: Somente as threads (0..TAMANHO_INDIVIDUO*TAXA_DE_MUTACAO) */   
   
    if(u_rand2(&seed) <= (float)TAXA_DE_MUTACAO){
        bitFilho1 = 1 - bitFilho1;
    }
    
    if(u_rand2(&seed) <= (float)TAXA_DE_MUTACAO){
        bitFilho2 = 1 - bitFilho2;        
    }
    
    newPop[2*gid].genotipo[lid]   = bitFilho1;
    newPop[2*gid+1].genotipo[lid] = bitFilho2;
    
    D_seeds[tid] = seed;
}

/*
 KERNEL 0:
 Utiliza TAMANHO_POPULACAO/2 itens de trabalho, cada 1 responsável por selecionar 2 pais
 e gerar 2 filhos, realizando crossover e mutação.
*/
__kernel void iteracao_2_por_work_item(__global individuo *pop, 
			const int geracao, 
			const int fixed_seed, 
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
    
    if(geracao <= 1){
    	 rng.ctr.v[0] = rng.ctr.v[1] = 0;
    }
    else{
    	rng.ctr = counter[tid];
    } 
 
    /*
	    Seleção
    */

    individuo pais[2], filhos[2]; 	

    pais[0] = pop[torneio(pop, tid, &rng)];
    pais[1] = pop[torneio(pop, tid+1, &rng)];

    /*
	Recombinação
    */
 
    recombinacao(pais, filhos, (float)TAXA_DE_RECOMBINACAO, &rng);	
  
    /*
	Mutação
    */
	
    mutacao(&filhos[0], TAXA_DE_MUTACAO, &rng);
    mutacao(&filhos[1], TAXA_DE_MUTACAO, &rng);
	
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
    
    if(geracao <= 1){
    	 rng.ctr.v[0] = rng.ctr.v[1] = 0;
    }
    else{
    	rng.ctr = counter[tid];
    } 

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

    /*__private short fenotipo[DIMENSOES_PROBLEMA];
    filhos[lid].aptidao = funcao_de_avaliacao(filhos[lid], fenotipo);*/
    		
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
    
    if(geracao <= 1){
    	 rng.ctr.v[0] = rng.ctr.v[1] = 0;
    }
    else{
    	rng.ctr = counter[tid];
    } 

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

