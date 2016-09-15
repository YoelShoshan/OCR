#include "Trainer_BackProp.h"
#include <SFMT.h>
#include "bmp.h"
#include <iostream>
//#include <crtdbg.h>
#include <stdio.h> 
#include <stdlib.h>
//#include <conio.h>
#include "Defines.h"
#include "NeuralNetworkUpscaling.h"
#include "NN_Executer.h"
#include "zedusOS.h"

using namespace std;

extern char g_session_dir[1024];
extern FILE* g_pLog;

//#define COL_ERROR (FOREGROUND_RED | FOREGROUND_INTENSITY)
//#define COL_SUCCESS (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
//#define COL_INFO (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

//#define CONSOLE_COLOR(x) {SetConsoleTextAttribute(g_hConsole, x);}

#define COL_ERROR
#define COL_SUCCESS
#define COL_INFO

#define CONSOLE_COLOR(x)

extern bool g_dbg;

//extern EProblemType m_problem_type;

void Trainer_BackProp::FindAllFilesInDir(const char* pPath,std::map<string,int>& files_map)
{
	assert(0);
	printf("Removed support when converting to linux! Need to restore.\n");
	return;
	
/*	unsigned int counter(0);
	bool working(true);

	string buffer;
	string dir(pPath);
	dir += "\\*.*";

	WIN32_FIND_DATA myimage;
	HANDLE myHandle=FindFirstFile(dir.c_str(),&myimage);

	if(myHandle!=INVALID_HANDLE_VALUE)
	{
		   buffer=myimage.cFileName;

		   if (buffer[0] != '.')
		   {
			 files_map[dir+buffer] = 1;
		   }

		   //fileName[counter]=buffer;

		   while(working)
		   {
				  FindNextFile(myHandle,&myimage);
				  if(myimage.cFileName!=buffer)
				  {
						 buffer=myimage.cFileName;
						 ++counter;
						 //fileName[counter]=buffer;
						 if (buffer[0] != '.')
						 {
							files_map[string(pPath)+"\\"+buffer] = 1;
						 }
				  }
				  else
				  {
						  //end of files reached
						  working=false;
				  }

		   }
	}
*/
}

/*void Trainer_BackProp::AdvancedMapIterator(IN OUT std::map<string,int>::iterator& it, IN OUT std::map<string,int>& map)
{
	if (it == map.end())
	{
		assert(0);
		MessageBox(0,"AdvancedMapIterator - it == map.end()!",0,0);
		DebugBreak();
	}

	it++;
	if (it == map.end())
	{
		it = map.begin();
	}
}*/

