#include "zedMatAllocator_Eigen.h"
#include <Eigen/Dense>
#include "defines.h"

zedMatAllocator_Eigen::zedMatAllocator_Eigen(void)
{
}


zedMatAllocator_Eigen::~zedMatAllocator_Eigen(void)
{
}

void* zedMatAllocator_Eigen::allocate(unsigned int rows, unsigned int cols)
{

	Eigen::Matrix<PREC_TYPE, Eigen::Dynamic, Eigen::Dynamic>* mat = NULL;

	mat = new Eigen::Matrix<PREC_TYPE, Eigen::Dynamic, Eigen::Dynamic>(rows,cols);

	if (!mat)
	{
		printf("Error! zedMatAllocator_Eigen::allocate allocation failed - [%d x %d elements of size %d bytes]. Perhaps out of mem ?\n", 
			rows,cols,sizeof(PREC_TYPE));
		assert(0);
		return NULL;
	}

	VERBOSE_ALLOC_PRINT("Eigen: Allocated [%p]\n",mat);

	return mat;
}

void zedMatAllocator_Eigen::free(void* mat)
{
	assert(mat);
	VERBOSE_ALLOC_PRINT("Eigen: freeing mat [%p]\n",mat);
	delete mat;			
}