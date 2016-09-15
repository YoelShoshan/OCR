#pragma once
#include "ZedMatPool.h"

class RefPointer
{	
public:
	RefPointer(void* data,ZedMatrixPool* pool);
	virtual ~RefPointer(void);

	void dbg_print();

	void Inc();
	void Dec();

	void* m_data;

protected:
	unsigned int m_count;	
	ZedMatrixPool* m_pool;  //don't mind adding this per item, since actual mat operations should be significantly more computationally heavy than this.

	//friend class ZedMatrix_Cpu;
	//friend class ZedMatrix_Cuda;
	

private:
	RefPointer(const RefPointer& other); //prevent usage
	RefPointer operator = (const RefPointer& other); //prevent usage
};

