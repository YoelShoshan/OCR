
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include "defines.h"
//#include <helper_math.h>
#include "zed_mat_mult.h"
#include "zed_mat_kernel.h"
#include <Windows.h>


//new
//#include <gpumatrix/CORE>
//using namespace gpumatrix;

//cudaError_t addWithCuda(PREC_TYPE *c, const PREC_TYPE *a, const PREC_TYPE *b, size_t size);

/*#define WA 16   // Matrix A width
#define HA 16   // Matrix A height
#define WB 16   // Matrix B width
#define HB WA     // Matrix B height
#define WC WB     // Matrix C width
#define HC HA     // Matrix C height*/

/*

__global__ void kerAdd(PREC_TYPE *c, const PREC_TYPE *a, const PREC_TYPE *b)
{
    int i = threadIdx.x;
    c[i] = a[i] + b[i];
}


// Helper function for using CUDA to add vectors in parallel.
cudaError_t addWithCuda(PREC_TYPE *c, const PREC_TYPE *a, const PREC_TYPE *b, size_t size)
{
    PREC_TYPE *dev_a = 0;
    PREC_TYPE *dev_b = 0;
    PREC_TYPE *dev_c = 0;
    cudaError_t cudaStatus;

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(PREC_TYPE));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(PREC_TYPE));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(PREC_TYPE));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(PREC_TYPE), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(PREC_TYPE), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    kerAdd<<<1, size>>>(dev_c, dev_a, dev_b);

    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(PREC_TYPE), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);
    
    return cudaStatus;
}

void randomInit(PREC_TYPE* data, int size)
{
   for (int i = 0; i < size; ++i)
   {
		data[i] = rand() / (PREC_TYPE)RAND_MAX;
   }
}

void unitInit(PREC_TYPE* data, int width, int height)
{
   for (int i=0;i<width;++i)
   {
	   for (int j=0;j<height;j++)
	   {
		   if (i==j)
		   {
			   data[(width*j)+i] = 1.0;
		   } else
		   {
			   data[(width*j)+i] = 0.0;
		   }
	   }
   }
}

void zeroInit(PREC_TYPE* data, int size)
{
   for (int i = 0; i < size; ++i)
   {
		data[i] = 0.0;
   }
}*/

/*
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

	DWORD start = timeGetTime();

	ZedMatMult<PREC_TYPE,16,16,16,16> zed_mat_mult;

	randomInit(zed_mat_mult.h_a, zed_mat_mult.a_width*zed_mat_mult.a_height);
	unitInit(zed_mat_mult.h_b, zed_mat_mult.b_width,zed_mat_mult.b_height);
	zeroInit(zed_mat_mult.h_c, zed_mat_mult.c_width*zed_mat_mult.c_height);
	
	zed_mat_mult.mult();

    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}
*/



