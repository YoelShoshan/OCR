#include "ZedMatPool.h"
#include <assert.h>

using namespace std;
#include "defines.h"


ZedMatrixPool::ZedMatrixPool(ZedMatrixAllocatorBase* mat_allocator)
{
	assert(mat_allocator);
	m_Matrix_Allocator = mat_allocator;
}

ZedMatrixPool::~ZedMatrixPool()
{

}

void* ZedMatrixPool::getMat(unsigned int rows, unsigned int cols)
{
	VERBOSE_ALLOC_PRINT("ZedMatrixPool::getMat - Requesting matrix (%dx%d)...\n",rows,cols);
	pool_map_key k;
	k.rows = rows;
	k.cols = cols;

	//check if there is a free matrix 
	int available_count = m_free_matrices.count(k);

	if (available_count>0)
	{
		//use one of the available matrices for this size
		pair<ZED_POOL_MM_IT, ZED_POOL_MM_IT> ppp;
		// equal_range(b) returns pair<iterator,iterator> representing the range of element with key b
		ppp = m_free_matrices.equal_range(k);

		// it seems that elements with identical keys are consecutive.

		void* available_mat = ppp.first->second;

		m_free_matrices.erase(ppp.first);

		VERBOSE_ALLOC_PRINT("ZedMatrixPool::getMat - found a free one - reusing [%p]\n",available_mat);

		m_active_matrices.insert(pair<pool_map_key,void*>(k,available_mat));
		m_active_matrix_by_data[available_mat] = k;

		return available_mat;
	}

	// if not available, create one.
	void* mat = m_Matrix_Allocator->allocate(rows,cols);
	printf("Allocated matrix [%p] (%dx%d)\n",mat,rows,cols);
	assert(mat);	

	m_active_matrices.insert(pair<pool_map_key,void*>(k,mat));
	m_active_matrix_by_data[mat] = k;
	
	VERBOSE_ALLOC_PRINT("ZedMatrixPool::getMat - had to create a new one [%p]\n",mat);

	return mat;
}

void ZedMatrixPool::freeMat(void* mat)
{
	VERBOSE_ALLOC_PRINT("ZedMatrixPool::freeMat - got a request to free mat [%p] ...\n",mat);
	assert(mat);

	// freeing would move from m_active_matrices to m_free_matrices (not really freeing at the moment anything.)
	ZED_POOL_REV_MAP_IT it = m_active_matrix_by_data.find(mat);
	if (it == m_active_matrix_by_data.end())
	{
		printf("ZedMatrixPool::freeMat - Error! trying to free mat [%p] but it is not found!",mat);
		printf("It happens because (due to performance consideration) operator= creates another reference to the other side matrix data.\n");
		printf("One possible solution is to really copy.\n");
		//assert(0);
		return;
	}


	VERBOSE_ALLOC_PRINT("ZedMatrixPool::freeMat - found it in m_active_matrix_by_data...\n");

	pool_map_key k = it->second;
	
	pair<ZED_POOL_MM_IT, ZED_POOL_MM_IT> ppp;
	// equal_range(b) returns pair<iterator,iterator> representing the range of element with key b
	ppp = m_active_matrices.equal_range(k);

	// now search between all of the matrices in this size, to find the specific matrix pointer.
	// OPTIMIZE: should probably optimize this to a faster method.
	for (ZED_POOL_MM_IT it = ppp.first; it!= ppp.second; ++it)
	{
		if (it->second == mat)
		{

			VERBOSE_ALLOC_PRINT("ZedMatrixPool::freeMat - found it in m_active_matrices!.\n");

			// first add this to the free matrices
			m_free_matrices.insert(pair<pool_map_key,void*>(it->first, it->second));

			// now, delete from the active matrices

			it = m_active_matrices.erase(it);
			m_active_matrix_by_data.erase(mat);

			return;
		}
	}

	printf("ZedMatrixPool::freeMat - Error! matrix [%p] was found at m_active_matrix_by_data but could not complete erase process.\n",mat);
	assert(0);
	//m_Matrix_Allocator->free(mat);
}