void Trainer_BackProp::LoadDataSets(const char* inputs_path, const char* labels_pth)
{	
	//assert(_CrtCheckMemory());

	SAFE_DEL_ARRAY(m_pHalfTestImage);

	unsigned int bPrevHalf_Width = m_HalfResImage_Width;
	unsigned int bPrevHalf_Height = m_HalfResImage_Height;

	//assert(_CrtCheckMemory());

	ERROR_EXIT_IF(m_HalfImages.size() != m_FullImages.size(), "There is a different number of files in low/high res images directories! Exiting.");
	
	m_pTrainingSet = new TrainingSet();


	switch (m_problem_type)
		{
			case PT_Upscale:

				FindAllFilesInDir(inputs_path,m_HalfImages);

				if (labels_pth)
				{
					FindAllFilesInDir(labels_pth,m_FullImages);
				}
				m_pTrainingSet->Init_Upscale(
					m_HalfImages,
					m_FullImages,
					true,
					TEST_CASES_PER_IMAGE
					);

				if (m_HalfImages.size() > 0)
				{
					//load the first one - just to get the data
					m_pHalfTestImage = LoadBMP(m_HalfImages.begin()->first.c_str(),m_HalfResImage_Width, m_HalfResImage_Height, m_HalfResImage_CompsNum);
				}
			break;

			case PT_SparseAutoEncoder:
				FindAllFilesInDir(inputs_path,m_HalfImages);

				if (labels_pth)
				{
					FindAllFilesInDir(labels_pth,m_FullImages);
				}
				m_pTrainingSet->Init_SparseAutoEncoder(
					m_HalfImages,
					true,
					TEST_CASES_PER_IMAGE
					);

				if (m_HalfImages.size() > 0)
				{
					//load the first one - just to get the data
					m_pHalfTestImage = LoadBMP(m_HalfImages.begin()->first.c_str(),m_HalfResImage_Width, m_HalfResImage_Height, m_HalfResImage_CompsNum);
				}
			break;

			case PT_Mnist:
				m_pTrainingSet->Init_MNIST(inputs_path,labels_pth);
			break;

			default:
				ERROR_EXIT_IF(1,"Unkown problem type! Supported options are [upscale/sparse_auto_encoder/mnist]");
		}

	
	

	

	//m_HalfResImage_Width = m_pTrainingSets[0]->get


	if (!m_bAllocated)
	{
		/*m_pHalfImage[Y] = new unsigned char[m_HalfResImage_Width*m_HalfResImage_Height];
		m_pHalfImage[U] = new unsigned char[m_HalfResImage_Width*m_HalfResImage_Height];
		m_pHalfImage[V] = new unsigned char[m_HalfResImage_Width*m_HalfResImage_Height];
	
		m_pUpscaledImage[Y] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];
		m_pUpscaledImage[U] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];
		m_pUpscaledImage[V] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];

		m_pUpscaledImage_NEAREST[Y] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];
		m_pUpscaledImage_NEAREST[U] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];
		m_pUpscaledImage_NEAREST[V] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];

		m_pUpscaledImage_LINEAR[Y] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];
		m_pUpscaledImage_LINEAR[U] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];
		m_pUpscaledImage_LINEAR[V] = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2];


		m_pHalfImage_Full = new unsigned char[m_HalfResImage_Width*m_HalfResImage_Height*3];
		m_pUpscaledImage_FULL = new unsigned char[m_HalfResImage_Width*2*m_HalfResImage_Height*2*3];*/


		m_pCurrentInputs = NULL;
		m_pCurrentOutputs = NULL;
		m_pDesiredOutputs = NULL;

		m_bAllocated = true;
	} else
	{
		ERROR_EXIT_IF(bPrevHalf_Width != m_HalfResImage_Width,"Incosistent image width in db frames (half image)! Exiting.");
		ERROR_EXIT_IF(bPrevHalf_Height != m_HalfResImage_Height,"Incosistent image height in db frames (half image)! Exiting.");
	}

	//SAFE_DEL(m_pTrainingSet);

	

	//assert(_CrtCheckMemory());


/*
	//////////////////////
	// Get Y',U,V images
	//////////////////////

	char image_name[512];

	m_pTrainingSet->GetImageComponent_Y(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfTestImage,
		m_pHalfImage[Y]);	
	sprintf(image_name,"%s/half_image_Y.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pHalfImage[Y],m_HalfResImage_Width,m_HalfResImage_Height,image_name);

	m_pTrainingSet->GetImageComponent_U(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfTestImage,
		m_pHalfImage[U]);
	sprintf(image_name,"%s/half_image_U.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pHalfImage[U],m_HalfResImage_Width,m_HalfResImage_Height,image_name);

	m_pTrainingSet->GetImageComponent_V(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfTestImage,
		m_pHalfImage[V]);
	sprintf(image_name,"%s/half_image_V.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pHalfImage[V],m_HalfResImage_Width,m_HalfResImage_Height,image_name);

	m_pTrainingSet->ConvertYUV_To_RGB(m_HalfResImage_Width, m_HalfResImage_Height, m_pHalfImage[Y],m_pHalfImage[U], m_pHalfImage[V], m_pHalfImage_Full);
	sprintf(image_name,"%s/half_image.bmp",g_session_dir);
	//SaveBMP_Color(m_pHalfImage_Full,m_HalfResImage_Width,m_HalfResImage_Height,image_name);
	

	////////////////////////
	// Get Nearest Images
	////////////////////////

	m_pTrainingSet->UpscaleNearest(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfImage[Y],
		m_pUpscaledImage_NEAREST[Y]);
	sprintf(image_name,"%s/half_image_Upscaled_Nearest_Y.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pUpscaledImage_NEAREST[Y],m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);

	m_pTrainingSet->UpscaleNearest(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfImage[U],
		m_pUpscaledImage_NEAREST[U]);
	sprintf(image_name,"%s/half_image_Upscaled_Nearest_U.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pUpscaledImage_NEAREST[U],m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);

	m_pTrainingSet->UpscaleNearest(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfImage[V],
		m_pUpscaledImage_NEAREST[V]);
	sprintf(image_name,"%s/half_image_Upscaled_Nearest_V.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pUpscaledImage_NEAREST[V],m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);

	m_pTrainingSet->ConvertYUV_To_RGB(m_HalfResImage_Width*2, m_HalfResImage_Height*2, m_pUpscaledImage_NEAREST[Y],m_pUpscaledImage_NEAREST[U], m_pUpscaledImage_NEAREST[V], m_pUpscaledImage_FULL);
	sprintf(image_name,"%s/upscaled_image_NEAREST.bmp",g_session_dir);
	//SaveBMP_Color(m_pUpscaledImage_FULL,m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);

	/////////////////////////////////////////////////////////////////

	////////////////////////
	// Get Linear Images
	////////////////////////

	m_pTrainingSet->UpscaleLinear(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfImage[Y],
		m_pUpscaledImage_LINEAR[Y]);
	sprintf(image_name,"%s/half_image_Upscaled_Linear_Y.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pUpscaledImage_LINEAR[Y],m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);

	m_pTrainingSet->UpscaleLinear(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfImage[U],
		m_pUpscaledImage_LINEAR[U]);
	sprintf(image_name,"%s/half_image_Upscaled_Linear_U.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pUpscaledImage_LINEAR[U],m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);

	m_pTrainingSet->UpscaleLinear(
		m_HalfResImage_Width,m_HalfResImage_Height,
		m_pHalfImage[V],
		m_pUpscaledImage_LINEAR[V]);
	sprintf(image_name,"%s/half_image_Upscaled_Linear_V.bmp",g_session_dir);
	//SaveBMP_GreyScale(m_pUpscaledImage_LINEAR[V],m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);

	m_pTrainingSet->ConvertYUV_To_RGB(m_HalfResImage_Width*2, m_HalfResImage_Height*2, m_pUpscaledImage_LINEAR[Y],m_pUpscaledImage_LINEAR[U], m_pUpscaledImage_LINEAR[V], m_pUpscaledImage_FULL);
	sprintf(image_name,"%s/upscaled_image_LINEAR.bmp",g_session_dir);
	//SaveBMP_Color(m_pUpscaledImage_FULL,m_HalfResImage_Width*2,m_HalfResImage_Height*2,image_name);


	*/

	/////////////////////////////////////////////////////////////////

	
}

