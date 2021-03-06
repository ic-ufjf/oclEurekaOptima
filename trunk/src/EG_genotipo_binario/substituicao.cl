typedef individuo data_t;
#define getKey(a) (a.aptidao)
#define getValue(a) (0)

__kernel void RankSort(__global const data_t * in,__global data_t * out)
{
  int i = get_global_id(0); 
  int n = get_global_size(0);
  data_t iData = in[i];
  float iKey = getKey(iData);
  
  // Encontra a posição da entrada in[i] no vetor ordenado 
  int pos = 0;
  for (int j=0;j<n;j++)
  {
    float jKey = getKey(in[j]);
    bool smaller = (jKey < iKey) || (jKey == iKey && j < i);
    pos += (smaller) ? 1:0;
  }

  out[pos] = iData;
}

__kernel void teste_async_copy( __global int* pop, __local int* program){
 
  __local uint program_size;
  
  program_size = 10;
  
  program[get_global_id(0)] = -1;
 
  event_t e_copy = async_work_group_copy(program, pop, program_size, 0 );
  
  wait_group_events( 1, &e_copy );
  
  e_copy = async_work_group_copy(pop, program, program_size, 0 );
  
  wait_group_events( 1, &e_copy ); 
}


__kernel void substituicao(__global data_t * geracaoAtual, __global data_t * novaGeracao, __global data_t * saida)
{
    int i = get_global_id(0); 
    int n = get_global_size(0);
    
    //Obtém o item geracaoAtual[i]
    data_t atualData = geracaoAtual[i];
    float itemGeracaoAtual = getKey(atualData);
	
    //Obtém o item novaGeracao[i]
    data_t novaData = novaGeracao[i];
    float itemNovaGeracao = getKey(novaData);

    // Encontra a posição da entrada geracaoAtual[i] e novaGeracao[i] no vetor ordenado utilizando rank sort
    int pos1 = 0, pos2 = 0;
    float jKey1, jKey2;
    bool maior=0;

    for (int j=0;j<n;j++)
    {
        //Conta elementos menores que itemGeracaoAtual
        jKey1 = getKey(geracaoAtual[j]);
        maior = (jKey1 > itemGeracaoAtual) || (jKey1 == itemGeracaoAtual && j > i);
        pos1 += (maior) ? 1:0;

        //Conta elementos menores que itemNovaGeracao
        jKey2 = getKey(novaGeracao[j]);
        maior = (jKey2 > itemNovaGeracao)  || (jKey2 == itemNovaGeracao &&  j > i);
        pos2 += (maior) ? 1:0;
    }
 
    //Mantém a elite da geração atual
    if(pos1 < ELITE){
        saida[pos1] = geracaoAtual[i];
    }

    //Substitui o elemento pelo indivíduo da geração atual.
    //No total serão substitúidos N-ELITE elementos.
    if(pos2 < n-ELITE){
        saida[pos2+ELITE] = novaGeracao[i];	
    }
}


__kernel void substituicao_gpu(__global data_t * geracaoAtual, 
			    __global data_t * novaGeracao,
	    		    __global data_t * saida, 
	    		    __local int *aux1, 
	    		    __local int *aux2)

{
    int i  = get_global_id(0); 
    int n  = get_global_size(0);        
    int wg = get_local_size(0);
  
    int blockSize = wg; //tamanho do bloco
    
    if(i < TAMANHO_POPULACAO){    
    
    	//Obtém o item geracaoAtual[i]
    	float itemGeracaoAtual = getKey(geracaoAtual[i]);

    	//Obtém o item novaGeracao[i]
    	float itemNovaGeracao = getKey(novaGeracao[i]);

    	// Encontra a posição das entradas geracaoAtual[i] e novaGeracao[i] no vetor ordenado utilizando rank sort
    	int pos1 = 0, pos2 = 0;
    	float jKey1, jKey2;
    	bool maior = 0;   
    	
    	// Laço em blocos de tamanho blockSize
  	for (int j=0; j<n; j+=blockSize)
  	{ 
	    //Copia n/blocksize para a memória local
	    barrier(CLK_LOCAL_MEM_FENCE);
	    
	    for (int index=i;index<blockSize;index+=wg)
	    {
	      aux1[index] = getKey(geracaoAtual[j+index]);	      
      	      aux2[index] = getKey(novaGeracao[j+index]);
	    }
        
	    barrier(CLK_LOCAL_MEM_FENCE);

	    // Laço percorrendo os valores copiados para a memória local
	    for (int index=0;index<blockSize;index++)
	    {
	      jKey1 = aux1[index]; //recupera as chaves da memória local
	      jKey2 = aux2[index];
	      
	      //Conta elementos menores que itemGeracaoAtual
	      maior = (jKey1 > itemGeracaoAtual) || (jKey1 == itemGeracaoAtual && (j+index) < i);
	      pos1  = (maior) ? 1:0;
	      
	      //Conta elementos menores que itemNovaGeracao	       
	      maior = (jKey2 > itemNovaGeracao)  || (jKey2 == itemNovaGeracao &&  (j+index) < i);
	      pos2 += (maior) ? 1:0;

	    }
  	}
 
        //Mantém a elite da geração atual
        if(pos1 < ELITE){
	    saida[pos1] = geracaoAtual[i];
        }

        //Substitui o elemento pelo indivíduo da geração atual.
        //No total serão substitúidos N-ELITE elementos.
        if(pos2 < n-ELITE){
	    saida[pos2+ELITE] = novaGeracao[i];	
        }
    }
}
