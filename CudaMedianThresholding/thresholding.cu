
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <iostream>
#include "bmp.h"
#include <assert.h>
using namespace std;

#define BLOCK_THREADS_X 16
#define BLOCK_THREADS_Y 16
#define THREADS_PER_BLOCK BLOCK_THREADS_X*BLOCK_THREADS_Y

__global__ void kernel_PassThrough(unsigned char *src, unsigned char *res, int width, int height)
{
	int x = (blockIdx.x*blockDim.x) + threadIdx.x;
	int y = (blockIdx.y*blockDim.y) + threadIdx.y;

	int index = (y*width)+x;

    res[index] = src[index] + 30;
}

__global__ void kernel_EdgeDetection(unsigned char *src, unsigned char *res, int width, int height)
{
	int x = (blockIdx.x*blockDim.x) + threadIdx.x;
	int y = (blockIdx.y*blockDim.y) + threadIdx.y;

	int index = (y*width)+x;
	int left_index = max(0, (y*width)+x-1);
	//int right = min(width-1, (y*width)+x+1);

	int diff = abs(int(src[index]) - int(src[left_index]));

	res[index] = unsigned char (diff);
    //res[index] = src[index] + 30;
}

#define KERNEL_DIM 7

__global__ void kernel_MedianThreshold(unsigned char *src, unsigned char *res, int width, int height)
{
	int x = (blockIdx.x*blockDim.x) + threadIdx.x;
	int y = (blockIdx.y*blockDim.y) + threadIdx.y;

	if (x > width-1 || y > height-1)
	{
		return;
	}

	int indices[KERNEL_DIM][KERNEL_DIM];

	int half_kernel_dim = KERNEL_DIM/2;
	int max_index = width*height-1;

	for(int i=0;i<KERNEL_DIM;i++)
	{
		for(int j=0;j<KERNEL_DIM;j++)
		{
			int _i = i-half_kernel_dim;
			int _j = j-half_kernel_dim;
			indices[i][j] = ( (y+_j)*width ) + (x+_i);
			indices[i][j] = max(0, indices[i][j]);
			indices[i][j] = min(max_index, indices[i][j]);
		}
	}

	int accum = 0;

	/*for(int i=0;i<KERNEL_DIM;i++)
	{
		for(int j=0;j<KERNEL_DIM;j++)
		{
			accum+= src[indices[i][j]];
		}
	}*/

	//perform binary search for the median

	int best_median = -1;
	int best_error = 0xFFFF;

	for (int c=0;c<256;c++)
	{
		int error = 0;

		for(int i=0;i<KERNEL_DIM;i++)
		{
			for(int j=0;j<KERNEL_DIM;j++)
			{
				unsigned char curr =  src[indices[i][j]];
				if (curr > c)
					error++;
				else
					error--;
			}
		}

		error = abs(error);
		if (error < best_error)
		{
			best_median = c;
			best_error = error;
		}
	}


	float fRes = float(accum) / float(KERNEL_DIM*KERNEL_DIM);
	unsigned char res_byte = (unsigned char) fRes;

	//res[indices[half_kernel_dim][half_kernel_dim]] = res_byte;

	if ( src[indices[half_kernel_dim][half_kernel_dim]] > best_median - 5)
	{
		res[indices[half_kernel_dim][half_kernel_dim]] = 255;
	} else
	{
		res[indices[half_kernel_dim][half_kernel_dim]] = 0;
	}
	
	/*int index = (y*width)+x;
	int left_index = max(0, (y*width)+x-1);
	//int right = min(width-1, (y*width)+x+1);

	int diff = abs(int(src[index]) - int(src[left_index]));

	res[index] = unsigned char (diff);*/
    //res[index] = src[index] + 30;
}



void checkForErrors()
{
	cudaError_t err = cudaGetLastError();
	if (cudaSuccess!= err)
	{
		const char * err_str = cudaGetErrorString(err);
		MessageBoxA(0,err_str,"Cuda Error!",0);
	}
}