void Trainer_BackProp::CreatePermutation(unsigned int size)
{
	if (!m_pPermutation || size != m_uiPermutationSize)
	{
		FPRINTF("Creating a new permutation. prev size was %d new size is %d ...\n",
			m_uiPermutationSize,
			size);

		SAFE_DEL_ARRAY(m_pPermutation);
		m_uiPermutationSize = size;
		m_pPermutation = new unsigned int[m_uiPermutationSize];

		unsigned int x1,x2,temp;

		for (int i=0;i<size;i++)
		{
			m_pPermutation[i] = i;
		}

		for (int i=0;i<size*100;i++)
		{
			x1 = SFMT_IN_RANGE_UINT(0,size);
			x2 = SFMT_IN_RANGE_UINT(0,size);

			temp = m_pPermutation[x1];
			m_pPermutation[x1] = m_pPermutation[x2];
			m_pPermutation[x2] = temp;
		}

		FPRINTF("Done creating permutation\n");
		
	}	
}


Trainer_BackProp::Trainer_BackProp(EProblemType problem_type,const char* inputs_path, const char* labels_pth)
{
	assert(inputs_path);	
	
	m_pInputsPath = NULL;
	m_pLabelsPath = NULL;

	if (inputs_path)
	{
		m_pInputsPath = new char[strlen(inputs_path)+1];
		strcpy(m_pInputsPath,inputs_path);
	}

	if (labels_pth)
	{
		m_pLabelsPath = new char[strlen(labels_pth)+1];
		strcpy(m_pLabelsPath,labels_pth);
	}	

	m_problem_type = problem_type;

	m_pPermutation = NULL;

	m_uiPermutationSize = 0;
	//CreatePermutation(1920*1200);
	m_pHalfTestImage = NULL;
	
	m_bAllocated = false;
	
	LoadDataSets(inputs_path,labels_pth);

		//m_pTrainingSet = NULL;
	m_pCurrentInputs = NULL;
	m_pCurrentOutputs = NULL;
	m_pDesiredOutputs = NULL;

	m_pNN = new NeuralNetwork(
		m_pTrainingSet->GetTestCase_Input_Width() * m_pTrainingSet->GetTestCase_Input_Height() , 
		HIDDEN_LAYERS_NUM, 
		HIDDEN_LAYER_SIZE, 
		m_pTrainingSet->GetOutputsNum());

	m_InputsNum = 1+(m_pTrainingSet->GetTestCase_Input_Width() * m_pTrainingSet->GetTestCase_Input_Height());

	//assert(_CrtCheckMemory());

	SAFE_DEL_ARRAY(m_pCurrentInputs);

	//assert(_CrtCheckMemory());

	m_pCurrentInputs = new double[m_InputsNum+1];

	//assert(_CrtCheckMemory());

	m_OutputsNum = m_pNN->m_OutputsNum; // 4 output pixels

	//assert(_CrtCheckMemory());

	SAFE_DEL_ARRAY(m_pCurrentOutputs);

	//_CrtCheckMemory();

	m_pCurrentOutputs = new double[m_OutputsNum+1];
	SAFE_DEL_ARRAY(m_pDesiredOutputs);
	m_pDesiredOutputs = new double[m_OutputsNum+1];
	

	

	//init_gen_rand(GetTickCount());	
	m_dLastImageScore = 6000000000.0;
	m_dLastBatchScore = 6000000000.0;

	m_WeightUpdateFactor = BACKPROP_WEIGHT_UPDATE_STEP;
	m_dwScoreIncreaseInARow = 0;
	m_dwScoreDecreaseInARow = 0;
	

	m_dwProcessedTestCases = 0;
	m_dwSkippedTestCases = 0;

	m_Viscosity = MOMENTUM_VISCOSITY_A;

}

