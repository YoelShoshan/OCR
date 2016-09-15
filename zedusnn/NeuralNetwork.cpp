//#include "stdafx.h"
#include "NeuralNetwork.h"
#include <assert.h>
//#include <SFMT/fmt.h>
//#include <SFMT/SFMT.h>
#include <SFMT.h>    
#include <math.h>
#include "Defines.h"
#include "zedusOS.h"

//TODO: fix it to return double


#define CHANGE_OUTPUT_LAYER_WEIGHT
#define CHANGE_HIDDEN_LAYER_WEIGHT

extern unsigned int g_dwSeed;
extern FILE* g_pLog;


NeuralNetwork::NeuralNetwork(unsigned int inputs_num, unsigned int hidden_layers_num, unsigned int hidden_layer_size, unsigned int outputs_num)
{
	m_iMagicVal = MAGIC_VAL;
	m_dwSeed = g_dwSeed;
	Init(inputs_num, hidden_layers_num, hidden_layer_size, outputs_num, true);
}

NeuralNetwork::NeuralNetwork(const char* pcNN_File)
{
	FILE* f = fopen(pcNN_File,"rb");

	ERROR_EXIT_IF(!f, "Error: LoadFromFile: failed creating file for reading: [%s]\n",pcNN_File);
	

	fread(&m_dwSeed,sizeof(unsigned int),1,f); //if we'll need to recreate the database creation, we should be able now.
	fread(&m_InputsNum,sizeof(unsigned int),1,f);	
	fread(&m_HiddenLayersNum,sizeof(unsigned int),1,f);
	fread(&m_HiddenLayerSize,sizeof(unsigned int),1,f);
	fread(&m_OutputsNum,sizeof(unsigned int),1,f);

	{
		char error_str[256];

		ERROR_EXIT_IF(m_InputsNum > 0xFFFF, "Inputs Num is far too big (0x%08X) - perhaps a corrupted file? Exiting.", m_InputsNum);
		ERROR_EXIT_IF(m_HiddenLayersNum > 0xFFFF, "Hidden Layers Num is far too big (0x%08X) - perhaps a corrupted file? Exiting.", m_HiddenLayersNum);
		ERROR_EXIT_IF(m_HiddenLayerSize > 0xFFFF, "Hidden Layer Size Num is far too big (0x%08X) - perhaps a corrupted file? Exiting.", m_HiddenLayerSize);
		ERROR_EXIT_IF(m_OutputsNum > 0xFFFF, "Outputs Num is far too big (0x%08X) - perhaps a corrupted file? Exiting.", m_OutputsNum);
				
	}

	double *pData = new double[1024*1024];
	double element;

	Init(m_InputsNum,m_HiddenLayersNum,m_HiddenLayerSize,m_OutputsNum, false);

	for (int i=0;i<m_HiddenLayerSize+1;i++)
	{
		for (int j=0;j<m_InputsNum+1;j++)
		{
			fread(&element,sizeof(double),1,f);
			(*m_pHiddenLayer_First)(i,j) = element;
		}
	}

	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		for (int i=0;i<m_HiddenLayerSize+1;i++)
		{
			for (int j=0;j<m_HiddenLayerSize+1;j++)
			{
				fread(&element,sizeof(double),1,f);
				(*m_ppNextHiddenLayers[l])(i,j) = element;
			}
		}
	}

	for (int i=0;i<m_OutputsNum+1;i++)
	{
		for (int j=0;j<m_HiddenLayerSize+1;j++)
		{
			fread(&element,sizeof(double),1,f);
			(*m_pOutputLayer)(i,j) = element;
		}
	}


	/*fread(m_pHiddenLayer_First->data(), sizeof(double),(m_HiddenLayerSize+1)*(m_InputsNum+1),f);
	
	for (int i=0;i<m_HiddenLayersNum-1;i++)
	{
		fread(m_ppNextHiddenLayers[i]->data(), sizeof(double), (m_HiddenLayerSize+1)*(m_HiddenLayerSize+1),f);
	}		
	
	fread(m_pOutputLayer, sizeof(double), (m_OutputsNum+1)*(m_HiddenLayerSize+1),f);*/
	

	SAFE_DEL_ARRAY(pData);

	fclose(f);
}

