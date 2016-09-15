#pragma once

#include "zedMatAllocatorBase.h"

class zedMatAllocator_Host :
	public ZedMatrixAllocatorBase
{
public:
	zedMatAllocator_Host(void);
	virtual ~zedMatAllocator_Host(void);

	virtual void* allocate(unsigned int rows, unsigned int cols);
	virtual void free(void* mat);

};

