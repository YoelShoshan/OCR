#include "ZedMatrix_Cuda.h"
#include "ZedMatPool.h"
#include <assert.h>
#include "zed_mat_kernel.h"
#include <stdio.h>
#include <math.h>
#include <Windows.h>
#include "zedMatrix_Cpu.h"
#include "RefPointer.h"


extern ZedMatrixPool* g_pMatrixPool_Cuda_Dev;
extern ZedMatrixPool* g_pMatrixPool_Cuda_Host;

ZedMatrix_Cuda::ZedMatrix_Cuda(unsigned int rows, unsigned int cols) : ZedMatrixBase(rows,cols)
{
	VERBOSE_DBG_PRINT("ZedMatrix_Cuda Constructor %p\n",this);	
	assert(g_pMatrixPool_Cuda_Dev);

	m_dev_mat = new RefPointer(g_pMatrixPool_Cuda_Dev->getMat(rows,cols),g_pMatrixPool_Cuda_Dev);
	m_host_mat = new RefPointer(g_pMatrixPool_Cuda_Host->getMat(rows,cols),g_pMatrixPool_Cuda_Host);

	m_lock_flags = 0;

	assert(m_dev_mat);
	assert(m_dev_mat->m_data);
	assert(m_host_mat);
	assert(m_host_mat->m_data);

	print_dbg_data();
}

//copy constructor is not allowed from outside this class.
ZedMatrix_Cuda::ZedMatrix_Cuda(const ZedMatrix_Cuda& other) : ZedMatrixBase(other)
{
	VERBOSE_DBG_PRINT("Copy Constructor ZedMatrix_Cuda %p\n",this);	

	m_lock_flags = 0;

	assert(g_pMatrixPool_Cuda_Dev);
	assert(g_pMatrixPool_Cuda_Host);

	/*
	//this version allocates new matrices and performs a cuda memcpy to it
	// in this version "a = b" will copy b data into a new a
	m_dev_mat = g_pMatrixPool_Cuda_Dev->getMat(other.GetWidth(),other.GetHeight());
	m_host_mat = g_pMatrixPool_Cuda_Host->getMat(other.GetWidth(),other.GetHeight());

	assert(other.GetWidth() == m_width);
	assert(other.GetHeight() == m_height);

	size_t size = m_width*m_height*sizeof(PREC_TYPE);

	cudaError_t cudaStatus;

	void *src = other.m_dev_mat;
	void *dst = m_dev_mat;

	VERBOSE_DBG_PRINT("Copying %d bytes from [%p] to [%p] (device matrices)\n", size, src, dst);
	cudaStatus = cudaMemcpy(dst, src, size, cudaMemcpyDeviceToDevice);

	if (cudaSuccess != cudaStatus)
	{
		printf("Error! cudaMemcpy cudaMemcpyDeviceToDevice(%p,%p,%d,...) failed!\n",
			dst, src, size
			);
		assert(0);
	} 
	//note: not copying the host side data.
	*/

	// in this version 


	m_dev_mat = other.m_dev_mat;
	m_dev_mat->Inc();

	m_host_mat = other.m_host_mat;
	m_host_mat->Inc();

	print_dbg_data();
}