/*Trainer_BackProp::Trainer_BackProp(const char* training_set_data,const char* training_set_labels)
{
	m_pTrainingSet = new TrainingSet(training_set_data, training_set_labels);
	m_InputsNum = 1+(m_pTrainingSet->GetTestCase_Input_Width() * m_pTrainingSet->GetTestCase_Input_Height());
	m_pCurrentInputs = new double[m_InputsNum];
	m_OutputsNum = 10; // 10 digits
	m_pCurrentOutputs = new double[m_OutputsNum];
	m_pDesiredOutputs = new double[m_OutputsNum];
	init_gen_rand(GetTickCount());	
	m_dLastScore = 0.0;
}*/

Trainer_BackProp::~Trainer_BackProp()
{
}

double Trainer_BackProp::Run_MultipleTestSequence(unsigned int ind_start, unsigned int ind_end, bool bEvolve, bool update_learning_rate)
{
	assert(ind_end > ind_start);
	double dImageScoreSum = 0.0;
	double dBatchScoreSum = 0.0;
	double dTestCaseScore = 0.0;

	CreatePermutation(m_pTrainingSet->GetTestCasesNum());

	// sub-batches
	unsigned int sub_batch_size = MINI_BATCH_SIZE;

	unsigned int batches_num = (ind_end-ind_start) / sub_batch_size;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// For example, if our database is at the size 1512, and batch size is at the size 500
	// we will have 3 batches of 500, so total of 1500 test cases, and we will DISCARD the last 12 test cases
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned int total_test_cases_num = batches_num * sub_batch_size;

	
	static bool s_warning_showed = false;

	if (!s_warning_showed)
	{
		if (total_test_cases_num != ind_end - ind_start)
		{
			FPRINTF("The last %d test cases will be discarded.\nA total of %d test cases will be used.\n",
				(ind_end - ind_start) - total_test_cases_num,
				total_test_cases_num);
			FPRINTF("%d batches of size %d used.\n",
				batches_num, 
				sub_batch_size);
			s_warning_showed = true;
		}

	}

	m_pNN->PrepareForMiniBatch();

	unsigned int test_cases_per_image = m_pTrainingSet->GetTestCasesNum();

	unsigned int test_case_num = 0;
	unsigned int batch_num = 0;

	for (unsigned int i=ind_start;i<ind_start+total_test_cases_num;i++)
	{
		//double dMaxError = 999999.0;
		//dTestCaseScore = ForwardPass_TestNum(i-ind_start, bEvolve/*, dMaxError*/); // start with zero

		//if (i >= 291612)
		/*if (i >= 291619) // the end of prev minibatch
		{
			g_dbg = true;
			//printf("Here!\n");
			FLUSH();
		}*/

		//image_num = SFMT_IN_RANGE_UINT(0, m_uiTrainingSetsNum);
		test_case_num = m_pPermutation[i];

		FLUSH();

		//test_case_num = (i-ind_start);// % test_cases_per_image;

		//permutated
		dTestCaseScore = ForwardPass_TestNum(test_case_num , bEvolve); // start with zero		

		dBatchScoreSum+= dTestCaseScore;
		dImageScoreSum+= dTestCaseScore;

		//FPRINTF("Score=%f\n",dTestCaseScore);

		if (g_dbg)
		{
			LOG("%d) test case %d Score=%f 0x%08X%08X\n", i,test_case_num, dTestCaseScore,
				((unsigned int*) &dTestCaseScore)[0],
				((unsigned int*) &dTestCaseScore)[1]
				);
		}

		//m_dwProcessedTestCases++;		 
		//double dSkipThreshold = 0.0; //only skip perfect (should be usually nothing)

		//assert(_CrtCheckMemory());

		if (bEvolve/* && dMaxError>dSkipThreshold*/)
		{
			CalcCaseDeltas();

			AccumulateDeltaChange_SubCase();
		} else
		{
			m_dwSkippedTestCases++;
		}

		if (i%sub_batch_size == sub_batch_size-1)
		{
			m_pNN->FinishMiniBatch(m_WeightUpdateFactor, m_Viscosity);

			batch_num++;

			double progress = double(i-ind_start) / double(total_test_cases_num);


			if (batch_num % 500 == 0)
			{
				std::cout << "\r";
				printf("%f%%",  progress*100.f);
			}

			if (batch_num == batches_num)
			{
				std::cout << "\r";
				printf("            ");
			}

			/*ChooseNextWeightFactor(dBatchScoreSum, m_dLastBatchScore);
			m_dLastBatchScore = dBatchScoreSum;
			dBatchScoreSum = 0;*/

			m_pNN->PrepareForMiniBatch();

			//FPRINTF("finished mini batch - i==%d\n", i);
		}
	}
	
	printf("\n");

	//assert(_CrtCheckMemory());

	if (update_learning_rate)
	{
		ChooseNextWeightFactor(dBatchScoreSum, m_dLastBatchScore);
	}
	m_dLastBatchScore = dBatchScoreSum;
	dBatchScoreSum = 0;

	m_pNN->BackupCurrentNN();

	LOG("Finished image training.\n");	
	
	return dImageScoreSum;
}