void NeuralNetwork::RandomizeVector(double* pVec, unsigned int size)
{
	assert(pVec);
	for (int i=0;i<size;i++)
	{
		//pVec[i] = SFMT_IN_RANGE(-1.0,1.0);
		//pVec[i] = 0.f;

		//pVec[i] = SFMT_IN_RANGE(-0.1,0.1);

		pVec[i] = SFMT_IN_RANGE(-0.01,0.01);

	}
}

void NeuralNetwork::Init(unsigned int inputs_num, unsigned int hidden_layers_num, unsigned int hidden_layer_size, unsigned int outputs_num, bool bRandomize)  
{	
	AssignID();
	m_InputsNum = inputs_num;
	m_HiddenLayersNum = hidden_layers_num;
	m_HiddenLayerSize = hidden_layer_size;
	m_OutputsNum = outputs_num;

	m_vInputTemp = new Matrix<double,Dynamic,1>(m_InputsNum+1,1);

	m_pHiddenLayer_First = new Matrix<double, Dynamic, Dynamic>(m_HiddenLayerSize+1, m_InputsNum+1);
	m_pHiddenLayer_First_MiniBatch_Accumulated = new Matrix<double, Dynamic, Dynamic>(m_HiddenLayerSize+1, m_InputsNum+1);
	m_pHiddenLayer_First_Prev =  new Matrix<double, Dynamic, Dynamic>(m_HiddenLayerSize+1, m_InputsNum+1);
	m_pHiddenLayer_First_RpropStep =  new Matrix<double, Dynamic, Dynamic>(m_HiddenLayerSize+1, m_InputsNum+1);
	m_pHiddenLayer_First_Backup =  new Matrix<double, Dynamic, Dynamic>(m_HiddenLayerSize+1, m_InputsNum+1);


	m_ppNextHiddenLayers = new Matrix<double, Dynamic, Dynamic >* [m_HiddenLayersNum-1];
	m_ppNextHiddenLayers_MiniBatch_Accumulated = new Matrix<double, Dynamic, Dynamic >* [m_HiddenLayersNum-1];
	m_ppNextHiddenLayers_Prev = new Matrix<double, Dynamic, Dynamic >* [m_HiddenLayersNum-1];
	m_ppNextHiddenLayers_RpropStep = new Matrix<double, Dynamic, Dynamic >* [m_HiddenLayersNum-1];
	m_ppNextHiddenLayers_Backup = new Matrix<double, Dynamic, Dynamic >* [m_HiddenLayersNum-1];


	for (int i=0;i<m_HiddenLayersNum-1;i++)
	{
		m_ppNextHiddenLayers[i] = new Matrix<double, Dynamic, Dynamic >(m_HiddenLayerSize+1, m_HiddenLayerSize+1);		
		m_ppNextHiddenLayers_MiniBatch_Accumulated[i] = new Matrix<double, Dynamic, Dynamic >(m_HiddenLayerSize+1, m_HiddenLayerSize+1);		
		m_ppNextHiddenLayers_Prev[i] = new Matrix<double, Dynamic, Dynamic >(m_HiddenLayerSize+1, m_HiddenLayerSize+1);		
		m_ppNextHiddenLayers_RpropStep[i] = new Matrix<double, Dynamic, Dynamic >(m_HiddenLayerSize+1, m_HiddenLayerSize+1);		
		m_ppNextHiddenLayers_Backup[i] = new Matrix<double, Dynamic, Dynamic >(m_HiddenLayerSize+1, m_HiddenLayerSize+1);		
	}		
	
	m_pOutputLayer = new Matrix<double, Dynamic, Dynamic >(m_OutputsNum+1, m_HiddenLayerSize+1);
	m_pOutputLayer_MiniBatch_Accumulated = new Matrix<double, Dynamic, Dynamic >(m_OutputsNum+1, m_HiddenLayerSize+1);
	m_pOutputLayer_Prev = new Matrix<double, Dynamic, Dynamic >(m_OutputsNum+1, m_HiddenLayerSize+1);
	m_pOutputLayer_RpropStep = new Matrix<double, Dynamic, Dynamic >(m_OutputsNum+1, m_HiddenLayerSize+1);
	m_pOutputLayer_Backup = new Matrix<double, Dynamic, Dynamic >(m_OutputsNum+1, m_HiddenLayerSize+1);


	// init prev values

	(*m_pOutputLayer_Prev).setZero();
	(*m_pHiddenLayer_First_Prev).setZero();
	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{		
		(*m_ppNextHiddenLayers_Prev[l]).setZero();		
	}

	// init rprop

	(*m_pOutputLayer_RpropStep).setConstant(RPROP_INITIAL_STEP_SIZE);
	(*m_pHiddenLayer_First_RpropStep).setConstant(RPROP_INITIAL_STEP_SIZE);
	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{		
		(*m_ppNextHiddenLayers_RpropStep[l]).setConstant(RPROP_INITIAL_STEP_SIZE);		
	}	


	m_ppvHiddenLayers_Result = new Matrix<double,Dynamic,1>* [m_HiddenLayersNum];
	m_ppvHiddenLayers_DeltaVals = new Matrix<double,Dynamic,1>* [m_HiddenLayersNum];
	
	for (int i=0;i<m_HiddenLayersNum;i++)
	{
		m_ppvHiddenLayers_Result[i] = new Matrix<double,Dynamic,1>(m_HiddenLayerSize+1,1);
		int k=0;
		int j=0;
		m_ppvHiddenLayers_DeltaVals[i] = new Matrix<double,Dynamic,1>(m_HiddenLayerSize+1,1);
	}	

	m_pvFinalOutput = new Matrix<double,Dynamic, 1>(m_OutputsNum+1,1);

	m_pvFinalOutput_DeltaVals = new Matrix<double,Dynamic, 1>(m_OutputsNum+1, 1);

	m_pvFinalOutput_DeltaVals_Numerical = new Matrix<double,Dynamic, 1>(m_OutputsNum+1, 1);

	m_pvOutputOnes = new Matrix<double,Dynamic, 1>(m_OutputsNum+1, 1);




	//////////////
	// Randomize
	//////////////
	
	RandomizeVector((*m_pHiddenLayer_First).data(),(m_HiddenLayerSize+1) * (m_InputsNum+1));

	for (int i=0;i<m_HiddenLayersNum-1;i++)
	{
		RandomizeVector((*m_ppNextHiddenLayers[i]).data(),(m_HiddenLayerSize+1) * (m_HiddenLayerSize+1));
	}

	RandomizeVector((*m_pOutputLayer).data(), (m_OutputsNum+1)*(m_HiddenLayerSize+1));

	for (int i=0;i<m_OutputsNum+1; i++)
	{
		(*m_pvOutputOnes)[i] = 1.f;
	}

	// init backup (to return to in case that we made a bad decision)
	BackupCurrentNN();	

}

