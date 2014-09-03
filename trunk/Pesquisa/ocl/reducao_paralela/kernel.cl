__kernel void somaOLD(__global int* E,
            __local int* EP)
{ 

    int lo_id = get_local_id(0);
    int gr_id = get_group_id(0);
    int s = get_local_size(0);

    for(;s>0;s/=2){
       barrier(CLK_LOCAL_MEM_FENCE);

       if(lo_id < s){
	 
    	   int value1 = E[lo_id], value2 = E[lo_id+s];
	       EP[lo_id] = value1+value2;
       }

    }

   if(lo_id==0) E[gr_id] = EP[0];
}

__kernel void soma(__global int *E, __local int *EP){ 

    int next_power_of_2 = (int)pown( (float)2.0, (int) ceil( log2( (float) get_local_size(0) ) ) );

    int lo_id = get_local_id(0);
    int gr_id = get_group_id(0);
    int s = next_power_of_2/2;

    EP[lo_id] = E[lo_id];
    EP[lo_id+s] = E[lo_id+s];

    for(;s>0;s/=2){
       barrier(CLK_LOCAL_MEM_FENCE);

       if(lo_id < s && (lo_id + s < get_local_size(0) ) )
	EP[lo_id] += EP[lo_id+s];
    }

    if(lo_id==0) E[gr_id] = EP[0];    
}
