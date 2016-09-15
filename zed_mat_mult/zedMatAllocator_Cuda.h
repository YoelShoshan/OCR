#ifndef ZED_MATRIX_ALLOCATOR_CUDA_H
#define ZED_MATRIX_ALLOCATOR_CUDA_H

#include "zedMatAllocatorBase.h"

class zedMatAllocator_Cuda : public ZedMatrixAllocatorBase
{
public:
	zedMatAllocator_Cuda(void);
	virtual ~zedMatAllocator_Cuda(void);

	virtual void* allocate(unsigned int rows, unsigned int cols);
	virtual void free(void* mat);
};

#endif