void ZedMatrix_Cuda::operator= (const ZedMatrix_Cuda& other)
{	
	assert(m_dev_mat);
	assert(m_host_mat);
	VERBOSE_DBG_PRINT("operator= ZedMatrix_Cuda %p\n",this);	

	//assert(other.GetWidth() == m_width);
	//assert(other.GetHeight() == m_height);

	m_rows = other.GetRows();
	m_cols = other.GetCols();

	/*size_t size = m_width*m_height*sizeof(PREC_TYPE);

	cudaError_t cudaStatus;

	void *src = other.m_dev_mat;
	void *dst = m_dev_mat;

	VERBOSE_DBG_PRINT("Copying %d bytes from [%p] to [%p] (device matrices)\n", size, src, dst);
	cudaStatus = cudaMemcpy(dst, src, size, cudaMemcpyDeviceToDevice);

	if (cudaSuccess != cudaStatus)
	{
		printf("Error! cudaMemcpy cudaMemcpyDeviceToDevice(%p,%p,%d,...) failed!\n",
			dst, src, size
			);
		assert(0);
	} 
	*/

	//////////////////////////////////////////////////////////////////////
	// this version just points to the other's matrix.
	// the problem is that when someone writes a=b,
	// the expected behaviour is a copy, not another reference.
	// however, this is faster. So if this is well defined in the syntax,
	// maybe it's ok.
	//////////////////////////////////////////////////////////////////////

	//increase the count of what we are about to receieve first.
	
	other.m_dev_mat->Inc();
	other.m_host_mat->Inc();

	RefPointer* prev_dev_mat = m_dev_mat;
	RefPointer* prev_host_mat = m_host_mat;

	m_dev_mat = other.m_dev_mat;
	m_host_mat = other.m_host_mat;

	//decrease the count of our own now

	prev_dev_mat->Dec();
	prev_host_mat->Dec();

	// copy data from other to this.


	print_dbg_data();

	m_lock_flags = 0;
}


ZedMatrix_Cuda::~ZedMatrix_Cuda(void)
{
	VERBOSE_DBG_PRINT("ZedMatrix_Cuda Destructor %p\n",this);
	print_dbg_data();
	assert(g_pMatrixPool_Cuda_Dev);
	assert(g_pMatrixPool_Cuda_Host);
	//printf("Freeing mat dev %p\n",m_dev_mat);
	//g_pMatrixPool_Cuda_Dev->freeMat(m_dev_mat);
	//g_pMatrixPool_Cuda_Host->freeMat(m_host_mat);

	m_dev_mat->Dec();
	m_host_mat->Dec();

	m_dev_mat = 0;
	m_host_mat = 0;
}


void ZedMatrix_Cuda::print_dbg_data()
{
	//printf("\tDev:%p, Host:%p\n",m_dev_mat,m_host_mat);
	VERBOSE_DBG_PRINT("Cuda_Dev: ");
	m_dev_mat->dbg_print();
	VERBOSE_DBG_PRINT("Cuda_Host: ");
	m_host_mat->dbg_print();
}


bool ZedMatrix_Cuda::operator== (ZedMatrix_Cpu& other)
{
	return (other == (*this));
}

ZedMatrix_Cuda ZedMatrix_Cuda::operator+ (const ZedMatrix_Cuda& other)
{
	assert(m_rows == other.m_rows);
	assert(m_cols == other.m_cols);
	
	ZedMatrix_Cuda res(m_rows,m_cols);

	assert(sizeof(PREC_TYPE)==sizeof(double));

	kerMatrixAdd_FromC(
		(double*)res.m_dev_mat->m_data,
		(double*)m_dev_mat->m_data,
		(double*)other.m_dev_mat->m_data,
		m_rows,m_cols);

	return res;

}

ZedMatrix_Cuda ZedMatrix_Cuda::operator* (const ZedMatrix_Cuda& other)
{
	if (m_lock_flags != 0)
	{
		printf("Error: ZedMatrix_Cuda::operator* - cannot perform algebric operations while locked for mem access. Please Unlock first.\n");
		assert(0);
		return ZedMatrix_Cuda(1,1);
	}

	assert(m_cols == other.m_rows);

	ZedMatrix_Cuda res(m_rows,other.m_cols);

	//void kerMatrixMul_FromC(PREC_TYPE* d_c, const PREC_TYPE* d_a, const PREC_TYPE* d_b, int a_width, int a_height,int b_width, int b_height);
	
	assert(sizeof(PREC_TYPE)==sizeof(double));

	kerMatrixMul_FromC(		
		(double*)res.m_dev_mat->m_data,
		(double*)m_dev_mat->m_data,		
		(double*)other.m_dev_mat->m_data,				
		m_rows,m_cols,
		other.m_rows, other.m_cols
		);

	return res;
}

