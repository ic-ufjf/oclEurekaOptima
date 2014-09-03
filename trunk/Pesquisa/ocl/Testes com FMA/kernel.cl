#define N 100000

__kernel void vector_add(__global const float *A, __global float *B, __global float *C) {
 
    int i = get_global_id(0);
   
    for(i=0;i<4;i++){
        B[i] = C[i] = 0;    
    }
   
    /*for(i=3; i < N; i++){
        float result1 = fabs((A[i-3]*A[i-2] + A[i-1])-10.65487f);
        B[i] = result1*result1;// A[i-3]*A[i-2] + A[i-1]-10.687f;
        
        result1 = fabs(fma(A[i-3], A[i-2], A[i-1])-10.65487f);
        
        C[i] = result1*result1;//fma(A[i-3], A[i-2], A[i-1])-10.687f;
    }*/
    
   for(i=0; i < N; i++){
        //float x1 = A[i];
        float x1 = -1.000117f;
        //B[i] = ( x1 / ( ( 1.000000 + 1.000000 ) * ( ( ( x1 + ( ( x1 / 1.000000 ) * x1 ) ) * ( 1.000000 - x1 ) ) / ( x1 / x1 ) ) ) );
        
        B[i] = ( x1 / ( ( 1.000000 + 1.000000 ) * ( ( (fma(x1,x1,x1) ) * ( 1.000000 - x1 ) ) / ( x1 / x1 ) ) ) );
                  
        B[i] = ( 1.000000 + 1.000000 ) *(x1 + ( ( x1 / 1.000000 ) * x1 ))* ( 1.000000 - x1 ) / ( x1 / x1 );
        C[i] = x1/(1.000000 + 1.000000) * fma(x1,x1,x1)* ( 1.000000 - x1 ) / ( x1 / x1 );
    }
}
