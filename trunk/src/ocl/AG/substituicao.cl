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


__kernel void substituicao(__global data_t * geracaoAtual, __global data_t * novaGeracao, __global data_t * saida)
{
    int i = get_global_id(0); 
    int n = get_global_size(0);

    //Obtém o item geracaoAtual[i]
    data_t atualData = geracaoAtual[i];
    int itemGeracaoAtual = getKey(atualData);
	
    //Obtém o item novaGeracao[i]
    data_t novaData = novaGeracao[i];
    int itemNovaGeracao = getKey(novaData);

    // Encontra a posição da entrada geracaoAtual[i] e novaGeracao[i] no vetor ordenado utilizando rank sort
    //TODO: Realizar ordenação utilizando RadixSort (O(N))
    int pos1 = 0, pos2 = 0;
    bool maior=0; int jKey1, jKey2;

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
        saida[pos1] = atualData;
    }
    
    //Substitui o elemento pelo indivíduo da geração atual.
    //No total serão substitúidos N-ELITE elementos.
    if(pos2 < n-ELITE){
        saida[pos2+ELITE] = novaData;	
    }

}


