#pragma once

#include "zedMatAllocatorBase.h"

class zedMatAllocator_Eigen :
	public ZedMatrixAllocatorBase
{
public:
	zedMatAllocator_Eigen(void);
	virtual ~zedMatAllocator_Eigen(void);

	virtual void* allocate(unsigned int rows, unsigned int cols);
	virtual void free(void* mat);
};