void NeuralNetwork::BackupCurrentNN()
{
	(*m_pOutputLayer_Backup).noalias() = (*m_pOutputLayer);
	(*m_pHiddenLayer_First_Backup).noalias() = (*m_pHiddenLayer_First);
	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{		
		(*m_ppNextHiddenLayers_Backup[l]).noalias()	= (*m_ppNextHiddenLayers[l]);
	}
	
}

void NeuralNetwork::RestoreBackup()
{
	(*m_pOutputLayer).noalias() = (*m_pOutputLayer_Backup);
	(*m_pHiddenLayer_First).noalias() = (*m_pHiddenLayer_First_Backup);
	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{		
		(*m_ppNextHiddenLayers[l]).noalias()	= (*m_ppNextHiddenLayers_Backup[l]);
	}
}

double NeuralNetwork::Sigmoid(double activation, double p)
{
	return ( 1 / ( 1 + exp(-activation / p)));
} 

///////////////////////////////////////////////////////////
// note: always remember to pass an extra input for bias !
//////////////////////////////////////////////////////////

 extern bool g_dbg;

 void NeuralNetwork::Forward_Pass(ZED_IN int iInputsNum,ZED_IN  double* pInputs, ZED_OUT int iOutputsNum, ZED_OUT double* pOutputs)
 {	
	//assert(_CrtCheckMemory());

	double dActivationResponse = 1.0;

	// get input values into the input vector
	
	//memcpy((*m_vInputTemp).data(),pInputs,m_InputsNum * sizeof(double));
	for (int i=0;i<m_InputsNum;i++)
	{
		(*m_vInputTemp)[i] = pInputs[i];
	}

	(*m_vInputTemp)[m_InputsNum] = -1.f; //for the bias
	
	//assert(_CrtCheckMemory());
	// calculate first hidden layer result
	
	(*m_ppvHiddenLayers_Result[0]) = (*m_pHiddenLayer_First)*(*m_vInputTemp);

	if (g_dbg)
	{
		LOG("Hidden Layer First:\n");
	}

	for (int i=0;i<m_HiddenLayerSize+1;i++)
	{
		(*m_ppvHiddenLayers_Result[0])[i] = Sigmoid((*m_ppvHiddenLayers_Result[0])[i], 1.0);
		if (g_dbg)
		{
			if (i%100==99)
			{
				LOG("\n");			
			}

			double val = (*m_ppvHiddenLayers_Result[0])[i];
			LOG("(%d,%f 0x%08X%08X)\t", i, val, ((unsigned int*)&val)[0], ((unsigned int*)&val)[1]);

			
		}		
	}
	
	if (g_dbg)
	{
		LOG("\n");
	}


	(*m_ppvHiddenLayers_Result[0])[m_HiddenLayerSize] = -1.f;

	if (g_dbg)
	{
		double val = (*m_ppvHiddenLayers_Result[0])[m_HiddenLayerSize];
		LOG("(%d,%f 0x%08X%08X)\t\n", m_HiddenLayerSize, val, ((unsigned int*)&val)[0], ((unsigned int*)&val)[1]);
	}

	//OutputDebugStringA("1\n");
		
	//assert(_CrtCheckMemory());
	// calculate the next hidden layer results

	for (int l=1;l<m_HiddenLayersNum;l++)
	{
		if (g_dbg)
		{
			LOG("Hidden Layer %d\n", l);
		}

		(*m_ppvHiddenLayers_Result[l]) = (*m_ppNextHiddenLayers[l-1]) * (*m_ppvHiddenLayers_Result[l-1]);
		for (int i=0;i<m_HiddenLayerSize;i++)
		{
			(*m_ppvHiddenLayers_Result[l])[i] = Sigmoid((*m_ppvHiddenLayers_Result[l])[i], 1.0);

			if (g_dbg)
			{
				if (i%100==99)
				{
					LOG("\n");			
				}
				double val = (*m_ppvHiddenLayers_Result[l])[i];
				LOG("(%d,%f 0x%08X%08X)\t", i, val, ((unsigned int*)&val)[0], ((unsigned int*)&val)[1]);
			}
		}

		if (g_dbg)
		{
			LOG("\n");	
		}

		//(*m_ppvHiddenLayers_Result[l]).data()[m_HiddenLayerSize] = -1.f; //bias
		(*m_ppvHiddenLayers_Result[l])[m_HiddenLayerSize] = -1.f; //bias
		if (g_dbg)
		{
			double val = (*m_ppvHiddenLayers_Result[0])[m_HiddenLayerSize];
			//LOG("(%d,%f)\t\n", m_HiddenLayerSize, (*m_ppvHiddenLayers_Result[0])[m_HiddenLayerSize]);			
			LOG("(%d,%f 0x%08X%08X)\t", m_HiddenLayerSize, val, ((unsigned int*)&val)[0], ((unsigned int*)&val)[1]);
		}
	}


	if (g_dbg)
	{
		LOG("Output layer\n");
	}
	
	//assert(_CrtCheckMemory());
	//OutputDebugStringA("2\n");

	(*m_pvFinalOutput) = (*m_pOutputLayer) * (*m_ppvHiddenLayers_Result[m_HiddenLayersNum-1]);

	//assert(_CrtCheckMemory());
	//OutputDebugStringA("3\n");

	for (int i=0;i<m_OutputsNum;i++)
	{
		if ((*m_pvFinalOutput)[i] > 0.f)
		{
			int dbg = 0.0;
		}

		(*m_pvFinalOutput)[i] = Sigmoid((*m_pvFinalOutput)[i], 1.0);
		if (g_dbg)
		{
			if (i%100==99)
			{
				LOG("\n");			
			}

			LOG("(%d,%f)\t",i,(*m_pvFinalOutput)[i]);
		}
	}

	if (g_dbg)
	{
		LOG("\n");	
	}

	//assert(_CrtCheckMemory());

	(*m_pvFinalOutput)[m_OutputsNum] = -1.f; //bias

	if (g_dbg)
	{
		LOG("%(%d,%f)\t\n",m_OutputsNum,(*m_pvFinalOutput)[m_OutputsNum]);
	}

	//assert(_CrtCheckMemory());

	//memcpy(pOutputs,(*m_pvFinalOutput).data(),sizeof(double)*m_OutputsNum+1);
	for (int i=0;i<m_OutputsNum+1;i++)
	{
		pOutputs[i] = (*m_pvFinalOutput)[i];
	}

	//assert(_CrtCheckMemory());
}

