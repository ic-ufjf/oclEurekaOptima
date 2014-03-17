#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable

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



__kernel void avaliacao(__global individuo *pop, 
			__global t_regra * Gramatica,
			__global float dataBase[][5]){
	
	int tid = get_global_id(0),
   	    lid = get_local_id(0),
   	    gid = get_group_id(0);
        
        __private short fenotipo[DIMENSOES_PROBLEMA] = {245,34,196,37,213,243,239,186,74,63,59,170,72,192,22,136,122,93,81,221,149,131,77,18,68,97,243,204,177,71};
                
	 __local t_item_programa programa[TAMANHO_MAX_PROGRAMA]; 	
 	 __local int erro;
 	 
 	 erro = 0;
 	 
   	 //1 workitem realiza o mapeamento
	 if(lid==0){ 	 	

		obtem_fenotipo_individuo3(pop[gid], fenotipo);
		
		int program_ctr = Decodifica(Gramatica, fenotipo, programa);	 	
	 	
	 	if(program_ctr == -1){
 		    pop[gid].aptidao = INF*(-1);
	 	}
	 	else{
	 	
	 	    /*int idx=0;
 	 
	 	    while(idx != FIM_PROGRAMA){
 			
			programas[gid].programa[idx].t.v[0] = programa[idx].t.v[0];
			programas[gid].programa[idx].t.v[1] = programa[idx].t.v[1];
			programas[gid].programa[idx].proximo = programa[idx].proximo;

	 		idx = programa[idx].proximo;
	 	    }*/
	 	
	 	    float erro = 0;
	 		 	
	 	    for(int j=0; j < TAMANHO_DATABASE; j++){
		        erro += pown(Avalia(programa, dataBase[j]), 2);
		    }
		   
		
  		    pop[gid].aptidao = erro*(-1);	 
		}	
 	}
 	
 	//Sincronismo local para que todos os workitens acessem o programa mapeado 	
 	barrier(CLK_LOCAL_MEM_FENCE);	
 	
 	/*if(lid <= TAMANHO_DATABASE){
 	
 		int _erro = (int)Avalia(programa, dataBase[lid]);
		atomic_add(&erro, _erro);
	}
	
	if(lid==0) pop[gid].aptidao = (int)erro; 	
	*/
 	
}


