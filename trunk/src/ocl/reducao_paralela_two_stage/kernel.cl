__kernel void soma_erro(__global int *E, __local int *EP){  

    int lo_id = get_local_id(0); 
    int gl_id = get_global_id(0); 
    int gr_id = get_group_id(0);  
    int s = get_local_size(0);  

    EP[lo_id] = E[gl_id]; 
    barrier(CLK_LOCAL_MEM_FENCE); 

    for(;s>0;s>>=1){ 
        barrier(CLK_LOCAL_MEM_FENCE); 
        if(lo_id < s) EP[lo_id] += EP[lo_id+s]; 
    }

    if(lo_id==0) E[gr_id] = EP[0];
}
__kernel
void reduce(__global int* buffer,
            __local int* scratch,
            __const int length,
            __global int* result) {

  int global_index = get_global_id(0);

  if(global_index > length) return;  

  float accumulator = 0;
  while (global_index < length) {
    float element = buffer[global_index];
    accumulator += element;
    global_index += get_global_size(0);
  }

  // Perform parallel reduction
  int local_index = get_local_id(0);
  scratch[local_index] = accumulator;
  barrier(CLK_LOCAL_MEM_FENCE);
  for(int offset = get_local_size(0) / 2;
      offset > 0;
      offset = offset / 2) {
    if (local_index < offset) {
      float other = scratch[local_index + offset];
      float mine = scratch[local_index];
      scratch[local_index] = mine + other;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  if (local_index == 0) {
    //result[0] = (int)get_local_size(0);
    result[get_group_id(0)] = scratch[0];
  }
}
