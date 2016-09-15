#include "stdio.h"
#include <conio.h>
#include <stdlib.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <Windows.h>
#include "defines.h"
#include "math.h"

//#include "zed_mat_mult.h"
//#include "EigenMatMult.h"


#include "zedMatAllocator_Cuda.h"
#include "zedMatAllocator_Host.h"
#include "zedMatAllocator_Eigen.h"
#include "ZedMatPool.h"
#include "zedMatrix_Cuda.h"
#include "zedMatrix_Cpu.h"

ZedMatrixPool* g_pMatrixPool_Cuda_Dev = NULL;
ZedMatrixPool* g_pMatrixPool_Cuda_Host = NULL;
ZedMatrixPool* g_pMatrixPool_Eigen = NULL;

#define A_ROWS 4
#define A_COLS 2
#define B_COLS 3

#define B_ROWS A_COLS
#define C_ROWS A_ROWS
#define C_COLS B_COLS

#define SCALAR_VAL 10.0

#define ITERATION_PRINT_SPACE 5

#define ITERATIONS_NUM 1

void test_new_matrix_class()
{
	//cuda gpu matrices pool
	ZedMatrixAllocatorBase *allocator_cuda_gpu = new zedMatAllocator_Cuda();
	g_pMatrixPool_Cuda_Dev = new ZedMatrixPool(allocator_cuda_gpu);

	//cuda host matrices pool
	ZedMatrixAllocatorBase *allocator_cuda_host = new zedMatAllocator_Host();
	g_pMatrixPool_Cuda_Host = new ZedMatrixPool(allocator_cuda_host);	

	ZedMatrixAllocatorBase *allocator_eigen = new zedMatAllocator_Eigen();
	g_pMatrixPool_Eigen = new ZedMatrixPool(allocator_eigen);	

	int tests_num = ITERATIONS_NUM;

	DWORD start,end;
	double delta_sec_CPU = 0.0;
	double delta_sec_GPU = 0.0;


	printf("**********\n GPU TEST\n**********\n");

	srand(0x12345678);

	ZedMatrix_Cuda a1(A_ROWS,A_COLS);
	a1.Rand();
	a1.Print();

	ZedMatrix_Cuda a2(B_ROWS,B_COLS);
	a2.Rand();
	a2.Print();

	ZedMatrix_Cuda a3(B_ROWS,A_COLS);
	
	///////////////
	// warm up
	printf("a3 = a1*a2;\n");

	a3 = (a1*a2);
	a3.Print();

	//a3 = (a1*a2)*SCALAR_VAL;
	//a3.Print();
	///////////////


	//////////////////
	// Cuda tests

	start = timeGetTime();

	for (int i=0;i<tests_num;i++)
	{
		if (!(i%ITERATION_PRINT_SPACE))
		{
			printf("%d\n",i);
		}

		//a3 = a1*a2;
		a3 = (a1*a2)*SCALAR_VAL;
		//a3.Print();
		a3 *= SCALAR_VAL;
		//a3.Print();
	}
	
	a3.Print();
	a3 = a3.Transpose();
	a3.Print();

	
	end = timeGetTime();

	delta_sec_GPU = (end-start) * 0.001;

	printf("GPU Cuda Test time = %f\n",delta_sec_GPU);

	//debug
	//return;

	///////////////////

	printf("**********\n CPU TEST\n**********\n");

	srand(0x12345678);

	ZedMatrix_Cpu b1(A_ROWS,A_COLS);
	b1.Rand();
	b1.Print();

	ZedMatrix_Cpu b2(B_ROWS,B_COLS);
	b2.Rand();
	b2.Print();

	ZedMatrix_Cpu b3(B_ROWS,A_COLS);

	///////////////
	// warm up
	b3 = (b1*b2)*SCALAR_VAL;
	//b3.Print();
	///////////////


	start = timeGetTime();

	for (int i=0;i<tests_num;i++)
	{
		if (!(i%ITERATION_PRINT_SPACE))
		{
			printf("%d\n",i);
		}
		//b3 = b1*b2;
		b3 = (b1*b2)*SCALAR_VAL;
		b3 *= SCALAR_VAL;
	}
	
	b3.Print();
	b3 = b3.Transpose();
	b3.Print();
	
	end = timeGetTime();

	delta_sec_CPU = (end-start) * 0.001;

	printf("CPU Eigen Test time = %f\n",delta_sec_CPU);

	printf("CPU is x%f times GPU time.\n",delta_sec_CPU / delta_sec_GPU);

	printf("Comparing CPU and GPU results...\n");

	if (a3 == b3)
	{
		printf("Results are identical (within an epsilon)\n");
	} else
	{
		printf("Validation failed! result matrices are different.\n");
	}

}


int main()
{
	cudaError_t cudaStatus;
    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) 
	{
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        return -1;
    }

	if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "test_matMult_cuda failed!");
        return 1;
    }

	test_new_matrix_class();

	printf("Press any key to continue...\n");
	getch();
    return 0;
}