void NeuralNetwork::log_Matrix(const char* msg, Matrix<double, Dynamic, Dynamic>* m)
{
	LOG(msg);
	for (int j=0;j<m->cols();j++)
	{
		for (int i=0;i<m->rows();i++)
		{
			LOG("(%d,%f)\t",i,(*m)(i,j));
		}
		LOG("\n");
	}
	LOG("\n");		
}

void NeuralNetwork::NonOptimizedRPropUpdate(
		ZED_IN Matrix<double, Dynamic, Dynamic>* curr_derv, 
		ZED_IN Matrix<double, Dynamic, Dynamic>* prev_derv, 
		ZED_IN Matrix<double, Dynamic, Dynamic>* rprop_steps, 
		ZED_IN ZED_OUT Matrix<double, Dynamic, Dynamic>* weights)
{
	double dMul = 0.0;

	for (int i=0;i<weights->rows();i++)
	{
		for (int j=0;j<weights->cols();j++)
		{
			dMul = (*curr_derv)(i,j) * (*prev_derv)(i,j);
			if ( dMul > 0.0)   // same sign
			{
				(*rprop_steps)(i,j) *= RPROP_INCREASE_STEP_FACTOR;
				(*rprop_steps)(i,j) = min ((*rprop_steps)(i,j), RPROP_MAX_ALLOWED_STEP); //max sure it doesn't go too high

			} else if (dMul < 0.0) //different sign
			{
				(*rprop_steps)(i,j) *= RPROP_DECREASE_STEP_FACTOR;
				(*rprop_steps)(i,j) = max ((*rprop_steps)(i,j), RPROP_MIN_ALLOWED_STEP); //max sure it doesn't go too high

			} else // dMul == 0.0 (currently same as > 0 )
			{
				(*rprop_steps)(i,j) *= RPROP_INCREASE_STEP_FACTOR;
				(*rprop_steps)(i,j) = min ((*rprop_steps)(i,j), RPROP_MAX_ALLOWED_STEP); //max sure it doesn't go too high

			}

			(*weights)(i,j) += (*rprop_steps)(i,j);
		}
	}

}

