#ifndef ZED_MATRIX_POOL_H
#define ZED_MATRIX_POOL_H

#include "ZedMatAllocatorBase.h"
#include <map>

class ZedMatrixPool
{
public:
	ZedMatrixPool(ZedMatrixAllocatorBase* mat_allocator);
	virtual ~ZedMatrixPool();

	void* getMat(unsigned int rows, unsigned int cols);
	void freeMat(void* mat);	

private:
	ZedMatrixAllocatorBase* m_Matrix_Allocator;

	struct pool_map_key
	{
		int rows;
		int cols;
		//TODO: add precision type - float/double

		bool operator < (const pool_map_key& other) const
		{
			if (rows < other.rows)
			{
				return true;
			} else if (rows > other.rows)
			{
				return false;
			}

			//rows==other.rows

			if (cols < other.cols)
			{
				return true;
			}

			return false;
		}

		bool operator == (const pool_map_key& other) const
		{
			return ( (rows == other.rows) && (cols == other.cols));
		}
	};

	typedef std::map<void*,pool_map_key> ZED_POOL_REV_MAP;
	typedef ZED_POOL_REV_MAP::iterator ZED_POOL_REV_MAP_IT;

	typedef std::multimap<pool_map_key,void*> ZED_POOL_MM;
	typedef ZED_POOL_MM::iterator ZED_POOL_MM_IT;

	/*std::map<pool_map_key,void*> m_pool_key_to_data; //to find a new free one
	std::map<void*,pool_map_key> m_pool_data_to_key;*/

	ZED_POOL_MM m_active_matrices;
	ZED_POOL_REV_MAP m_active_matrix_by_data;

	ZED_POOL_MM m_free_matrices;		
};


#endif