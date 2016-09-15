#ifndef TRAINER_BACK_PROPAGATION_H
#define TRAINER_BACK_PROPAGATION_H

#include "NeuralNetwork.h"
#include "Training_Set.h"
#include "NeuralNetworkUpscaling.h"


#include <vector>
#include <map>

class Trainer_BackProp
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Trainer_BackProp(EProblemType problem_type, const char* inputs_path, const char* labels_pth);
	virtual ~Trainer_BackProp();

	void LoadDataSets(const char* inputs_path, const char* labels_pth);

	void Evolve();

	double ForwardPass_TestNum(unsigned int test_num, bool bEvolve);

	double Run_SingleRandomTest();	
	double Run_MultipleTestSequence(unsigned int ind_start, unsigned int ind_end, bool bEvolve, bool update_learning_rate);

	double LoadFromFile(const char* org_file, bool bCalcScore, char* out_image_file);

private:

	void FindAllFilesInDir(const char* pPath, std::map<string,int>& files_map);
	
	double ForwardPass_Raw(double* inputs_raw, double* outputs);

	void CalcCaseDeltas();

	void AccumulateDeltaChange_SubCase();

	void SaveCurrentNNToFile(double dScore/*, double dTime*/);
		
	void ChooseNextWeightFactor(double dCurrentScore, double dPrevScore);

	//void AdvancedMapIterator(OUT std::map<string,int>::iterator& it, OUT std::map<string,int>& map);

	//size means 
	void CreatePermutation(unsigned int size);
	
	EProblemType m_problem_type;

	NeuralNetwork* m_pNN;

	//TrainingSet** m_pTrainingSets;
	TrainingSet*m_pTrainingSet;

	//TrainingSet* m_pTrainingSet;

	unsigned int m_uiPermutationSize;
	unsigned int* m_pPermutation;

	unsigned int m_InputsNum;
	double* m_pCurrentInputs;
	
	unsigned int m_OutputsNum;
	double* m_pCurrentOutputs;
	double* m_pDesiredOutputs;

	double m_dLastImageScore;
	double m_dLastBatchScore;

	// test image
	unsigned char* m_pHalfTestImage;
	unsigned int m_HalfResImage_Width;
	unsigned int m_HalfResImage_Height;
	unsigned int m_HalfResImage_CompsNum;

	char* m_pInputsPath;
	char* m_pLabelsPath;
			
	/*unsigned char* m_pUpscaledImage[3];	 // for outputs of NN	
	unsigned char* m_pHalfImage[3];	 // for converted lowres image (converted into YVU)
	unsigned char* m_pHalfImage_Full;
	unsigned char* m_pUpscaledImage_NEAREST[3]; // for outputs of nearest
	unsigned char* m_pUpscaledImage_LINEAR[3]; // for outputs of nearest	
	unsigned char* m_pUpscaledImage_FULL;*/


	/////////////////////////////
	// learning parameters

	double m_WeightUpdateFactor;
	double m_Viscosity; // for momentum

	unsigned int m_dwScoreIncreaseInARow;
	unsigned int m_dwScoreDecreaseInARow;
	

	//debugging

	unsigned int m_dwProcessedTestCases;
	unsigned int m_dwSkippedTestCases;

	std::map<string,int> m_HalfImages;
	std::map<string,int> m_FullImages;

	//std::map<string,int>::iterator m_HalfImages_Pos;
	//std::map<string,int>::iterator m_FullImages_Pos;

	bool m_bAllocated;

};





#endif