ZedMatrix_Cuda ZedMatrix_Cuda::operator* (PREC_TYPE scalar)
{
	if (m_lock_flags != 0)
	{
		printf("Error: ZedMatrix_Cuda::operator* - cannot perform algebric operations while locked for mem access. Please Unlock first.\n");
		assert(0);
		return ZedMatrix_Cuda(1,1);
	}

	ZedMatrix_Cuda res(m_rows,m_cols);

	//void kerMatrixMul_FromC(PREC_TYPE* d_c, const PREC_TYPE* d_a, const PREC_TYPE* d_b, int a_width, int a_height,int b_width, int b_height);
	
	assert(sizeof(PREC_TYPE)==sizeof(double));

	kerMatrixMul_With_Scalar_FromC(
		(double*)res.m_dev_mat->m_data,
		(double*)m_dev_mat->m_data,
		scalar,
		m_rows,m_cols);

	return res;
}

void ZedMatrix_Cuda::operator*= (PREC_TYPE scalar)
{
	if (m_lock_flags != 0)
	{
		printf("Error: ZedMatrix_Cuda::operator*(PREC_TYPE scalar) - cannot perform algebric operations while locked for mem access. Please Unlock first.\n");
		assert(0);
	}

	//since we can't read+write from the same matrix, since it will return bad results (imagine a part being written to while other threads are reading from it)
	//i have to copy to a tmp matrix first.
	
	assert(sizeof(PREC_TYPE)==sizeof(double));	
	
	void* tmp_mat = g_pMatrixPool_Cuda_Dev->getMat(m_rows,m_cols);
	assert(tmp_mat);

	cudaError_t cudaStatus;
	void *dst = tmp_mat;
	void *src = m_dev_mat->m_data;
	size_t size = m_rows*m_cols*sizeof(PREC_TYPE);
	cudaStatus = cudaMemcpy(dst, src, size, cudaMemcpyDeviceToDevice);
	if (cudaSuccess != cudaStatus)
	{
		printf("Error! cudaMemcpy cudaMemcpyDeviceToDevice(%p,%p,%d,...) failed!\n",
			dst, src, size
			);
		assert(0);		
	} 


	kerMatrixMul_With_Scalar_FromC(
		(double*)m_dev_mat->m_data,
		(double*)tmp_mat,
		scalar,
		m_rows,m_cols);

	g_pMatrixPool_Cuda_Dev->freeMat(tmp_mat);
}

void ZedMatrix_Cuda::operator*= (const ZedMatrix_Cuda& other)
{
	if (m_lock_flags != 0)
	{
		printf("Error: ZedMatrix_Cuda::operator*=(const ZedMatrix_Cuda& other) - cannot perform algebric operations while locked for mem access. Please Unlock first.\n");
		assert(0);
	}

		
	assert(m_cols == other.m_rows);
			
	//since we can't read+write from the same matrix, since it will return bad results (imagine a part being written to while other threads are reading from it)
	//i have to copy to a tmp matrix first.

	assert(sizeof(PREC_TYPE)==sizeof(double));	
			
	void* tmp_mat = g_pMatrixPool_Cuda_Dev->getMat(m_rows,m_cols);
	assert(tmp_mat);
		
	cudaError_t cudaStatus;
	void *dst = tmp_mat;
	void *src = m_dev_mat->m_data;
	size_t size = m_rows*m_cols*sizeof(PREC_TYPE);
	cudaStatus = cudaMemcpy(dst, src, size, cudaMemcpyDeviceToDevice);
	if (cudaSuccess != cudaStatus)
	{
		printf("Error! cudaMemcpy cudaMemcpyDeviceToDevice(%p,%p,%d,...) failed!\n",
			dst, src, size
			);
		assert(0);		
	} 

	kerMatrixMul_FromC(		
		(double*)m_dev_mat->m_data,
		(double*)tmp_mat,		
		(double*)other.m_dev_mat->m_data,				
		m_rows,m_cols,
		other.m_rows, other.m_cols
		);

	g_pMatrixPool_Cuda_Dev->freeMat(tmp_mat);

}