extern bool g_dbg;

double Trainer_BackProp::ForwardPass_Raw(double* inputs_raw, double* outputs)
{
	//assert(_CrtCheckMemory());

	memcpy(m_pCurrentInputs, inputs_raw, sizeof(double) * m_pTrainingSet->GetTestCase_Input_Width() * m_pTrainingSet->GetTestCase_Input_Height());
	m_pCurrentInputs[m_pTrainingSet->GetTestCase_Input_Width() * m_pTrainingSet->GetTestCase_Input_Height()] = -1.0; //for bias

	
	//assert(_CrtCheckMemory());

	m_pNN->Forward_Pass(
		m_InputsNum,
		m_pCurrentInputs,
		m_OutputsNum,
		m_pCurrentOutputs);

	if (g_dbg)
	{
		for (int i=0;i<m_OutputsNum;i++)
		{
			LOG("current output %d: %f\n", i, m_pCurrentOutputs[i]);
		}
	}

	//assert(_CrtCheckMemory());

	memcpy(m_pDesiredOutputs, outputs, sizeof(double) * m_OutputsNum);

	m_pNN->m_dScore = 0.0;

	//max_error = -99999.0;

	if (g_dbg)
	{
		for (int i=0;i<m_OutputsNum;i++)
		{
			LOG("BEFORE desired output %d: %f\n", i, m_pDesiredOutputs[i]);
		}
	}


	for (int i=0;i<m_OutputsNum;i++)
	{
		m_pNN->m_dScore+= (m_pDesiredOutputs[i] - m_pCurrentOutputs[i]) * (m_pDesiredOutputs[i] - m_pCurrentOutputs[i]);
	}

	if (g_dbg)
	{
		for (int i=0;i<m_OutputsNum;i++)
		{
			LOG("AFTER desired output %d: %f\n", i, m_pDesiredOutputs[i]);			
		}
	}

	return m_pNN->m_dScore;
}

