#define DATABASE(x,y) dataBase[x*NUM_VARIAVEIS + y]

#include "representacao.h"
#include "parser.cl"
#include "gramatica.cl"
#include "utils.cl"

__kernel void avaliacao(__global t_prog *pop,
			__global float * fitness,
			
			#ifdef Y_DOES_NOT_FIT_IN_CONSTANT_BUFFER
	 		__global const
			#else
			__constant 
			#endif 
 			float * dataBase){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0);

    __local t_item_programa programa[TAMANHO_MAX_PROGRAMA];   	    
 	__local int erro;
   	    
	if(lid==0){		
		
	    int idx = 0;
	 	    
 	    while(idx!=-1){
 	        programa[idx] = pop[gid].programa[idx];
 	        idx =  pop[gid].programa[idx].proximo;   
 	    }
 	    	 	 
		erro = 0;	
		
	 	if(programa[0].t.v[0] == -1){
		    fitness[gid] = MAXFLOAT*(-1);
	 	}
	 	else{
	 	
	 	    float erro = 0;
	 		 	
	 	    for(int j=0; j < TAMANHO_DATABASE; j++){
			    erro += pown(Avalia(programa, dataBase, j), 2);			
			    if( isinf( erro ) || isnan( erro ) ) { erro = MAXFLOAT; break; }
		    }   

	   	    if(erro == MAXFLOAT) fitness[gid] = MAXFLOAT*(-1);
	   	       	    	
		    else{
		    	fitness[gid] = erro*(-1.0);
    	    }
		    
		}
	}	
}

__kernel void avaliacao_gpu(__global t_prog *pop, 
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
                
	__local t_item_programa programa[TAMANHO_MAX_PROGRAMA]; 	 	 

   	//1 workitem realiza a cópia do programa da memória global -> local
	if(lid==0){		
		
	 	if(pop[gid].programa[0].t.v[0] == -1){
		    fitness[gid] = MAXFLOAT*(-1);
	 	}	 		 	
	 	else{
	 	    int idx = 0;
	 	    
	 	    while(idx!=-1){
	 	        programa[idx] = pop[gid].programa[idx];
	 	        idx =  pop[gid].programa[idx].proximo;   
	 	    }	 	
	 	}
 	}
 	 	
 	//Sincronismo local para que todos os workitens acessem o programa mapeado 	
 	barrier(CLK_LOCAL_MEM_FENCE);	
 	
 	if(pop[gid].programa[0].t.v[0] != -1){
		
  		 erros[lid] = 0;
		 //Avaliação paralela entre work-itens do mesmo work-group

		#ifndef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
		   for( uint iter = 0; iter < TAMANHO_DATABASE/LOCAL_SIZE; ++iter )
		   {	
		#else
   		   for( uint iter = 0; iter < ceil( TAMANHO_DATABASE / (float) LOCAL_SIZE ); ++iter )
   		   {

              if( iter * LOCAL_SIZE + lid < TAMANHO_DATABASE )
              {
    	#endif		
			    erros[lid] += pown(Avalia(programa, dataBase, iter * LOCAL_SIZE + lid), 2);
		
		#ifdef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
		      }
		#endif		
			
		  }

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