void NeuralNetwork::FinishMiniBatch(double dLearnFactor, double dViscosity)
{	
	
	// added averaging of the accumulated mini batch weight updates

	if (g_dbg)
	{
		LOG("*** FinishMiniBatch ***\n");
	}

	////////////////////////////////////////////////////////
	// Average all of the accumulated mini-batches deltas 
	
	(*m_pHiddenLayer_First_MiniBatch_Accumulated) *= 1.0 / MINI_BATCH_SIZE;//average it

	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		(*m_ppNextHiddenLayers_MiniBatch_Accumulated[l]) *= 1.0 / MINI_BATCH_SIZE;//average it		
	}

	(*m_pOutputLayer_MiniBatch_Accumulated) *= 1.0 / MINI_BATCH_SIZE;//average it	


#ifdef USE_MOMENTUM
	/////////////////////
	// Apply "momentum"
	/////////////////////

	(*m_pHiddenLayer_First_MiniBatch_Accumulated).noalias() += (*m_pHiddenLayer_First_Prev);


	// next hidden layers

	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{		
		(*m_ppNextHiddenLayers_MiniBatch_Accumulated[l]).noalias() += (*m_ppNextHiddenLayers_Prev[l]);
	}

	// output hidden layer

	(*m_pOutputLayer_MiniBatch_Accumulated).noalias() += (*m_pOutputLayer_Prev);

#endif

#ifdef USE_GRADIENT_DESCEND
	/////////////////////////////////////////////////////////////////////////////////////////////
	// Updated the neural network weights by adding the (scaled) averaged mini batch gradients

	// first hidden layer

	(*m_pHiddenLayer_First).noalias() += (*m_pHiddenLayer_First_MiniBatch_Accumulated) * dLearnFactor;

	// next hidden layers
	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		(*m_ppNextHiddenLayers[l]).noalias() += (*m_ppNextHiddenLayers_MiniBatch_Accumulated[l]) * dLearnFactor;
	}
	
	// output layer
	(*m_pOutputLayer).noalias() += (*m_pOutputLayer_MiniBatch_Accumulated) * dLearnFactor;
#endif	