void Trainer_BackProp::CalcCaseDeltas()
{
	m_pNN->CalculateDeltaValsForBackProp(m_pDesiredOutputs);	
}	


void Trainer_BackProp::AccumulateDeltaChange_SubCase()
{ 
	m_pNN->Accumulate_BackPropBasedWeightChanges_BatchSubCase();
}

double Trainer_BackProp::ForwardPass_TestNum(unsigned int test_num, bool bEvolve)
{
	unsigned int inputs_num = -1;
	double* inputs_raw = NULL;
	double* outputs = NULL;
	m_pTrainingSet->GetTestCase(test_num,inputs_raw,outputs);

	//assert(_CrtCheckMemory());	

	// OCR
	//unsigned char correct_answer;		
	//m_pTrainingSet->GetLabel(test_num,correct_answer);		

	double dScore = ForwardPass_Raw(inputs_raw,outputs);

	//LOG("Case num %d score=%f\n", test_num, dScore);

	//assert(_CrtCheckMemory());
	
	return dScore;
}

double Trainer_BackProp::Run_SingleRandomTest()
{
	assert(0); // didn't develop this function - might be outdated.
	return -1.f;
}

double Trainer_BackProp::LoadFromFile(const char* org_file, bool bCalcScore, char* out_image_file)
{
	SAFE_DEL(m_pNN);
	m_pNN = new NeuralNetwork(org_file);
	//m_pNN->LoadFromFile(org_file);

	// use the first image as the showcase for the loaded NN

	if (out_image_file)
	{
		NN_Executer::OutputResult_Upscaling(m_pNN,m_HalfImages.begin()->first.c_str(),out_image_file);
	} else
	{
		char result_image[512];
		if (PT_Upscale == m_problem_type)
		{
			sprintf(result_image,"%s/PID_0x%08X - loaded_org",g_session_dir,zosGetProcessID());
			NN_Executer::OutputResult_Upscaling(m_pNN,m_HalfImages.begin()->first.c_str(),result_image);
		} else if (PT_SparseAutoEncoder == m_problem_type)
		{
			sprintf(result_image,"%s/PID_0x%08X - loaded_org",g_session_dir,zosGetProcessID());
			NN_Executer::OutputResult_SparseAutoEncoder(m_pNN,m_HalfImages.begin()->first.c_str(),result_image);
		}
		else if (PT_Mnist == m_problem_type)
		{						
			sprintf(result_image,"%s/PID_0x%08X - loaded_org.AI_RESULT",g_session_dir,zosGetProcessID());			
			NN_Executer::OutputResult_Mnist(m_pNN, m_pInputsPath, m_pLabelsPath, result_image);
		}
		
		/*sprintf(result_image,"%s/PID_0x%08X - loaded_org",g_session_dir,zosGetProcessID());
		NN_Executer::OutputResult_Upscaling(m_pNN,m_HalfImages.begin()->first.c_str(),result_image);*/
	}

	double score = 0.0;

	if (bCalcScore)
	{
		score = Run_MultipleTestSequence(0,m_pTrainingSet->GetTestCasesNum(), false, false);
	}
	
	return score;
}

