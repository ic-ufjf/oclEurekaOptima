#include "representacao.h"
#include "utils.cl"
#define FUNCAO_OBJETIVO(x1) ((x1*x1*x1*x1)+(x1*x1*x1)+(x1*x1)+(x1))

__kernel void avaliacao_gpu(
			    __global float * fitness,			
         		__global float * dataBase,
	            __local float * erros){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0),
	    local_size = get_local_size(0);
	
     erros[lid] = 0;
     
     //Avaliação paralela entre work-itens do mesmo work-group

     #ifndef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
       for( uint iter = 0; iter < TAMANHO_DATABASE/local_size; ++iter )
       {	
     #else
       for( uint iter = 0; iter < ceil( TAMANHO_DATABASE / (float) local_size ); ++iter )
       {
          if( iter * local_size + lid < TAMANHO_DATABASE)
          {
     #endif	
            int line = iter * local_size + lid;
            
            //float result = funcaoobjetivo(gid, DATABASE(line, 0));           
            //float x1 = DATABASE(line, 0);
            
            float result = funcaoobjetivo(gid, dataBase, line);		    	        		
           
            float y = DATABASE(line, NUM_VARIAVEIS-1);
            
            if(isnan(result) || isinf(result)){
                erros[lid] = MAXFLOAT;
                break;
            }
            else{    	        
                erros[lid] += pown(result-y, 2);
            }

      #ifdef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
          }
      #endif		

      }

      int next_power_of_2 = LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2;

      for(int s = next_power_of_2*0.5;s>0 ; s*=0.5){
      	barrier(CLK_LOCAL_MEM_FENCE);

        #ifndef LOCAL_SIZE_IS_NOT_POWER_OF_2
          if(lid < s )
        #else
          if(lid < s && (lid + s < local_size ) )
        #endif		        
            erros[lid] += erros[lid+s];
      }		

      if(lid==0){

        if( isinf( erros[0] ) || isnan( erros[0] ) ) 
            erros[0] = MAXFLOAT;
       
        fitness[gid] = erros[0]*(-1.0);
      }
}