#ifdef USE_RPROP

	// update the weight update factors, according to the change (or lack of change) in the dervivate sign
	// TODO: optimize it into matrix/array Eigen lib operations!
	
	NonOptimizedRPropUpdate(
		m_pHiddenLayer_First_MiniBatch_Accumulated,
		m_pHiddenLayer_First_Prev,
		m_pHiddenLayer_First_RpropStep,
		m_pHiddenLayer_First);

	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		NonOptimizedRPropUpdate(
			m_ppNextHiddenLayers_MiniBatch_Accumulated[l],
			m_ppNextHiddenLayers_Prev[l],
			m_ppNextHiddenLayers_RpropStep[l],
			m_ppNextHiddenLayers[l]);
	}

	NonOptimizedRPropUpdate(
		m_pOutputLayer_MiniBatch_Accumulated,
		m_pOutputLayer_Prev,
		m_pOutputLayer_RpropStep,
		m_pOutputLayer);


	/*
	//(*m_pHiddenLayer_First).array().

	//update the weight matrices 

	(*m_pHiddenLayer_First).noalias() += (*m_pHiddenLayer_First_RpropStep);

	// next hidden layers
	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		(*m_ppNextHiddenLayers[l]).noalias() += (*m_ppNextHiddenLayers_RpropStep[l]);
	}
	
	// output layer
	(*m_pOutputLayer).noalias() += (*m_pOutputLayer_RpropStep);
	*/


#endif

	////////////////////////////////////////////////////////////
	// store curret value to be used in the next iteration
	// If momentum leaning method is used, we damp the value.
	////////////////////////////////////////////////////////////	

	(*m_pHiddenLayer_First_Prev).noalias() = (*m_pHiddenLayer_First_MiniBatch_Accumulated) MUL_WITH_VISCOSITY; // damp it

	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		(*m_ppNextHiddenLayers_Prev[l]).noalias() = (*m_ppNextHiddenLayers_MiniBatch_Accumulated[l]) MUL_WITH_VISCOSITY; // damp it	
	}

	(*m_pOutputLayer_Prev).noalias() = (*m_pOutputLayer_MiniBatch_Accumulated) MUL_WITH_VISCOSITY; // damp it	

}



void NeuralNetwork::PrepareForMiniBatch()
{
	if (g_dbg)
	{
		LOG("*** PrepareForMiniBatch ***\n");
	}
	
	
	///////////////////////////////////////////////
	// zero the mini-batch accumulation matrices
	
	// first hidden layer
	
	(*m_pHiddenLayer_First_MiniBatch_Accumulated) = Matrix<double, Dynamic, Dynamic >::Zero(
		m_pHiddenLayer_First_MiniBatch_Accumulated->rows(),
		m_pHiddenLayer_First_MiniBatch_Accumulated->cols()
		);

	// next hidden layers

	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{		
		(*m_ppNextHiddenLayers_MiniBatch_Accumulated[l]) = Matrix<double, Dynamic, Dynamic >::Zero(
			m_ppNextHiddenLayers_MiniBatch_Accumulated[l]->rows(),
			m_ppNextHiddenLayers_MiniBatch_Accumulated[l]->cols()
			);
	}

	// output hidden layer

	(*m_pOutputLayer_MiniBatch_Accumulated) = Matrix<double, Dynamic, Dynamic >::Zero(
			m_pOutputLayer_MiniBatch_Accumulated->rows(),
			m_pOutputLayer_MiniBatch_Accumulated->cols()
			);
	
			

	/*
	if (g_dbg)
	{
		LOG("*** Velocity ***\n");
		log_Matrix("Velocity Output BEFORE\n",m_pOutputLayer_Prev);
	}
	

	(*m_pOutputLayer_Prev) += (*m_pOutputLayer_MiniBatch_Accumulated)*MOMENTUM_VISCOSITY;

	if (g_dbg)
	{
		log_Matrix("Velocity Output AFTER\n",m_pOutputLayer_Prev);
	}	

	if (g_dbg)
	{
		log_Matrix("Velocity Hidden First Layer BEFORE\n",m_pHiddenLayer_First_Prev);
	}

	(*m_pHiddenLayer_First_Prev) += (*m_pHiddenLayer_First_MiniBatch_Accumulated)*MOMENTUM_VISCOSITY  ;// update velocity

	if (g_dbg)
	{
		log_Matrix("Velocity Hidden First Layer AFTER\n",m_pHiddenLayer_First_Prev);
	}

		
	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		if (g_dbg)
		{	
			LOG("Velocity Hidden Layer %d\n", l);
		}

		if (g_dbg)
		{
			log_Matrix("Velocity Hidden First Layer BEFORE\n",m_ppNextHiddenLayers_Prev[l]);
		}

		(*m_ppNextHiddenLayers_Prev[l]) += (*m_ppNextHiddenLayers_MiniBatch_Accumulated[l])*MOMENTUM_VISCOSITY  ;// update velocity

		if (g_dbg)
		{
			log_Matrix("Velocity Hidden First Layer AFTER\n",m_ppNextHiddenLayers_Prev[l]);
		}
		
	}

	*/
}




