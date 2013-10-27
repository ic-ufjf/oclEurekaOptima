 #pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void reduce(__global double* buffer,
            __local double* scratch,
            const int length,
            __global double* result) {

  int global_index = get_global_id(0);

  if(global_index >= length) return;  

  double accumulator = 0;
  while (global_index < length) {
    double element = buffer[global_index];
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
       if (local_index < offset && local_index+offset < length ) {
      double other = scratch[local_index + offset];
      double mine = scratch[local_index];
      scratch[local_index] = mine + other;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
  if (local_index == 0) {
    result[get_group_id(0)] = scratch[0];
  }
}