void Trainer_BackProp::Evolve()
{
	FPRINTF("Starting training.\n");
	FPRINTF("Used methods:\n");
#ifdef USE_GRADIENT_DESCEND
	FPRINTF("Gradient Descend.\n");
#endif

#ifdef USE_MOMENTUM
	FPRINTF("Momentum.\n");
#endif

#ifdef USE_RPROP
	FPRINTF("RPROP.\n");
#endif


	///////////////////////////////////////////////
	// Timing related
/*	static double s_dFreq = 0;
	static bool s_inited_freq = false;
	static LARGE_INTEGER s_prev_time;

	if (!s_inited_freq)
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		s_dFreq = double(freq.QuadPart);

		s_inited_freq = true;

		QueryPerformanceCounter(&s_prev_time);
	}*/
	///////////////////////////////////////////////	
	double score = 60000000.0;
		

	for (int i=0;;i++)
	{
		//score = Run_MultipleRandomTest();
		//score = Run_MultipleTestSequence(0,5000);

		unsigned int test_cases_num = m_pTrainingSet->GetTestCasesNum();

#ifdef ONLY_FEW_TEST_CASES_DEBUG
		test_cases_num = 100;
#endif
	
		score = Run_MultipleTestSequence(0,test_cases_num, true, true);

		FPRINTF("Tested %d cases.\n", test_cases_num);

		/*FPRINTF("\-- [%s]->[%s]\n", 
			m_HalfImages_Pos->first.c_str(),
			m_FullImages_Pos->first.c_str()
			);*/

		if (i >= 100)
		{
			m_Viscosity = MOMENTUM_VISCOSITY_A;
			m_Viscosity+= (MOMENTUM_VISCOSITY_B - MOMENTUM_VISCOSITY_A) * (double(i) - 100.0) * 0.001;

			if (i==100)
			{
				CONSOLE_COLOR(COL_SUCCESS);
				FPRINTF("Starting to increase viscosity.\n");
				CONSOLE_COLOR(COL_INFO);
			}
		}

		///////////////////////////////////////////////
		// Timing related
/*		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		double dTime = double(now.QuadPart - s_prev_time.QuadPart) / s_dFreq;*/
		///////////////////////////////////////////////
		
		FPRINTF("Iter %d) Score = %f. [%f%% change]\t[weight change factor=%f] [Viscosity=%f]\n\n", 
			i, 
			score,
			((score-m_dLastImageScore) / m_dLastImageScore)*100.0,
			m_WeightUpdateFactor,
			m_Viscosity);
		//SaveCurrentNNToFile(score,dTime/(60.0*60.0));
		SaveCurrentNNToFile(score);

		m_dLastImageScore = score;

		/*
		//if (i%SAME_IMAGE_IN_A_ROW_NUM == SAME_IMAGE_IN_A_ROW_NUM-1)
		{
			AdvancedMapIterator(m_HalfImages_Pos, m_HalfImages);
			AdvancedMapIterator(m_FullImages_Pos, m_FullImages);
	
			LoadCurrentImage();
		}
		*/
	}
}

void Trainer_BackProp::ChooseNextWeightFactor(double dCurrentScore, double dPrevScore)
{
	//debug
	//return;	
	

	double dChange_Increase = 1.11 + SFMT_IN_RANGE(0.00,0.01);
	double dChange_Decrease = 1.11 + SFMT_IN_RANGE(0.00,0.01);
	double dMax_Weight = 7.0;


	//unsigned int required_improvs = 30;
	unsigned int required_improvs = 30;
	unsigned int required_worse_iterations = 1;

	if (dCurrentScore < dPrevScore) // if the score improved
	{
		m_dwScoreDecreaseInARow++;		
		m_dwScoreIncreaseInARow = 0;

		if (m_dwScoreDecreaseInARow >= required_improvs && m_WeightUpdateFactor < dMax_Weight)
		{
			m_WeightUpdateFactor *= dChange_Increase;

			CONSOLE_COLOR(COL_SUCCESS);	
			FPRINTF("%d improvements in a row met! increasing weight update factor size to %f!\n",
				required_improvs,m_WeightUpdateFactor);
			CONSOLE_COLOR(COL_INFO);

			//m_dwScoreDecreaseInARow = 0.0;
		}
	} else	
	{		
		/*CONSOLE_COLOR(COL_ERROR);	
		if (dPrevScore < dCurrentScore)
		{
			FPRINTF("After %d improvements in a row score got worse.\n",
				m_dwScoreDecreaseInARow);
		} else
		{
			FPRINTF("After %d improvements in a row score remained the same.\n",
				m_dwScoreDecreaseInARow);
		}*/

		m_dwScoreIncreaseInARow++;
		m_dwScoreDecreaseInARow = 0;	

		if (m_dwScoreIncreaseInARow >= required_worse_iterations)
		{			
			m_WeightUpdateFactor /= dChange_Decrease;
			CONSOLE_COLOR(COL_ERROR);	
			FPRINTF("Decreasing weight update factor to %f\n", m_WeightUpdateFactor);
			CONSOLE_COLOR(COL_INFO);

			m_pNN->RestoreBackup();
			CONSOLE_COLOR(COL_ERROR);	
			FPRINTF("Restoring NN from backup.\n");
			CONSOLE_COLOR(COL_INFO);
		}
		
		//CONSOLE_COLOR(COL_INFO);
	}



}