void NeuralNetwork::CalculateDeltaValsForBackProp(double* pTargetOutputs)
{
	assert(pTargetOutputs);
	// start with the output layer, moving backwards

	for (int i=0;i<m_OutputsNum;i++)
	{
		//calculate the delta term for the output layer
		// o means the output layer
		// O means the final output value
		// T means the desired value
		// Delta(o) = O(o) * [1 - O(o)] * [O(o) - T(o)]
		(*m_pvFinalOutput_DeltaVals)[i] = (*m_pvFinalOutput)[i] * (1.0 - (*m_pvFinalOutput)[i]) * ((*m_pvFinalOutput)[i] - pTargetOutputs[i]);		
	}

	// since the bias term is always 1.0 (to allow thresholding tweaking to be done by the NN itself)
	// Delta(o) = O(o) * [1 - O(o)]
	(*m_pvFinalOutput_DeltaVals)[m_OutputsNum] = (*m_pvFinalOutput)[m_OutputsNum] * (1.0 - (*m_pvFinalOutput)[m_OutputsNum]);

	// start with the last hidden layer and move backwards

	for (int l=m_HiddenLayersNum-1;l>=0;l--)
	{
		for (int i=0; i< m_HiddenLayerSize; i++)
		{
			//calculate the delta term for node "i" in the hidden layer "l"
			// h means current hidden layer
			// n means next layer
			// the sigma runs on every node that is connected to current node
			// (has a weight that will get multipled with current node output)
			// W means weight
			// Delta(h) = O(h) * [1 - O(h)] * Sigma( Delta(n) * W(h->n)
			
			// build the sigma
			double sigma = 0.0;			
			
			if (l == m_HiddenLayersNum-1) // if it's the last hidden layer, it will directly effect the output layer
			{
				for (int j=0;j< m_OutputsNum; j++) // we have a feed forward network, every neuron in the next layer will be effected by us.
				{
					sigma += (*m_pvFinalOutput_DeltaVals)[j] * (*m_pOutputLayer)(j,i);
				}

			} else // next layer is still a hidden layer
			{
				for (int j=0;j< m_HiddenLayerSize; j++) // we have a feed forward network, every neuron in the next layer will be effected by us.
				{
					sigma += (*m_ppvHiddenLayers_DeltaVals[l+1])[j] * (*m_ppNextHiddenLayers[l])(j,i);
				}
			}
			
			(*m_ppvHiddenLayers_DeltaVals[l])[i] = (*m_ppvHiddenLayers_Result[l])[i] * (1.0 - (*m_ppvHiddenLayers_Result[l])[i]) * sigma;
						
		}

		// since the bias term is always 1.0 (to allow thresholding tweaking to be done by the NN itself)
		// Delta(o) = O(o) * [1 - O(o)]
		
		unsigned int hidden_layer_bias_index = (*m_ppvHiddenLayers_DeltaVals[l]).rows()-1;
		
		(*m_ppvHiddenLayers_DeltaVals[l])[hidden_layer_bias_index] = (*m_ppvHiddenLayers_Result[l])[hidden_layer_bias_index] * (1.0 - (*m_ppvHiddenLayers_Result[l])[hidden_layer_bias_index]);		
	}
}

