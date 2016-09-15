#pragma once

#include "defines.h"
#include <Eigen/Dense>
#include "zedmatrixbase.h"
//#include "RefPointer.h"


///////////////////////////
// Note: there is a difference between the cuda implementation and the cpu implementation.
// cuda is reference based - so "a = b" will make both a and b point to the same real resources.
// cpu isn't - so "a = b" will allocate a matrix for a and really copy the data from b to a.

class ZedMatrix_Cuda; //predecl
class ZedMatrix_Cpu :
	public ZedMatrixBase
{
public:
	ZedMatrix_Cpu(unsigned int rows, unsigned int cols);
	virtual ~ZedMatrix_Cpu(void);

	virtual ZedMatrix_Cpu operator+ (const ZedMatrix_Cpu& other);	
	virtual ZedMatrix_Cpu operator* (const ZedMatrix_Cpu& other);
	virtual ZedMatrix_Cpu operator* (PREC_TYPE scalar);
	virtual void operator*= (PREC_TYPE scalar);
	virtual void operator*= (const ZedMatrix_Cpu& other);
	virtual ZedMatrix_Cpu Transpose();

	virtual bool operator== (ZedMatrix_Cuda& other);

	virtual void Rand();
	virtual void Print();

	virtual void* Lock(unsigned int flags) {return NULL;}
	virtual void Unlock() {}

	//TODO:
	//virtual void Transpose();
	
	//operator =
	void operator= (const ZedMatrix_Cpu& other);


	void print_dbg_data();

private:

	typedef Eigen::Matrix<PREC_TYPE, Eigen::Dynamic, Eigen::Dynamic> EIGEN_DYN_MAT;

	//copy constructor
	ZedMatrix_Cpu(const ZedMatrix_Cpu& other);

	//Eigen::Matrix<PREC_TYPE, Eigen::Dynamic, Eigen::Dynamic> m_mat;

	private:

	//RefPointer* m_eigen_mat;
	EIGEN_DYN_MAT* m_eigen_mat;
};

