#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <CL/cl.h>
 
#define MAX_SOURCE_SIZE (0x100000)
 
float randr(int min, int max)
{
    double scaled = (double)rand()/RAND_MAX;
    return (max - min +1)*scaled + min;
}

void desabilita_cache_compilacao(){
    setenv("CUDA_CACHE_DISABLE", "1", 1);
}


 
int main(void) {

    desabilita_cache_compilacao();

    // Create the input vector
    int i;
    const int LIST_SIZE = 100000;
    float *A = (float*)malloc(sizeof(float)*LIST_SIZE);
    
	FILE *arq = fopen("p4.txt", "r");
	
	while( (fscanf(arq,"%f\n", &A[i]))!=EOF ){			
		//printf("%f\n", A[i]);
		i++;
	}	
    
    /*for(i = 0; i < LIST_SIZE; i++) {
        A[i] = randr(-5, 5);
    }*/
 
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;
 
    fp = fopen("kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );
 
    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_GPU, 1, 
            &device_id, &ret_num_devices);
 
    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
 
    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
 
    // Create memory buffers on the device for each vector 
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
            LIST_SIZE * sizeof(float), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
            LIST_SIZE * sizeof(float), NULL, &ret);            
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
            LIST_SIZE * sizeof(float), NULL, &ret);
 
    // Copy the lists A memory buffer
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
            LIST_SIZE * sizeof(float), A, 0, NULL, NULL);
 
    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, 
            (const char **)&source_str, (const size_t *)&source_size, &ret);
 
    // Build the program
    ret = clBuildProgram(program, 1, &device_id, "-cl-opt-disable", NULL, NULL);
 
    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);
 
    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
 
    // Execute the OpenCL kernel
    size_t global_item_size = 1;
    size_t local_item_size  = 1;
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);
 
    float *B = (float*)malloc(sizeof(float)*LIST_SIZE);
    float *C = (float*)malloc(sizeof(float)*LIST_SIZE);
    
    ret = clEnqueueReadBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(float), B, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(float), C, 0, NULL, NULL); 
    
    float totalDiff = 0;

    for(i = 0; i < LIST_SIZE; i++){
        printf("%f\t%.15f\n", A[i], B[i]);
       //printf("%f\t%f\t%f\n", A[i], B[i], C[i]);
        
       // totalDiff += fabs(B[i]-C[i]);
    }
    
    //printf("Diferença total: %f\n", totalDiff);
 
    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    
    free(A);
    free(B);
    free(C);
    return 0;
}
