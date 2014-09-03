__kernel void group_size(__global int *E){  

    int lo_id = get_local_id(0); 
    int gl_id = get_global_id(0); 
    int gr_id = get_group_id(0);  
    int s = get_local_size(0);  

    E[gl_id] = s;
}