/*void Trainer_BackProp::ChooseNextWeightFactor(double dCurrentScore, double dPrevScore)
{
	//debug
	//return;

	bool bScoreGotWorse = false;

	m_dwIterationsWithoutScoreDecrease++;
	m_dwIterationsWithoutScoreDecrease--;

	if (dCurrentScore > dPrevScore)
	{
		bScoreGotWorse = true;
		m_dwIterationsWithoutScoreDecrease = 0;
	} else if (dCurrentScore < dPrevScore)
	{
		m_dwIterationsWithoutScoreIncrease = 0;
	}		

	//if (bScoreGotWorse)
	//{
		//FPRINTF("--- \n");
	//} else
	//{
		//FPRINTF("   +++ \n");
	//}

	double dChange_Increase = 1.11 + SFMT_IN_RANGE(0.00,0.01);
	double dChange_Decrease = 1.11 + SFMT_IN_RANGE(0.00,0.01);
	if (dCurrentScore < dPrevScore) // if the score improved
	{
		if (m_dwIterationsWithoutScoreDecrease > 10)
		{
		 	m_WeightUpdateFactor *= dChange_Increase;

			//FPRINTF("+ In-creased to %f [%f]->[%f]\n",m_WeightUpdateFactor, dPrevScore,dCurrentScore);

			m_dwIterationsWithoutScoreDecrease = 0;
		} else
		{
			//FPRINTF("+ .\n");
		}
	} else
	{
		if (m_dwIterationsWithoutScoreDecrease > 10)
		{
			m_WeightUpdateFactor /= dChange_Decrease;

			//FPRINTF("- De-creased to %f  [%f]->[%f]\n",m_WeightUpdateFactor, dPrevScore,dCurrentScore);
		} else
		{
			//FPRINTF("- .\n");
		}

		
	}

}*/




void Trainer_BackProp::SaveCurrentNNToFile(double dScore/*, double dTime*/)
{
	static int winner_num = -1;
	winner_num++;

/*	if (winner_num < 2000)
	{
		if (winner_num%100 != 0)
			return;
	} else*/
	{
		if (winner_num%1000 != 1)
			return;
	}

	char file_name[1024];
//	sprintf(file_name,"%s/session_winner_PID_%d_%04d_score_%.3f_%.4f_In_%d_Hidden_%d_Hidden_Size_%d_Output_%d.org",
	sprintf(file_name,"%s/session_winner_PID_%d_%04d_score_%.3f_In_%d_Hidden_%d_Hidden_Size_%d_Output_%d.org",
		g_session_dir,zosGetProcessID(),winner_num,dScore,/*dTime,*/
		m_pNN->m_InputsNum,
		m_pNN->m_HiddenLayersNum,
		m_pNN->m_HiddenLayerSize,
		m_pNN->m_OutputsNum
		);
	m_pNN->SaveToFile(file_name);		

	//save an example result image

	if (PT_Upscale == m_problem_type)
	{
		sprintf(file_name,"%s/session_winner_PID_%d_%04d_%.3f_.bmp",g_session_dir,zosGetProcessID(),winner_num,dScore/*,dTime*/);
		NN_Executer::OutputResult_Upscaling(m_pNN,m_HalfImages.begin()->first.c_str(),file_name);
	} else if (PT_SparseAutoEncoder == m_problem_type)
	{
		sprintf(file_name,"%s/session_winner_PID_%d_%04d_%.3f_.bmp",g_session_dir,zosGetProcessID(),winner_num,dScore/*,dTime*/);
		NN_Executer::OutputResult_SparseAutoEncoder(m_pNN,m_HalfImages.begin()->first.c_str(),file_name);
	}
	else if (PT_Mnist == m_problem_type)
	{
		sprintf(file_name,"%s/session_winner_PID_%d_%04d_%.3f_.AI_RESULT",g_session_dir,zosGetProcessID(),winner_num,dScore/*,dTime*/);
		NN_Executer::OutputResult_Mnist(m_pNN, m_pInputsPath, m_pLabelsPath, file_name);

	}

	
}