void NeuralNetwork::Accumulate_BackPropBasedWeightChanges_BatchSubCase()
{
	// when updating the weights we use the following definitions:
	// o - output layer
	// h - the previous hidden layer
	// O - output (post sigmoid activation)
	// For output layer - the error is defined as Error(o) = O(h) * Delta(o)

	double dWeightChange = -6000000000.0;

	// start with the output layer, moving backwards

	// for every weight in this layer
#ifdef CHANGE_OUTPUT_LAYER_WEIGHT
	(*m_pOutputLayer_MiniBatch_Accumulated).noalias() -= (*m_pvFinalOutput_DeltaVals) * (*m_ppvHiddenLayers_Result[m_HiddenLayersNum-1]).transpose();

	// since we decreased it for the bias as well, we must add it twice to reach the desired goal of adding it once

	for (int n=0;n<m_OutputsNum;n++) // per node
	{
		//bias
		dWeightChange = (*m_pvFinalOutput_DeltaVals)[n];
		(*m_pOutputLayer_MiniBatch_Accumulated)(n,(*m_pOutputLayer_MiniBatch_Accumulated).cols()-1) += 2.0*dWeightChange;
	}

#endif
	

#ifdef CHANGE_HIDDEN_LAYER_WEIGHT
	// start with the last hidden layer and move backwards

	for (int l=m_HiddenLayersNum-1;l>=0;l--)
	{
		if (l == 0) // the first hidden layer (or the only one if there's only one)
		{
			(*m_pHiddenLayer_First_MiniBatch_Accumulated).noalias() -= (*m_ppvHiddenLayers_DeltaVals[l]) * (*m_vInputTemp).transpose();

			for (int n=0;n<m_HiddenLayerSize;n++) // per node
			{
				dWeightChange =  (*m_ppvHiddenLayers_DeltaVals[l])[n];
				(*m_pHiddenLayer_First_MiniBatch_Accumulated)(n,(*m_pHiddenLayer_First_MiniBatch_Accumulated).cols()-1) += 2.0*dWeightChange;
			}

		} else
		{
			(*m_ppNextHiddenLayers_MiniBatch_Accumulated[l-1]).noalias() -= (*m_ppvHiddenLayers_DeltaVals[l]) * (*m_ppvHiddenLayers_Result[l-1]).transpose();		

			for (int n=0;n<m_HiddenLayerSize;n++) // per node
			{
				dWeightChange =  (*m_ppvHiddenLayers_DeltaVals[l])[n];
				(*m_ppNextHiddenLayers_MiniBatch_Accumulated[l-1])(n,(*m_ppNextHiddenLayers_MiniBatch_Accumulated[l-1]).cols()-1) += 2.0*dWeightChange;
			}

		}
	}

#endif	

	
}

void NeuralNetwork::SaveToFile(const char* pFileName)
{
	FILE* f = fopen(pFileName,"wb");

	ERROR_EXIT_IF(!f, "Error: SaveToFile - failed creating file for writing: [%s]\n",pFileName);
	
	fwrite(&g_dwSeed,sizeof(unsigned int),1,f);
	fwrite(&m_InputsNum,sizeof(unsigned int),1,f);	
	fwrite(&m_HiddenLayersNum,sizeof(unsigned int),1,f);
	fwrite(&m_HiddenLayerSize,sizeof(unsigned int),1,f);
	fwrite(&m_OutputsNum,sizeof(unsigned int),1,f);

	double element;

	for (int i=0;i<m_HiddenLayerSize+1;i++)
	{
		for (int j=0;j<m_InputsNum+1;j++)
		{
			element = (*m_pHiddenLayer_First)(i,j);
			fwrite(&element,sizeof(double),1,f);
		}
	}

	for (int l=0;l<m_HiddenLayersNum-1;l++)
	{
		for (int i=0;i<m_HiddenLayerSize+1;i++)
		{
			for (int j=0;j<m_HiddenLayerSize+1;j++)
			{
				element = (*m_ppNextHiddenLayers[l])(i,j);
				fwrite(&element,sizeof(double),1,f);
			}
		}
	}

	for (int i=0;i<m_OutputsNum+1;i++)
	{
		for (int j=0;j<m_HiddenLayerSize+1;j++)
		{
			element = (*m_pOutputLayer)(i,j);
			fwrite(&element,sizeof(double),1,f);
		}
	}
	
	/*fwrite(m_pHiddenLayer_First->data(), sizeof(double),m_pHiddenLayer_First->size(),f);
	
	for (int i=0;i<m_HiddenLayersNum-1;i++)
	{
		fwrite(m_ppNextHiddenLayers[i]->data(), sizeof(double), m_ppNextHiddenLayers[i]->size(),f);
	}		
	
	fwrite(m_pOutputLayer, sizeof(double), m_pOutputLayer->size(),f);*/
	
	fclose(f);
}

