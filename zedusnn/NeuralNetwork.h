#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

//#include "NN_Genome.h"
//#include <Windows.h>
//#include <Windows.h>
#include <iostream>
#include <Eigen/Dense>
#include "Defines.h"

using namespace Eigen;
using namespace std;

struct NeuralNetwork
{
private:
	inline double	  Sigmoid(double activation, double p);
	void RandomizeVector(double* pVec, unsigned int size);
public:

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	
	void AssignID()
	{
		static unsigned int S_NextID = 0;

		m_uiID = S_NextID;
		S_NextID++;
	}

	NeuralNetwork(unsigned int inputs_num, unsigned int hidden_layers_num, unsigned int hidden_layer_size, unsigned int outputs_num);
	NeuralNetwork(const char* pcNN_File);


	void Init(unsigned int inputs_num, unsigned int hidden_layers_num, unsigned int hidden_layer_size, unsigned int outputs_num, bool bRandomize);
	
	void Forward_Pass(ZED_IN int iInputsNum,ZED_IN double* pInputs, ZED_OUT int iOutputsNum, ZED_OUT double* pOutputs);
	
	void PrepareForMiniBatch();

	void CalculateDeltaValsForBackProp(double* pTargetOutputs);

	void Accumulate_BackPropBasedWeightChanges_BatchSubCase(/*double dStep,*//* double* inputs*/);

	void FinishMiniBatch(double dLearnFactor, double dViscosity);

	void SaveToFile(const char* pFileName);
	
	static const int MAGIC_VAL = 0x12345;	
	int m_iMagicVal; //to identify in case of void pointer.

	void log_Matrix(const char* msg, Matrix<double, Dynamic, Dynamic>* m);

	void NonOptimizedRPropUpdate(
		ZED_IN Matrix<double, Dynamic, Dynamic>* curr_derv, 
		ZED_IN Matrix<double, Dynamic, Dynamic>* prev_derv, 
		ZED_IN Matrix<double, Dynamic, Dynamic>* rprop_steps, 
		ZED_IN ZED_OUT Matrix<double, Dynamic, Dynamic>* weights);

	void BackupCurrentNN();
	void RestoreBackup();
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// nodes weights matrices
	//rows are weights
	//columns are neurons ("perceptron")
	//always +1 to weight because of bias

	//Matrix<double,m_InputsNum+1,1> *m_vInputTemp;
	Matrix<double,Dynamic,1> *m_vInputTemp;

	Matrix<double, Dynamic, Dynamic> *m_pHiddenLayer_First;
	Matrix<double, Dynamic, Dynamic > *m_pHiddenLayer_First_MiniBatch_Accumulated;
	Matrix<double, Dynamic, Dynamic > *m_pHiddenLayer_First_Prev;
	Matrix<double, Dynamic, Dynamic > *m_pHiddenLayer_First_RpropStep;
	Matrix<double, Dynamic, Dynamic> *m_pHiddenLayer_First_Backup; // in order to be able to return to previous state.

//#if HIDDEN_LAYERS_NUM > 1	
	Matrix<double, Dynamic, Dynamic > **m_ppNextHiddenLayers;//[HIDDEN_LAYERS_NUM-1];
	Matrix<double, Dynamic, Dynamic > **m_ppNextHiddenLayers_MiniBatch_Accumulated;//[HIDDEN_LAYERS_NUM-1];
	Matrix<double, Dynamic, Dynamic > **m_ppNextHiddenLayers_Prev;//[HIDDEN_LAYERS_NUM-1];
	Matrix<double, Dynamic, Dynamic > **m_ppNextHiddenLayers_RpropStep;//[HIDDEN_LAYERS_NUM-1];
	Matrix<double, Dynamic, Dynamic > **m_ppNextHiddenLayers_Backup;//[HIDDEN_LAYERS_NUM-1];
//#endif	
		
	Matrix<double, Dynamic, Dynamic > *m_pOutputLayer;
	Matrix<double, Dynamic, Dynamic> *m_pOutputLayer_MiniBatch_Accumulated;
	Matrix<double, Dynamic, Dynamic> *m_pOutputLayer_Prev;
	Matrix<double, Dynamic, Dynamic> *m_pOutputLayer_RpropStep;
	Matrix<double, Dynamic, Dynamic> *m_pOutputLayer_Backup;


	//Matrix<double, Dynamic, HIDDEN_LAYER_SIZE+1> m_OutputLayer;
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// vectors (inputs/outputs)
	//storing computation intermediate values for usage in the back prop
	
	Matrix<double,Dynamic,1> **m_ppvHiddenLayers_Result;//[HIDDEN_LAYERS_NUM];
	Matrix<double,Dynamic,1> **m_ppvHiddenLayers_DeltaVals;//[HIDDEN_LAYERS_NUM];
	
	Matrix<double,Dynamic, 1> *m_pvFinalOutput;
	Matrix<double,Dynamic, 1> *m_pvFinalOutput_DeltaVals;
	Matrix<double,Dynamic, 1> *m_pvFinalOutput_DeltaVals_Numerical;//testing it numerically to make sure the math is ok (debug)
	Matrix<double,Dynamic, 1> *m_pvOutputOnes;

	unsigned int m_InputsNum;	
	unsigned int m_HiddenLayersNum;
	unsigned int m_HiddenLayerSize;
	unsigned int m_OutputsNum;
	
	double	m_dScore;
	unsigned int m_uiID;
	unsigned int m_dwSeed;
};






#endif
