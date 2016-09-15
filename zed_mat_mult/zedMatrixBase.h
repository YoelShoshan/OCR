#ifndef ZED_MATRIX_BASE_H
#define ZED_MATRIX_BASE_H

class ZedMatrixBase
{
public:

	enum ZED_LOCK_TYPE
	{
		ZED_LOCK_NOT_INITIALIZED=0,
		ZED_LOCK_READ = 1 << 1,
		ZED_LOCK_WRITE = 1 << 2
	};

	ZedMatrixBase(unsigned int rows, unsigned int cols) : m_rows(rows) , m_cols(cols)
	{}

	ZedMatrixBase(const ZedMatrixBase& other) 
	{		
		m_rows = other.m_rows;
		m_cols = other.m_cols;
	};


	virtual ~ZedMatrixBase(void){}

	virtual void Rand() = 0;
	virtual void Print() = 0;
	
	virtual void* Lock(unsigned int flags) = 0;
	virtual void Unlock() = 0;
	
	virtual unsigned int GetRows() const { return m_rows; }
	virtual unsigned int GetCols() const { return m_cols; }
	
protected:
	unsigned int m_rows,m_cols;
};

#endif