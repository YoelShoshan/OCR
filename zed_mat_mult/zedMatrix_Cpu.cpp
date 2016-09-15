#include "ZedMatrix_Cpu.h"
#include "ZedMatPool.h"
#include "defines.h"
#include "zedMatrix_Cuda.h"

extern ZedMatrixPool* g_pMatrixPool_Eigen;

ZedMatrix_Cpu::ZedMatrix_Cpu(unsigned int rows, unsigned int cols) : ZedMatrixBase(rows,cols)
{
	VERBOSE_DBG_PRINT("ZedMatrix_Cpu Constructor %p\n",this);	
	assert(g_pMatrixPool_Eigen);

	//m_eigen_mat = new RefPointer(g_pMatrixPool_Eigen->getMat(width,height),g_pMatrixPool_Eigen );
	m_eigen_mat = (EIGEN_DYN_MAT*)g_pMatrixPool_Eigen->getMat(rows,cols);
	assert(m_eigen_mat);

	print_dbg_data();
}

ZedMatrix_Cpu::ZedMatrix_Cpu(const ZedMatrix_Cpu& other) : ZedMatrixBase(other)
{
	VERBOSE_DBG_PRINT("Copy Constructor ZedMatrix_Cpu %p\n",this);	

	assert(g_pMatrixPool_Eigen);
	m_eigen_mat = (EIGEN_DYN_MAT*)g_pMatrixPool_Eigen->getMat(other.GetRows(),other.GetCols());
	assert(m_eigen_mat);


	(*m_eigen_mat).noalias() = (*other.m_eigen_mat);
	//m_eigen_mat->Inc();

	print_dbg_data();
}

void ZedMatrix_Cpu::operator= (const ZedMatrix_Cpu& other)
{
	//////////////////////////////////////////////////////////////////////
	// this version just points to the other's matrix.
	// the problem is that when someone writes a=b,
	// the expected behaviour is a copy, not another reference.
	// however, this is faster. So if this is well defined in the syntax,
	// maybe it's ok.
	//////////////////////////////////////////////////////////////////////
	assert(m_eigen_mat);
	//m_eigen_mat->Dec();

	m_rows = other.GetRows();
	m_cols = other.GetCols();

	VERBOSE_DBG_PRINT("operator= ZedMatrix_Cpu %p\n",this);	
	(*m_eigen_mat).noalias() = (*other.m_eigen_mat);
	//m_eigen_mat->Inc();

	print_dbg_data();
}


ZedMatrix_Cpu::~ZedMatrix_Cpu(void)
{
	VERBOSE_DBG_PRINT("ZedMatrix_Cuda Destructor %p\n",this);
	print_dbg_data();
	assert(g_pMatrixPool_Eigen);
	//printf("Freeing mat dev %p\n",m_dev_mat);
	g_pMatrixPool_Eigen->freeMat(m_eigen_mat);

	//m_eigen_mat->Dec();
	m_eigen_mat = 0;
}




void ZedMatrix_Cpu::print_dbg_data()
{
	//printf("\tDev:%p, Host:%p\n",m_dev_mat,m_host_mat);
	VERBOSE_DBG_PRINT("Eigen: ");
	//m_eigen_mat->dbg_print();
}


ZedMatrix_Cpu ZedMatrix_Cpu::operator+ (const ZedMatrix_Cpu& other)
{
	assert(m_rows == other.m_rows);
	assert(m_cols == other.m_cols);

	ZedMatrix_Cpu res(other.m_rows,m_cols);

	EIGEN_DYN_MAT* a = m_eigen_mat;
	EIGEN_DYN_MAT* b = other.m_eigen_mat;
	EIGEN_DYN_MAT* c = res.m_eigen_mat;

	(*c).noalias() = (*a)+(*b);

	return res;
}


ZedMatrix_Cpu ZedMatrix_Cpu::operator* (const ZedMatrix_Cpu& other)
{
	assert(m_cols == other.m_rows);

	ZedMatrix_Cpu res(m_rows,other.m_cols);

	EIGEN_DYN_MAT* a = m_eigen_mat;
	EIGEN_DYN_MAT* b = other.m_eigen_mat;
	EIGEN_DYN_MAT* c = res.m_eigen_mat;

	(*c).noalias() = (*a)*(*b);
	//(*c).noalias() = (*b)*(*a);

	return res;
}

ZedMatrix_Cpu ZedMatrix_Cpu::operator* (PREC_TYPE scalar)
{
	ZedMatrix_Cpu res(m_rows,m_cols);

	(*res.m_eigen_mat).noalias() = (*m_eigen_mat)*scalar;
	
	return res;
}

void ZedMatrix_Cpu::operator*= (PREC_TYPE scalar)
{
	(*m_eigen_mat)*= scalar;
}

void ZedMatrix_Cpu::operator*= (const ZedMatrix_Cpu& other)
{
	(*m_eigen_mat)*= (*other.m_eigen_mat);
}

ZedMatrix_Cpu ZedMatrix_Cpu::Transpose()
{
	ZedMatrix_Cpu res(m_cols,m_rows);

	(*res.m_eigen_mat)/*.noalias()*/ = (*m_eigen_mat).transpose();
	
	return res;

	
}

bool ZedMatrix_Cpu::operator== (ZedMatrix_Cuda& other)
{
	if (m_rows != other.GetRows())
	{
		assert(0=="operator==    matrices rows is different.");
		return false;
	}

	if (m_cols != other.GetCols())
	{
		assert(0=="operator==    matrices cols is different.");
		return false;
	}

	bool res = true;
	PREC_TYPE* cuda_mat_data = (PREC_TYPE*)other.Lock(ZED_LOCK_READ);
	EIGEN_DYN_MAT* mat =m_eigen_mat;

	PREC_TYPE cpu_val,gpu_val;

	for (int j=0;j<m_rows;j++)
	{
		for (int i=0;i<m_cols;i++)
		{
			cpu_val = (*mat)(j,i);
			gpu_val = cuda_mat_data[i+(j*m_cols)];

			if (cpu_val != gpu_val && (fabs(cpu_val-gpu_val) > 0.00001f))
			{
				printf("Comparison failed! (%d,%d) cuda_val=%f, cpu_val=%f\n",
					i,j,gpu_val,cpu_val);
					
				res = false;
				break;
			}			
		}

		if (!res)
		{
			break;
		}
	}

	other.Unlock();

	return res;
}

void ZedMatrix_Cpu::Rand()
{
	EIGEN_DYN_MAT* mat =m_eigen_mat;

	for (int i=0;i<m_rows;i++)	
	{
		for (int j=0;j<m_cols;j++)
		{
			(*mat)(i,j) = rand() / (PREC_TYPE)RAND_MAX;
		}
	}
}

void ZedMatrix_Cpu::Print()
{
	printf("Matrix cpu eigen [%p]:\n",m_eigen_mat);

	EIGEN_DYN_MAT* mat = m_eigen_mat;

	for (int j=0;j<m_rows;j++)
	{
		for (int i=0;i<m_cols;i++)
		{
			//printf("%.3f ",(*mat)[i+(j*m_width)]);
			printf("%.3f ",(*mat)(j,i));
			if (i>5)
			{
				printf("...");
				break;
			}
		}
		printf("\n");

		if (j>5)
		{
			printf("...");
			break;
		}
	}
	printf("\n");
}


