void obtem_fenotipo_individuo2(short gray[], short fenotipo[]){
	
    int i, j=0;
    
    short genotipo_binario[TAMANHO_INDIVIDUO];

    gray_para_binario(gray, genotipo_binario);
   
    for(i=0; i<DIMENSOES_PROBLEMA; i++, j+=TAMANHO_VALOR){
       fenotipo[i] = binario_para_decimal(genotipo_binario, j, j+TAMANHO_VALOR);
    }   
}

int obtem_fenotipo_individuo3(individuo p, short fenotipo[]){    

    obtem_fenotipo_individuo2(p.genotipo, fenotipo);   
}


#define DATABASE(x,y) dataBase[x*NUM_VARIAVEIS + y]

__kernel void avaliacao(__global individuo *pop, 
			__global t_regra * Gramatica,
			__global float * dataBase){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0);
        
        __private short fenotipo[DIMENSOES_PROBLEMA];
                
	__local t_item_programa programa[TAMANHO_MAX_PROGRAMA]; 	
 	__local int erro;
 	 
	erro = 0;
	obtem_fenotipo_individuo3(pop[gid], fenotipo);
	
	int program_ctr = Decodifica(Gramatica, fenotipo, programa);	 	
 	
 	if(program_ctr == -1){
	    pop[gid].aptidao = MAXFLOAT*(-1);
 	}
 	else{
 	
 	    float erro = 0;
 		 	
 	    for(int j=0; j < TAMANHO_DATABASE; j++){
	        erro += pown(Avalia(programa, dataBase, j), 2);
	        
	        if( isinf( erro ) || isnan( erro ) ) { erro = MAXFLOAT; break; }
	    }   

   	    if(erro == MAXFLOAT) pop[gid].aptidao= MAXFLOAT*(-1);
   	       	    	
	    else{
	    	pop[gid].aptidao = erro*(-1.0);
    	    }
	    
	}	
}


__kernel void avaliacao_gpu(__global individuo *pop, 
			__global t_regra * Gramatica,
			__global float * dataBase,
			__local float * erros){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0);
        
        __private short fenotipo[DIMENSOES_PROBLEMA];
                
	 __local t_item_programa programa[TAMANHO_MAX_PROGRAMA]; 	
 	 
 	 __local int program_ctr;
 	 
   	 //1 workitem realiza o mapeamento
	 if(lid==0){ 	 	

		obtem_fenotipo_individuo3(pop[gid], fenotipo);
		
		program_ctr = Decodifica(Gramatica, fenotipo, programa);

	 	if(program_ctr == -1){
 		    pop[gid].aptidao = MAXFLOAT*(-1);
	 	}		 	
 	}
 	 	
 	//Sincronismo local para que todos os workitens acessem o programa mapeado 	
 	barrier(CLK_LOCAL_MEM_FENCE);	
 	
 	if(program_ctr != -1){

		//Avaliação paralela entre work-itens do mesmo work-group
		if(lid <= TAMANHO_DATABASE){		 		

 		    erros[lid] = pown(Avalia(programa, dataBase, lid), 2);

	            int next_power_of_2 = (int)pown( (float)2.0, (int) ceil( log2( (float) get_local_size(0) ) ) );

		    int s = next_power_of_2/2;
		    int LOCAL_SIZE = get_local_size(0);

		    for(;s>0;s/=2){
		       barrier(CLK_LOCAL_MEM_FENCE);

		       if(lid < s && (lid + s < LOCAL_SIZE ) )
			erros[lid] += erros[lid+s];
		    }

		    if(lid==0){

			if( isinf( erros[0] ) || isnan( erros[0] ) ) erros[0] = MAXFLOAT;
			pop[gid].aptidao = erros[0]*(-1);		

		    }

		}	
	}

		
}