ZedMatrix_Cuda ZedMatrix_Cuda::Transpose()
{
	if (m_lock_flags != 0)
	{
		printf("Error: ZedMatrix_Cuda::operator* - cannot perform algebric operations while locked for mem access. Please Unlock first.\n");
		assert(0);
		return ZedMatrix_Cuda(1,1);
	}
	
	ZedMatrix_Cuda res(m_cols,m_rows);

	assert(sizeof(PREC_TYPE)==sizeof(double));

	kerMatrixTranspose_FromC(
		(double*)res.m_dev_mat->m_data,
		(double*)m_dev_mat->m_data,
		m_rows,m_cols);

	return res;
}

void ZedMatrix_Cuda::Rand()
{
	//update the matrix with random values

	PREC_TYPE* data = (PREC_TYPE*) Lock(ZED_LOCK_WRITE);

	for (int i=0;i<m_rows*m_cols;i++)
	{
		data[i] = rand() / (PREC_TYPE)RAND_MAX;
	}

	Unlock();
}

void ZedMatrix_Cuda::Print()
{
	printf("Matrix cuda_gpu [%p]:\n",m_dev_mat->m_data);
	PREC_TYPE* data = (PREC_TYPE*) Lock(ZED_LOCK_READ);

	for (int j=0;j<m_rows;j++)
	{
		for (int i=0;i<m_cols;i++)
		{
			//printf("%.3f ",data[j+(i*m_rows)]);
			
			printf("%.3f ",data[i+(j*m_cols)]);

			if (i>5)
			{
				printf("...");
				break;
			}
		}
		printf("\n");

		if (j>5)
		{
			printf("...");
			break;
		}
	}
	printf("\n");
	Unlock();
}

void* ZedMatrix_Cuda::Lock(unsigned int flags)
{
	if (m_lock_flags != 0)
	{
		printf("Error: ZedMatrix_Cuda::Lock already locked - Unlock first.\n");
		return NULL;
	}

	if (0==flags)
	{
		printf("Error: ZedMatrix_Cuda::Lock with zero lock flags!\n");
		return NULL;
	}

	m_lock_flags = flags;

	if (ZED_LOCK_READ & flags)
	{
		// move data from the device to the host
		cudaError_t cudaStatus;
		void *dst = m_host_mat->m_data;
		void *src = m_dev_mat->m_data;
		size_t size = m_rows*m_cols*sizeof(PREC_TYPE);
		cudaStatus = cudaMemcpy(dst, src, size, cudaMemcpyDeviceToHost);
		if (cudaSuccess != cudaStatus)
		{
			printf("Error! cudaMemcpy cudaMemcpyDeviceToHost(%p,%p,%d,...) failed!\n",
				dst, src, size
				);
			assert(0);
			return NULL;
		} 
		
		
	}

	if (ZED_LOCK_WRITE & flags)
	{
		//nothing to do
	}

	return m_host_mat->m_data;
}

void ZedMatrix_Cuda::Unlock()
{
	if (ZED_LOCK_READ & m_lock_flags)
	{
		//nothing to do
	}

	if (ZED_LOCK_WRITE & m_lock_flags)
	{
		// move data from the host to the device
		cudaError_t cudaStatus;		
		void *dst = m_dev_mat->m_data;
		void *src = m_host_mat->m_data;
		size_t size = m_rows*m_cols*sizeof(PREC_TYPE);
		cudaStatus = cudaMemcpy(dst, src, size, cudaMemcpyHostToDevice);
		if (cudaSuccess != cudaStatus)
		{
			printf("Error! cudaMemcpy cudaMemcpyHostToDevice(%p,%p,%d,...) failed!\n",
				dst, src,size
				);
			assert(0);
			return;
		} 
	}

	m_lock_flags = 0;
}