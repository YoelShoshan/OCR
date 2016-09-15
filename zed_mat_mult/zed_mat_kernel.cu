#include "zed_mat_kernel.h"

// simple cuda matrix mul

#include "defines.h"
#include "stdio.h"

#define CHECK_BANK_CONFLICTS 0
#if CHECK_BANK_CONFLICTS
#define AS(i, j) cutilBankChecker(((double*)&As[0][0]), (block_size * i + j))
#define BS(i, j) cutilBankChecker(((double*)&Bs[0][0]), (block_size * i + j))
#else
#define AS(i, j) As[i][j]
#define BS(i, j) Bs[i][j]
#endif



void kerMatrixAdd_FromC(PREC_TYPE* d_c, const PREC_TYPE* d_a, const PREC_TYPE* d_b,int rows, int cols)
{
	dim3 threads(BLOCK_SIZE, BLOCK_SIZE);

	dim3 grid( 
		(rows / threads.x)+(rows%threads.x==0 ? 0 : 1),
		(cols / threads.y)+(cols%threads.y==0 ? 0 : 1));

	kerMatrixAdd<<< grid, threads >>>(d_c, d_a, d_b,rows,cols);	
}



void kerMatrixMul_FromC(PREC_TYPE* d_c, const PREC_TYPE* d_a, const PREC_TYPE* d_b,int a_rows, int a_cols,int b_rows, int b_cols)
{
	//dim3 grid((n+31)/32, (n+31)/32);

	// execute the kernel
	dim3 threads(BLOCK_SIZE, BLOCK_SIZE);

	//rows/BLOCK_THREADS_X+ (rows%BLOCK_THREADS_X == 0 ? 0:1

	dim3 grid( 
		(b_rows / threads.x)+(b_rows%threads.x==0 ? 0 : 1),
		(a_cols / threads.y)+(a_cols%threads.y==0 ? 0 : 1));

	//printf("block=%d x %d\n",threads.x,threads.y);
	//printf("grid=%d x %d\n",grid.x,grid.y);


	kerMatrixMul<<< grid, threads >>>(d_c, d_a, d_b,a_rows, a_cols,b_rows, b_cols);	

	//kerMatrixMul<<< grid, threads >>>(d_c, d_a, d_b, a_rows, b_rows);	
}

void kerMatrixMul_With_Scalar_FromC(PREC_TYPE* res, PREC_TYPE* src, PREC_TYPE s, int rows, int cols)
{
	dim3 threads(BLOCK_SIZE, BLOCK_SIZE);
	dim3 grid( 
		(rows / threads.x)+(rows%threads.x==0 ? 0 : 1),
		(cols / threads.y)+(cols%threads.y==0 ? 0 : 1));

	kerMatrixMulWithScalar<<< grid, threads >>>(res, src, s, rows, cols);	
}

void kerMatrixTranspose_FromC(PREC_TYPE* dst, PREC_TYPE* src, int src_rows, int src_cols)
{
	dim3 threads(BLOCK_SIZE, BLOCK_SIZE);
	dim3 grid( 
		(src_rows / threads.x)+(src_rows%threads.x==0 ? 0 : 1),
		(src_cols / threads.y)+(src_cols%threads.y==0 ? 0 : 1));

	kerMatrixTranspose<<< grid, threads >>>(dst,src,src_rows,src_cols);	
}


__global__ void kerMatrixAdd( PREC_TYPE* C, const PREC_TYPE* A, const PREC_TYPE* B,int rows, int cols)
{ 
	//current row
	int x = threadIdx.x + blockIdx.x * blockDim.x; 
	//current column
	int y = threadIdx.y + blockIdx.y * blockDim.y; 

	//debug
	//printf("x=%d,y=%d\n",x,y);
	//C[y*b_rows + x] = 10.0;
	//return;

	if (x > rows-1)
	{
		return;
	}

	if (y > cols-1)
	{
		return;
	}

	C[y*cols + x] = A[y*cols + x] + B[y*cols + x];
}


