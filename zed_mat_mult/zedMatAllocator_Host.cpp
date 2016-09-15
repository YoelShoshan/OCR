#include "zedMatAllocator_Host.h"
#include <stdio.h>
#include <assert.h>
#include "defines.h"

zedMatAllocator_Host::zedMatAllocator_Host(void)
{
}


zedMatAllocator_Host::~zedMatAllocator_Host(void)
{
}

void* zedMatAllocator_Host::allocate(unsigned int rows, unsigned int cols)
{
	PREC_TYPE* mat = NULL;
	mat = new PREC_TYPE[rows*cols];

	if (!mat)
	{
		printf("Error! zedMatAllocator_Host::allocate allocation failed - alloc size=[%d] bytes. Perhaps out of mem ?\n", rows*cols*sizeof(PREC_TYPE));
		assert(0);
		return NULL;
	}

	VERBOSE_ALLOC_PRINT("Host: Allocated [%p]\n",mat);

	return mat;
}

void zedMatAllocator_Host::free(void* mat)
{
	assert(mat);
	VERBOSE_ALLOC_PRINT("Host: freeing mat [%p]\n",mat);
	delete [] mat;			
}
