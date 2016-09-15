#ifndef ZED_MATRIX_CUDA_H
#define ZED_MATRIX_CUDA_H

#include "zedMatrixBase.h"
//#include "RefPointer.h"
#include "defines.h"
#include "RefPointer.h"


class ZedMatrix_Cpu;
class ZedMatrix_Cuda : public ZedMatrixBase
{
public:
	//TODO: I should consider adding creation access flags to not waste host mem when no locks are intended.
	ZedMatrix_Cuda(unsigned int rows, unsigned int cols/*, unsigned int access_flags*/); 
	virtual ~ZedMatrix_Cuda(void);

	virtual bool operator== (ZedMatrix_Cpu& other);

	virtual ZedMatrix_Cuda operator+ (const ZedMatrix_Cuda& other);
	virtual ZedMatrix_Cuda operator* (const ZedMatrix_Cuda& other);
	virtual ZedMatrix_Cuda operator* (PREC_TYPE scalar);
	virtual void operator*= (PREC_TYPE scalar);
	virtual void operator*= (const ZedMatrix_Cuda& other);
	virtual ZedMatrix_Cuda Transpose();

	virtual void Rand();
	virtual void Print();

	//TODO:
	//virtual void Copy()
		
	void operator= (const ZedMatrix_Cuda& other);

	//TODO: optimization - only create host memory when lock is requested.
	//access mem 
	virtual void* Lock(unsigned int flags);
	virtual void Unlock();

private:

	void print_dbg_data();

	ZedMatrix_Cuda(const ZedMatrix_Cuda& other);

	//void* m_dev_mat;

	RefPointer* m_dev_mat;
	RefPointer* m_host_mat;

	//void* m_dev_mat;
	//void* m_host_mat;

	unsigned int m_lock_flags;

	//unsigned int m_creation_access_flags;
};


#endif

