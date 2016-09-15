#ifndef ZED_MATRIX_ALLOCATOR_BASE_H
#define ZED_MATRIX_ALLOCATOR_BASE_H

class ZedMatrixAllocatorBase
{
public:
	ZedMatrixAllocatorBase() {}
	virtual ~ZedMatrixAllocatorBase() {}

	virtual void* allocate(unsigned int rows, unsigned int cols) = 0;
	virtual void free(void* mat) = 0;

};


#endif