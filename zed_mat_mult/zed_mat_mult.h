#ifndef ZED_MAT_MULT_H
#define ZED_MAT_MULT_H

#include "cuda_runtime.h"
#include "zed_mat_kernel.h"
#include <assert.h>
#include "defines.h"

//TODO: I need to add support for:
// 1. B = A * s
// 2. B += A
// 3. B += A*s


template <class T,int WA, int HA,int WB, int HB>
class ZedMatMult
{
public:

	T *h_a, *h_b, *h_c;
	size_t mem_size_a, mem_size_b, mem_size_c;
	
	int a_width,a_height;
	int b_width,b_height;
	int c_width,c_height;

	ZedMatMult()
	{
		if ( (WA!=HB) )
		{
			printf("Error! Matrix A height must equal Matrix B width!\n");			
		}

		// allocation
		a_width = WA;
		a_height = HA;

		b_width = WB;
		b_height = HB;

		c_width = WB;
		c_height = HA;

		//allocate host mem

		mem_size_a = sizeof(T) * a_width*a_height;
		h_a = (T*) malloc(mem_size_a);
 
		mem_size_b = sizeof(T) * b_width*b_height;
		h_b = (T*) malloc(mem_size_b);

		mem_size_c = sizeof(T) * c_width*c_height;
		h_c = (T*) malloc(mem_size_c);

		//allocate dev mem

		cudaError_t cudaStatus;

		cudaStatus = cudaMalloc((void**)&d_a, mem_size_a);
		ZMM_CHECK_CUDA_ERROR(cudaStatus, "Allocating device matrix a");
		
		cudaStatus = cudaMalloc((void**)&d_b, mem_size_b);
		ZMM_CHECK_CUDA_ERROR(cudaStatus, "Allocating device matrix b");

		cudaStatus = cudaMalloc((void**)&d_c, mem_size_c);
		ZMM_CHECK_CUDA_ERROR(cudaStatus, "Allocating device matrix c");
	}

	~ZedMatMult()
	{
		free(h_a);
		free(h_b);
		free(h_c);
		cudaFree(d_a);
		cudaFree(d_b);
		cudaFree(d_c);
	}

	bool mult()
	{
		cudaError_t cudaStatus;
		// move data from host to the device
		cudaStatus = cudaMemcpy(d_a, h_a, mem_size_a, cudaMemcpyHostToDevice);
		ZMM_CHECK_CUDA_ERROR_RET(cudaStatus,"Copying data from host to device - mat a");
		cudaStatus = cudaMemcpy(d_b, h_b, mem_size_b, cudaMemcpyHostToDevice);
		ZMM_CHECK_CUDA_ERROR_RET(cudaStatus,"Copying data from host to device - mat b");

		kerMatrixMul_FromC(d_c,d_a,d_b,a_width,a_height,b_width,b_height);
		//kerMatrixMul_FromC(d_c,d_a,d_b,a_width,b_width,c_width);
		//ZMM_CHECK_CUDA_ERROR_RET(cudaStatus,"execute the kernel");
 
		// copy result from device to host
		cudaStatus = cudaMemcpy(h_c, d_c, mem_size_c, cudaMemcpyDeviceToHost);
		ZMM_CHECK_CUDA_ERROR_RET(cudaStatus,"copy result from device to hostb");		

		return true;
	}

private:
	
	T *d_a, *d_b, *d_c;		
};

#endif