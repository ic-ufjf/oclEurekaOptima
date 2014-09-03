#define DATABASE(x,y) dataBase[x*NUM_VARIAVEIS + y]

#include "representacao.h"
#include "parser.cl"
#include "gramatica.cl"
#include "utils.cl"
//#define FUNCAO_OBJETIVO(x1) (x1*x1*2)

__kernel void avaliacao(
            __global float * fitness,			
			#ifdef Y_DOES_NOT_FIT_IN_CONSTANT_BUFFER
	 		__global const
			#else
			__constant 
			#endif 
 			float * dataBase,
		    __local float * erros){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0),
   	    LOCAL_SIZE = get_local_size(0);
      	    
	if(tid < TAMANHO_DATABASE){
	 	
    	float x1 = DATABASE(tid, 0);
    	float y  = DATABASE(tid, NUM_VARIAVEIS-1);
	
		float result = FUNCAO_OBJETIVO(x1);
		    		
        erros[lid] = pown(fabs(result-y),2);
        
	    if( isinf( erros[lid] ) || isnan( erros[lid] ) ) { erros[lid] = MAXFLOAT; }	    
	   
        int next_power_of_2 = LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2;
	
		  for(int s = next_power_of_2*0.5;s>0 ; s*=0.5){
		  	barrier(CLK_LOCAL_MEM_FENCE);
			
			#ifndef LOCAL_SIZE_IS_NOT_POWER_OF_2
			      if( lid < s )
			#else
			      if(lid < s && (lid + s < LOCAL_SIZE ) )
			#endif		        
			erros[lid] += erros[lid+s];
		  }
		
		  if(lid==0){

            if( isinf( erros[0] ) || isnan( erros[0] ) ) erros[0] = MAXFLOAT;
               fitness[gid] = erros[0]*(-1.0);
		  }
       
	}	
}

__kernel void avaliacao_gpu(
			   __global float * fitness,			
			#ifdef Y_DOES_NOT_FIT_IN_CONSTANT_BUFFER
	 		__global const
			#else
			__constant 
			#endif 
 			float * dataBase,
		    __local float * erros){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0),
   	    LOCAL_SIZE = get_local_size(0);
      	    
	if(tid < TAMANHO_DATABASE){
	 	
    	float x1 = DATABASE(tid, 0);
    	float y  = DATABASE(tid, NUM_VARIAVEIS-1);
	
		float result = FUNCAO_OBJETIVO(x1);
		    		
        erros[lid] = pown(fabs(result-y), 2);
        
	    if( isinf( erros[lid] ) || isnan( erros[lid] ) ) { erros[lid] = MAXFLOAT; }	    
	   
        int next_power_of_2 = LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2;
	
		  for(int s = next_power_of_2*0.5;s>0 ; s*=0.5){
		  	barrier(CLK_LOCAL_MEM_FENCE);
			
			#ifndef LOCAL_SIZE_IS_NOT_POWER_OF_2
			      if( lid < s )
			#else
			      if(lid < s && (lid + s < LOCAL_SIZE ) )
			#endif		        
			erros[lid] += erros[lid+s];
		  }
		
		  if(lid==0){

            if( isinf( erros[0] ) || isnan( erros[0] ) ) erros[0] = MAXFLOAT;
            
            fitness[gid] = erros[0]*(-1.0);
		  }
       
	}
}
