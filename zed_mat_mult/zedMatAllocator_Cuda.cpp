#include "zedMatAllocator_Cuda.h"
#include "cuda_runtime.h"
#include "zed_mat_kernel.h"
#include <stdio.h>
#include <assert.h>


zedMatAllocator_Cuda::zedMatAllocator_Cuda(void)
{

}


zedMatAllocator_Cuda::~zedMatAllocator_Cuda(void)
{
}

void* zedMatAllocator_Cuda::allocate(unsigned int rows, unsigned int cols)
{
	PREC_TYPE* mat = NULL;

	cudaError_t cudaStatus;
	size_t size = rows*cols*sizeof(PREC_TYPE);
	cudaStatus = cudaMalloc((void**)&mat, size);

	if (cudaSuccess != cudaStatus)
	{
		printf("Error! cudaMalloc failed!\n");
		assert(0);
		return NULL;
	} 

	VERBOSE_ALLOC_PRINT("Cuda: Allocated [%p]\n",mat);

	return mat;
}

void zedMatAllocator_Cuda::free(void* mat)
{
	cudaError_t cudaStatus = cudaGetLastError();
	assert(mat);
	VERBOSE_ALLOC_PRINT("Cuda: freeing mat [%p]\n");
	cudaStatus = cudaFree(mat);

	if (cudaSuccess != cudaStatus)
	{
		printf("Error! cudaFree failed on [%p]!\n",mat);
		//assert(0);
	}
}
