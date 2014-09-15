void reducao_paralela( __global float * fitness, __local float  * erros, uint gid, uint lid, const int offset){

    for(uint s = LOCAL_SIZE_ROUNDED_UP_TO_POWER_OF_2*0.5;s>0 ; s*=0.5){
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
       
        fitness[offset/*+gid*/] = erros[0]*(-1.0);
    }    
}

/*
__kernel void avaliacao_gpu(
			    __global float * fitness,			
         		__global float * dataBase,
	            __local float  * erros,
	            const int offset){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0),
	    local_size = get_local_size(0);
	
     //Avaliação paralela entre work-itens do mesmo work-group
     #ifndef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
       for( uint iter = 0; iter < TAMANHO_DATABASE/local_size; ++iter )
       {
	uint line = iter * local_size + lid;	
     #else
       for( uint iter = 0; iter < ceil( TAMANHO_DATABASE / (float) local_size ); ++iter )
       {
          uint line = iter * local_size + lid;
          if( line < TAMANHO_DATABASE)
          {
     #endif	                      
            float result = funcaoobjetivo(offset+gid, dataBase, line);
            float y = DATABASE(line, NUM_VARIAVEIS-1);

            erros[lid] += pown(result-y, 2);

      #ifdef NUM_POINTS_IS_NOT_DIVISIBLE_BY_LOCAL_SIZE
          }
      #endif
      }
      reducao_paralela(fitness, erros, gid, lid, offset);     
}*/
