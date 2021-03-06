__kernel void sort(__global int *a, int k, int j)
{                                                 
      int i = get_global_id(0);             
      int ixj=i^j;                                    
                                 
      if (ixj>i) {         
  	 
	 int iValue = a[i], ixjValue = a[ixj];                           
         
  	 if ((i&k)==0 && iValue>ixjValue){
             a[i]=ixjValue;
             a[ixj]=iValue;
	 }        
         if ((i&k)!=0 && iValue<ixjValue)                 
         { 
             a[i]=ixjValue;
	     a[ixj]=iValue; 
	 }        
      }
}


__kernel void sort_n(__global int *a, int N, int k, int j)
{                                                 
      int i = get_global_id(0);  
        
      int ixj=i^j;                 
                                 
      if(i>=N || ixj >= N) return;

      if (ixj>i) {         
  	 
	 int iValue = a[i], ixjValue = a[ixj];                           
         
  	 if ((i&k)==0 && iValue>ixjValue){
             a[i]=ixjValue;
             a[ixj]=iValue;
	 }        
         if ((i&k)!=0 && iValue<ixjValue)                 
         { 
             a[i]=ixjValue;
	     a[ixj]=iValue; 
	 }        
      }
}


__kernel void sort2(__global int *a, int stage, int passOfStage)
{                                                 
        uint sortIncreasing = 1; // Direction
        uint gid = get_global_id(0);

        uint pairDistance = 1 << (stage - passOfStage);
        uint blockWidth   = 2 * pairDistance;

        uint leftId = (gid % pairDistance) + (gid / pairDistance) * blockWidth;
        uint rightId = leftId + pairDistance;

        int leftElement  = a[leftId];
        int rightElement = a[rightId];
        
        uint sameDirectionBlockWidth = 1 << stage;
        
        sortIncreasing = ((gid/sameDirectionBlockWidth) % 2 == 1) ? (1 - sortIncreasing) : sortIncreasing;

        uint leftKey  = a[leftId];
        uint rightKey = a[rightId];
        
        int greater = leftKey > rightKey ? leftElement : rightElement;
    	int lesser = leftKey > rightKey ? rightElement : leftElement;
        
        a[leftId] = sortIncreasing ? lesser : greater;    
    	a[rightId] = sortIncreasing ? greater : lesser;
}

typedef uint data_t;
#define getKey(a) (a)
#define getValue(a) (0)


__kernel void RankSort(__global const data_t * in,__global data_t * out)
{
  int i = get_global_id(0); 
  int n = get_global_size(0);
  data_t iData = in[i];
  uint iKey = getKey(iData);
  // Encontra a posição da entrada in[i] no vetor ordenado 
  int pos = 0;
  for (int j=0;j<n;j++)
  {
    uint jKey = getKey(in[j]);
    bool smaller = (jKey < iKey) || (jKey == iKey && j < i);
    pos += (smaller) ? 1:0;
  }

  out[pos] = iData;
}
