#ifndef EIGEN_MAT_MUL
#define EIGEN_MAT_MUL

#include "defines.h"
#include <Eigen/Dense>

template <class T,int WA, int HA,int WB, int HB>
class EigenMatMult
{
public:

	int a_width,a_height;
	int b_width,b_height;
	int c_width,c_height;

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	EigenMatMult(void)
	{
		// allocation
		a_width = WA;
		a_height = HA;

		b_width = WB;
		b_height = HB;

		c_width = WA;
		c_height = HB;

		m_a = new Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic >(a_width,a_height);		
		m_b = new Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic >(b_width,b_height);		
		m_c = new Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic >(c_width,c_height);		
	}

	virtual ~EigenMatMult(void)
	{
		delete m_a;
		m_a = NULL;

		delete m_b;
		m_b = NULL;

		delete m_c;
		m_c = NULL;
	}

	bool mult()
	{
		(*m_c).noalias() = (*m_a)*(*m_b);
		return true;
	}

	//Eigen::MatrixXd<

	Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic > *m_a, *m_b, *m_c;
};

#endif

