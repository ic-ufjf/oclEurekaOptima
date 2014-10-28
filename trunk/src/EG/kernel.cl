//#define DATABASE(x,y) dataBase[x*NUM_VARIAVEIS + y]

#define DATABASE(row,column) dataBase[(column)*TAMANHO_DATABASE + row]

#include "representacao.h"
#include "substituicao.cl"
#include "utils.cl"
#include "parser.cl"
#include "gramatica.cl"
#include "avaliacao.cl"

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
    pop[gid].genotipo[lid] = rand2(&seed) % 255;    
    
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
    
    /*
        Seleção
    */    
   
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
            ponto = rand2(&seed) % TAMANHO_INDIVIDUO;            
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
    
    if(u_rand2(&seed) <= (float)TAXA_DE_MUTACAO){
        //bitFilho1 = 1 - bitFilho1;
        bitFilho1 = rand2(&seed) % 255;
    }
    
    if(u_rand2(&seed) <= (float)TAXA_DE_MUTACAO){
        bitFilho2 = rand2(&seed) % 255;     
    }
    
    newPop[2*gid].genotipo[lid]   = bitFilho1;
    newPop[2*gid+1].genotipo[lid] = bitFilho2;
    
    D_seeds[tid] = seed;
}
