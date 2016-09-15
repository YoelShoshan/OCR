#include "RefPointer.h"
#include <stdio.h>
#include <assert.h>
#include "defines.h"


RefPointer::RefPointer(void* data,ZedMatrixPool* pool)
{
	assert(data);
	assert(pool);
	m_count = 1;
	m_data = data;
	m_pool = pool;

	//printf("ERROR !!!!!! I STOPPED USING THIS CLASS FOR NOW (RefPointer)\n");
	//assert(0);
}


RefPointer::~RefPointer(void)
{
}

void RefPointer::dbg_print()
{
	VERBOSE_DBG_PRINT("data=[%p]\n",m_data);
}

void RefPointer::Inc()
{
	m_count++;
	VERBOSE_DBG_PRINT("\t++ count=%d\n",m_count);
}

void RefPointer::Dec()
{
	m_count--;
	if (0==m_count)
	{
		VERBOSE_DBG_PRINT("Count reached zero! Deleting [%p]\n",m_data);
		m_pool->freeMat(m_data);
	} else
	{
		VERBOSE_DBG_PRINT("\t-- count=%d\n",m_count);
	}

	if (m_count <0 )
	{
		printf("RefPointer::Dec - Error! ref count < 0!\n");
	}
}