void cuda_device_init(void)
{
	int ndev;
	cudaGetDeviceCount(&ndev);
	cudaThreadSynchronize();
	printf("---- Cuda Devices Configuration ----\n");
	printf("There are %d GPUs.\n",ndev);
     
	for(int i=0;i<ndev;i++) {
	cudaDeviceProp pdev;
	cudaGetDeviceProperties(&pdev,i);
	cudaThreadSynchronize();
	printf("Name  : %s\n",pdev.name);
	printf("Capability  : %d %d\n",pdev.major,pdev.minor);
	printf("Memory Global: %d Mb\n",(pdev.totalGlobalMem+1024*1024)/1024/1024);
	printf("Memory Const : %d Kb\n",pdev.totalConstMem/1024);
	printf("Memory Shared: %d Kb\n",pdev.sharedMemPerBlock/1024);
	printf("Clock  : %.3f GHz\n",pdev.clockRate/1000000.0);
	printf("Processors  : %d\n",pdev.multiProcessorCount);
	printf("Cores  : %d\n",8*pdev.multiProcessorCount);
	printf("Warp  : %d\n",pdev.warpSize);
	printf("Max Thr/Blk  : %d\n",pdev.maxThreadsPerBlock);
	printf("Max Blk dimention Size : %d %d %d\n",pdev.maxThreadsDim[0],pdev.maxThreadsDim[1],pdev.maxThreadsDim[2]);
	printf("Max Grid dimention Size: %d %d %d\n",pdev.maxGridSize[0],pdev.maxGridSize[1],pdev.maxGridSize[2]);
}
}

int main(int args, char* argv[])
{
	if (args < 3)
	{
		printf("Usage: call with ""input.bmp"" ""output.bmp""\n");
		return 0;
	}



	cuda_device_init();

	unsigned int width=0;
	unsigned int height=0;
	unsigned int comps=0;
	
	//src image on host
	unsigned char* h_src = LoadBMP(argv[1],width,height,comps);
	assert(1==comps);

	//verify
	//SaveBMP_GreyScale(h_src,width,height,"C:/temp/cuda_thresholding/verify_bmp_lib.bmp");

	unsigned int pixels_num = width*height;
	int byte_size = sizeof(char)*pixels_num;
	//src image on device
	unsigned char* d_src = NULL;
	cudaMalloc((void **)&d_src, byte_size);

	//result image on host
	unsigned char* h_res = new unsigned char[width*height];
	unsigned char* d_res = NULL;
	cudaMalloc((void **)&d_res, byte_size);

	//copy src from host to device
	cudaMemcpy(d_src, h_src, byte_size, cudaMemcpyHostToDevice);

	checkForErrors();

	unsigned int blocks_num = pixels_num/THREADS_PER_BLOCK;

	dim3 block_threads(BLOCK_THREADS_X, BLOCK_THREADS_Y);
	//dim3 block_threads(10,10);

	//int n_blocks = N/block_size + (N%block_size == 0 ? 0:1);

	//if it's not a perfect division, we need to add an extra block.
	//part of this thread block will work on pixels outside the image scope, we will make sure in the kernel that we don't write out-of-bounds
	dim3 grid_blocks(width/BLOCK_THREADS_X+ (width%BLOCK_THREADS_X == 0 ? 0:1), height/BLOCK_THREADS_Y + (height%BLOCK_THREADS_Y == 0 ? 0:1));
	//dim3 grid_blocks(10,10);

	printf("------------------------------\n");
	printf("Running (%d,%d,%d) blocks, each contains (%d,%d,%d) Threads.\n",
		block_threads.x,block_threads.y,block_threads.z,
		grid_blocks.x,grid_blocks.y,grid_blocks.z);

	kernel_MedianThreshold<<<grid_blocks,block_threads>>>(d_src,d_res, width, height);
	checkForErrors();

	//copy result from device to host
	cudaMemcpy(h_res, d_res, byte_size, cudaMemcpyDeviceToHost);

	SaveBMP_GreyScale(h_res,width,height,argv[2]);

    return 0;
}
