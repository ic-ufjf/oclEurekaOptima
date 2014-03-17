//This program implements a vector addition using OpenCL

//System includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//OpenCL includes
#include <CL/cl.h>

//OpenCL kernel
const char * programSource = 

"__kernel void vecadd(__global int *A,   \n"
"					 __global int  *B,   \n"
"					 __global int  *C){  \n"
"	//Get work-item id				     \n"
"	int gid = get_global_id(0); 	     \n"
"	C[gid] = A[gid] + B[gid]; 		     \n"	
"} 										 \n";

int main(){
	
	//OpenCL host code

	//Host data
	int *  A = NULL, //input 
		*  B = NULL, //input
		*  C = NULL; //output
	
	const int elements = 8;
	
	//Data size in bytes
	size_t datasize = sizeof(int)*elements;
	
	
	//Allocate space for input/output elements
	A = (int*)malloc(datasize);
	B = (int*)malloc(datasize);
	C = (int*)malloc(datasize);
	
	
	//Initialize input data
	int i;
	for(i=0;i<elements;i++){
		A[i] = i;
		B[i] = i;
		C[i] = 2;
	}
	
	cl_int status;
		
	//---------------------------------------------
	// STEP1: Discover and initialize the platforms
	//---------------------------------------------
	
	cl_uint numPlatforms = 0;
	cl_platform_id * platforms = NULL;
	
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	
	//Allocate space for each platform
	platforms = (cl_platform_id*) malloc(numPlatforms*sizeof(cl_platform_id));
	
	//Fill platforms with clGetPlatformsID()
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	
	//---------------------------------------------
	// STEP2: Discover and initialize the devices
	//---------------------------------------------
	
	cl_uint numDevices = 0;
	cl_device_id * devices = NULL;
	
	//Retrieve the number of devices in the 1st platform
	status = clGetDeviceIDs(platforms[0],
							CL_DEVICE_TYPE_ALL,
							0,
							NULL,
							&numDevices);
	
	
	//Allocate space for each device
	devices = (cl_device_id*) malloc(numDevices*sizeof(cl_device_id));
	
	//Fill devices with
	GetDevicesID()
	status = clGetDeviceIDs(platforms[0], 
							CL_DEVICE_TYPE_ALL, 
							numDevices, 
							devices,
							NULL);

	//---------------------------------------------
	// STEP3: Create a context
	//---------------------------------------------
	
	cl_context context = NULL;
	
	//Create a context using clCreateContext() and associate it with the devices
	context= clCreateContext(NULL,
							 numDevices,
						 	 devices,
						 	 NULL,
						 	 NULL,
						 	 &status);
	
	//-----------------------
	----------------------
	// STEP4: Create a command queue
	//---------------------------------------------
	
	cl_command_queue cmdQueue;
	
	//Create a command queue using clCreateCommandQueue(),
	//and associate it with the devices we want to execute on
	
	cmdQueue = clCreateCommandQueue(context,
									devices[0],
									0,
									&status);
	 
	
	//---------------------------------------------
	// STEP5: Create the device buffers
	//---------------------------------------------
	
	cl_mem bufferA;
	cl_mem bufferB;
	cl_mem bufferC;
	
	bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY , datasize, NULL, &status);
	bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY , datasize, NULL, &status);	
	bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize, NULL, &status);	
		
	
	//---------------------------------------------
	// STEP6: Write host data to device buffers
	//---------------------------------------------
		
	//Write input array A to device buffer bufferA
	status = clEnqueueWriteBuffer(cmdQueue,
								  bufferA, 
								  CL_TRUE, 
								  0,
								  datasize,
								  A,
								  0,
								  NULL,
								  NULL);
							 
	status = clEnqueueWriteBuffer(cmdQueue,
								  bufferB, 
								  CL_TRUE, 
								  0,
								  datasize,
								  B,
								  0,
								  NULL,
								  NULL);	
	

	//---------------------------------------------
	// STEP7: Create and compile the program
	//---------------------------------------------
	
	size_t programSize = (size_t)strlen(programSource);
	
	//Create a program
	cl_program program = clCreateProgramWithSource(context, 
												   1,
												   (const char **)&programSource,
												   &programSize,
												   &status);
	//Build the program for the devices
	status = clBuildProgram(program,
							  numDevices,
							  devices,
							  NULL,
							  NULL,
							  NULL);
	
	
	//---------------------------------------------
	// STEP 8: Create the kernel
	//---------------------------------------------
		
	cl_kernel kernel = NULL;
	
	//Create a kernel from the vector addition function (vecadd)
	kernel = clCreateKernel(program, "vecadd", &status);
	
	
	//---------------------------------------------
	// STEP 9: Set the kernel arguments
	//---------------------------------------------
	
	//Associate the input and output buffers with the kernel
	status = clSetKernelArg(kernel,
							0,
							sizeof(bufferA),
							&bufferA);
	
	status = clSetKernelArg(kernel,
							1,
							sizeof(bufferB),
							&bufferB);
	
	status = clSetKernelArg(kernel,
							2,
							sizeof(bufferC),
							&bufferC);
	
	//---------------------------------------------
	// STEP 10: Configure the work-item structure
	//---------------------------------------------
		
	//Global work 1D, with 'elements' work-items
	size_t globalWorkSize[1];
	globalWorkSize[0] = elements;
	
	
	//---------------------------------------------
	// STEP 11: Enqueue the kernel for execution
	//---------------------------------------------
			
	status = clEnqueueNDRangeKernel(cmdQueue,
									kernel,
									1,
									NULL,
									globalWorkSize,
									NULL,
									0,
									NULL,
									NULL);
	
	
	
	//-------------------------------------------------
	// STEP 12: Read the output buffer back to the host
	//-------------------------------------------------
	
	//Use clEnqueueReadBuffer() to read the OpenCL output bufferC to host array C
	
	clEnqueueReadBuffer(cmdQueue,
						bufferC,
						CL_TRUE,
						0,
						datasize,
						C,
						0,
						NULL,
						NULL);
		
	clFinish(cmdQueue);
	
	
	int result = 1;
	for(i=0;i<elements;i++){
		if(C[i]!= i+i){
			result = 0;
			break;
		}		
	}
	
	if(result){
		printf("Output is correct \n");
	}
	else{
		printf("Output is incorrect \n");
	}
	
	
	//---------------------------------------------
	// STEP 13: Release OpenCL resources
	//
	
	//free openCL resources
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(bufferA);
	clReleaseMemObject(bufferB);
	clReleaseMemObject(bufferC);
	
	//free host resources
	free(A);
	free(B);
	free(C);
	free(platforms);
	free(devices);	
	
	return 0;
}