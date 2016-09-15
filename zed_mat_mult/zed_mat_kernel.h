#ifndef ZED_MAT_KERNEL_H
#define ZED_MAT_KERNEL_H

#include "cuda_runtime.h"

#include "defines.h"



void kerMatrixAdd_FromC(PREC_TYPE* d_c, const PREC_TYPE* d_a, const PREC_TYPE* d_b, int rows, int cols);
void kerMatrixMul_FromC(PREC_TYPE* d_c, const PREC_TYPE* d_a, const PREC_TYPE* d_b, int a_rows, int a_cols,int b_rows, int b_cols);

// res = src*s
void kerMatrixMul_With_Scalar_FromC(PREC_TYPE* res, PREC_TYPE* src, PREC_TYPE s, int rows, int cols);

void kerMatrixTranspose_FromC(PREC_TYPE* dst, PREC_TYPE* src, int src_rows, int src_cols);

__global__ void kerMatrixAdd( PREC_TYPE* C, const PREC_TYPE* A, const PREC_TYPE* B, int rows, int cols);
__global__ void kerMatrixMul( PREC_TYPE* C, const PREC_TYPE* A, const PREC_TYPE* B,int a_rows, int a_cols,int b_rows, int b_cols);
__global__ void kerMatrixMulWithScalar( PREC_TYPE* res, PREC_TYPE* src, PREC_TYPE s, int rows, int cols);
__global__ void kerMatrixTranspose(PREC_TYPE* dst, PREC_TYPE* src, int src_rows, int src_cols);

#endif