//simplest - not optimizedw
//TODO: add optimization by using sub-matrices optimization + shared memory.
__global__ void kerMatrixMul( PREC_TYPE* C, const PREC_TYPE* A, const PREC_TYPE* B,int a_rows, int a_cols,int b_rows, int b_cols)
{ 
	//current row
	int x = threadIdx.x + blockIdx.x * blockDim.x; 
	//current column
	int y = threadIdx.y + blockIdx.y * blockDim.y; 

	//debug
	//printf("x=%d,y=%d\n",x,y);
	//C[y*b_rows + x] = 10.0;
	//return;

	int c_rows = a_rows;
	int c_cols = b_cols;

	if (y > c_cols-1)
	{
		return;
	}

	if (x > c_rows-1)
	{
		return;
	}

	PREC_TYPE sum_val = 0;
	// each thread computes one element of the output matrix Pd.      
	for (int k = 0; k < a_cols; ++k) 
	{
		int a_ind = x*a_cols + k;
		int b_ind = k*b_cols + y;
		sum_val += A[a_ind] * B[b_ind];
	}

	// write back to the global memory
	C[x*c_cols + y] = sum_val;	
}

   /*/// 1. 2D Thread ID
   int tx = blockIdx.x * TILE_SIZE + threadIdx.x;
   int ty = blockIdx.y * TILE_SIZE + threadIdx.y;
 
   // value stores the element that is 
   // computed by the thread
   PREC_TYPE value = 0;
   for (int i = 0; i < wA; ++i)
   {
      PREC_TYPE elementA = A[ty * wA + i];
      PREC_TYPE elementB = B[i * wB + tx];
      value += elementA * elementB;
   }
 
   // Write the matrix to device memory each 
   // thread writes one element
   C[ty * wA + tx] = value;*/

/*__global__ void kerMatrixMul( PREC_TYPE* C, const PREC_TYPE* A, const PREC_TYPE* B, int wA, int wB)
{


	// Block index
	int bx = blockIdx.x;
	int by = blockIdx.y;

	// Thread index
	int tx = threadIdx.x;
	int ty = threadIdx.y;


	//check out of bounds
	if (bx == gridDim.x-1)
	{
		if (tx > 0)
		{
			return;
		}
	}

	// Index of the first sub-matrix of A processed by the block
	int aBegin = wA * BLOCK_SIZE * by;

	// Index of the last sub-matrix of A processed by the block
	int aEnd   = aBegin + wA - 1;

	// Step size used to iterate through the sub-matrices of A
	int aStep  = BLOCK_SIZE;

	// Index of the first sub-matrix of B processed by the block
	int bBegin = BLOCK_SIZE * bx;

	// Step size used to iterate through the sub-matrices of B
	int bStep  = BLOCK_SIZE * wB;

	// Csub is used to store the element of the block sub-matrix
	// that is computed by the thread
	PREC_TYPE Csub = 0;

	// Loop over all the sub-matrices of A and B
	// required to compute the block sub-matrix
	for (int a = aBegin, b = bBegin;
			a <= aEnd;
			a += aStep, b += bStep)
	{

		// Declaration of the shared memory array As used to
		// store the sub-matrix of A
		__shared__ PREC_TYPE As[BLOCK_SIZE][BLOCK_SIZE];

		// Declaration of the shared memory array Bs used to
		// store the sub-matrix of B
		__shared__ PREC_TYPE Bs[BLOCK_SIZE][BLOCK_SIZE];

		// Load the matrices from device memory
		// to shared memory; each thread loads
		// one element of each matrix
		AS(ty, tx) = A[a + wA * ty + tx];
		BS(ty, tx) = B[b + wB * ty + tx];

		// Synchronize to make sure the matrices are loaded
		__syncthreads();

		// Multiply the two matrices together;
		// each thread computes one element
		// of the block sub-matrix
	#pragma unroll

		for (int k = 0; k < BLOCK_SIZE; ++k)
			Csub += AS(ty, k) * BS(k, tx);

		// Synchronize to make sure that the preceding
		// computation is done before loading two new
		// sub-matrices of A and B in the next iteration
		__syncthreads();
	}

	// Write the block sub-matrix to device memory;
	// each thread writes one element
	int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;

	int ind = c + wB * ty + tx;

	C[ind] = Csub;
}
*/


__global__ void kerMatrixMulWithScalar( PREC_TYPE* res, PREC_TYPE* src, PREC_TYPE s, int rows, int cols)
{
	//current row
	int x = threadIdx.x + blockIdx.x * blockDim.x; 
	//current column
	int y = threadIdx.y + blockIdx.y * blockDim.y; 

	if (x > rows-1)
	{
		return;
	}

	if (y > cols-1)
	{
		return;
	}

	res[y*rows + x] = src[y*rows + x]*s;
}

__global__ void kerMatrixTranspose(PREC_TYPE* dst, PREC_TYPE* src, int src_rows, int src_cols)
{
	//current row
	int x = threadIdx.x + blockIdx.x * blockDim.x; 
	//current column
	int y = threadIdx.y + blockIdx.y * blockDim.y; 

	if (x > src_rows-1)
	{
		return;
	}

	if (y > src_cols-1)
	{
		return;
	}

	dst[y*src_rows + x] = src[x*src_cols + y];
}