
#define FUNCAO_OBJETIVO(x1) ((x1*x1*x1*x1)+(x1*x1*x1)+(x1*x1)+(x1))
/*
void avalia1(int tid, int lid, int gid, int local_size, __global float * dataBase, __local float * erros ){
    erros[lid] = 0;  		 
    #ifndef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
	   for( uint iter = 0; iter < TAMANHO_DATABASE/local_size; ++iter ){	   
	#else
	   for( uint iter = 0; iter < ceil( TAMANHO_DATABASE / (float) local_size ); ++iter ){	   
          if( iter * local_size + lid < TAMANHO_DATABASE){          
	#endif	
	        uint line = iter * local_size + lid;
	        float x1 = DATABASE(line, 0);
	        float result = FUNCAO_OBJETIVO(x1);
	        if(isnan(result) || isinf(result))erros[lid] = MAXFLOAT;            
            else erros[lid] += pown(fabs(result-DATABASE(line, NUM_VARIAVEIS-1)), 2);            
#ifdef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
}
#endif
}
}*/
/*
void avalia(int tid, int lid, int gid, int local_size, __global float * dataBase, __local float * erros ){    
    switch(gid){
    
        case 1:
            avalia1(tid, lid, gid, local_size, dataBase, erros);
            break;
        default:
            avalia1(tid, lid, gid, local_size, dataBase, erros);    
            break;
    }    
}*/

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
	    local_size = get_local_size(0);	
		
	if(tid < TAMANHO_DATABASE){
	
	    avalia(tid, lid, gid, local_size, dataBase, erros);
		
        //Redução local
		uint next_power_of_2 = LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2;
	
		for(uint s = next_power_of_2*0.5;s>0 ; s*=0.5){
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
}
