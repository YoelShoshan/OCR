#include "Training_Set.h"
#include "stdio.h"
#include <assert.h>
#include "Defines.h"
#include <SFMT.h>
#include "bmp.h"
#include "NN_Executer.h"
#include "zedusOS.h"

extern char g_session_dir[1024];
extern FILE* g_pLog;

using namespace std;

TrainingSet::TrainingSet()
{
	m_bInitialized = false;
	//m_pInputCases = NULL;
	m_pInputCases_Double = NULL;
	m_labels_per_entry = 0;
	m_pLabels_Double = NULL;
	m_TestCasesNum = 0;
	m_TestCase_Input_Width = 0;
	m_TestCase_Input_Height = 0;
	m_InputSize = 0;
}


bool TrainingSet::Init_SparseAutoEncoder(
	map<string,int>& images,
	bool randomize_order,
	unsigned int requested_test_cases_num_per_image)
{	
	m_TestCase_Input_Width = 5;
	m_TestCase_Input_Height = 5;

	m_labels_per_entry = m_TestCase_Input_Width*m_TestCase_Input_Height; //we want to output the same amount of data as we got as input

	m_InputSize = m_TestCase_Input_Width*m_TestCase_Input_Height;
	
	m_src_width = -1;
	m_src_height = -1;
	m_src_comps_num = 0;

	// convert to luma, train only luma change for now
	if (0==requested_test_cases_num_per_image)
	{
		m_TestCasesNum = images.size()*(m_src_width-4)*(m_src_height-4);//EPOCH_TEST_CASES; 
	} else
	{
		m_TestCasesNum = images.size()*requested_test_cases_num_per_image;
	}

	// every input is 5x5 kernel around a pixel in the half resolution image
	m_pInputCases_Double = new double[m_TestCasesNum*m_InputSize];
	m_pLabels_Double = new double[m_TestCasesNum*m_labels_per_entry];

	std::map<string,int>::iterator it_half = images.begin();
	unsigned int ts_ind = 0;
	bool bInitedTrainingSet = false;

	m_pInputTravel = m_pInputCases_Double;
	m_pOutputTravel = m_pLabels_Double;

	while(it_half!= images.end())
	{
		AddSingleImage_PassThrough(
			it_half->first.c_str(),
			true,
			requested_test_cases_num_per_image);
		
		it_half++;
		ts_ind++;

		if (ts_ind%10 == 9)
		{
			FPRINTF("%d images loaded\n",ts_ind+1);
		}
	}
		
	return true;
}

bool TrainingSet::Init_Upscale(
	map<string,int>& low_res_images,//const char* half_res_image, 
	map<string,int>& high_res_images,//const char* full_res_image, 
	bool randomize_order,
	unsigned int requested_test_cases_num_per_image)
{
	m_labels_per_entry = 4;

	m_TestCase_Input_Width = 5;
	m_TestCase_Input_Height = 5;

	m_InputSize = m_TestCase_Input_Width*m_TestCase_Input_Height;
	
	m_src_width = -1;
	m_src_height = -1;
	m_src_comps_num = 0;

	ERROR_EXIT_IF(low_res_images.size() != high_res_images.size(),"Different number of images in the low res directory and high res directory! Exiting.");

	// convert to luma, train only luma change for now
	if (0==requested_test_cases_num_per_image)
	{
		m_TestCasesNum = low_res_images.size()*(m_src_width-4)*(m_src_height-4);//EPOCH_TEST_CASES; 
	} else
	{
		m_TestCasesNum = low_res_images.size()*requested_test_cases_num_per_image;
	}

	// every input is 5x5 kernel around a pixel in the half resolution image
	m_pInputCases_Double = new double[m_TestCasesNum*m_InputSize];
	m_pLabels_Double = new double[m_TestCasesNum*m_labels_per_entry];

	std::map<string,int>::iterator it_half = low_res_images.begin();
	std::map<string,int>::iterator it_full = high_res_images.begin();
	unsigned int ts_ind = 0;
	bool bInitedTrainingSet = false;

	m_pInputTravel = m_pInputCases_Double;
	m_pOutputTravel = m_pLabels_Double;

	while(it_half!= low_res_images.end())
	{
		AddSingleImage_Upscale(
			it_half->first.c_str(),
			it_full->first.c_str(),
			true,
			requested_test_cases_num_per_image);
		
		it_half++;
		it_full++;
		ts_ind++;

		if (ts_ind%10 == 9)
		{
			FPRINTF("%d images loaded\n",ts_ind+1);
		}
	}
		
	return true;
}


bool TrainingSet::AddSingleImage_PassThrough(
		const char* image,
		bool randomize_order,
		unsigned int requested_test_cases_num)
{
	unsigned char* data = LoadBMP(image, m_src_width, m_src_height, m_src_comps_num);

	ERROR_IF(!requested_test_cases_num, "AddSingleImage_Upscale - requested_test_cases_num==0 is not allowed! Exiting.");
	
	ERROR_RETURN_FALSE_IF(m_src_comps_num != 3,"Only 3 components (bgr) images are supported!");

	ERROR_RETURN_FALSE_IF(!data, "Can't load input image!");
	
	ERROR_EXIT_IF(!data, "Error: no low resolution image provided! Exiting.");
	
	m_dest_width = -1;
	m_dest_height = -1;
	m_dest_comps_num = 0;

	m_bInitialized = true;	

	int comp_num = m_src_comps_num;		

	for (int i=0;i<requested_test_cases_num;i++)
	{
		//debug
		unsigned int rand_x;
		unsigned int rand_y;

		if (randomize_order)
		{
			rand_x = SFMT_IN_RANGE_UINT(2,m_src_width-3);
			rand_y = SFMT_IN_RANGE_UINT(2,m_src_height-3);
		} else
		{
			rand_x = (i%(m_src_width-4))+2;
			rand_y = (i/(m_src_width-4))+2;
		}

		NN_Executer::Get_Kernel_Luma(data,
					rand_x,rand_y,
					m_src_width,m_src_height,m_src_comps_num,
					m_pInputTravel); //returns inputs in rand [-1.0, 1.0] as a NN would expect
		
		//memcpy(m_pOutputTravel,m_pInputTravel, sizeof(double)*m_TestCase_Input_Width*m_TestCase_Input_Height);
		for (int o=0;o<m_TestCase_Input_Width*m_TestCase_Input_Height;o++)
		{
			//move from [-1.0,1.0] to [0.0,1.0] - the Neuron output range.
			m_pOutputTravel[o] = (m_pInputTravel[o]+1.0)*0.5;
		}

		
		m_pInputTravel+= m_TestCase_Input_Width*m_TestCase_Input_Height;
		m_pOutputTravel+= m_TestCase_Input_Width*m_TestCase_Input_Height;
	}

	SAFE_DEL_ARRAY(data);
}

bool TrainingSet::AddSingleImage_Upscale(
		const char* low_res_image,
		const char* high_res_image,
		bool randomize_order,
		unsigned int requested_test_cases_num)
{
	unsigned char* low_res_data = LoadBMP(low_res_image, m_src_width, m_src_height, m_src_comps_num);

	ERROR_IF(!requested_test_cases_num, "AddSingleImage_Upscale - requested_test_cases_num==0 is not allowed! Exiting.");
	
	ERROR_RETURN_FALSE_IF(m_src_comps_num != 3,"Only 3 components (bgr) images are supported!");

	ERROR_RETURN_FALSE_IF(!low_res_data, "Can't load input image!");
	
	ERROR_EXIT_IF(!low_res_image, "Error: no low resolution image provided! Exiting.");
	ERROR_EXIT_IF(!high_res_image, "Error: no high resolution image provided! Exiting.");
	
	m_dest_width = -1;
	m_dest_height = -1;
	m_dest_comps_num = 0;
	unsigned char* high_res_data = LoadBMP(high_res_image, m_dest_width,m_dest_height,m_dest_comps_num);

	ERROR_RETURN_FALSE_IF(m_dest_comps_num != 3,"Only 3 components (bgr) images are supported!");
		
	ERROR_RETURN_FALSE_IF(!high_res_data,"Can't load output image!.");

	ERROR_RETURN_FALSE_IF(m_src_comps_num != m_dest_comps_num,"Components num different between input and output images!.");

	ERROR_RETURN_FALSE_IF(
		! ((m_src_width == m_dest_width/2) && (m_src_height == m_dest_height/2)),
		"Output image must be twice bigger than input image in every dimension!."
		);

	m_bInitialized = true;	

	int comp_num = m_src_comps_num;		

	for (int i=0;i<requested_test_cases_num;i++)
	{
		//debug
		unsigned int rand_x;
		unsigned int rand_y;

		if (randomize_order)
		{
			rand_x = SFMT_IN_RANGE_UINT(2,m_src_width-3);
			rand_y = SFMT_IN_RANGE_UINT(2,m_src_height-3);
		} else
		{
			rand_x = (i%(m_src_width-4))+2;
			rand_y = (i/(m_src_width-4))+2;
		}

		NN_Executer::Get_Kernel_Luma(low_res_data,
					rand_x,rand_y,
					m_src_width,m_src_height,m_src_comps_num,
					m_pInputTravel);

		// now fill in the 4 "labels"
		// these are the 4 pixels in the full resolution that are in the place of the single pixel in the half_res (in the center of the kernel)

		int ind_0 = ((rand_x*2))*comp_num + (rand_y*2)*comp_num*m_dest_width;
		int ind_1 = ((rand_x*2)+1)*comp_num + (rand_y*2)*comp_num*m_dest_width;
		int ind_2 = ((rand_x*2))*comp_num + ((rand_y*2)+1)*comp_num*m_dest_width;
		int ind_3 = ((rand_x*2)+1)*comp_num + ((rand_y*2)+1)*comp_num*m_dest_width;

		// debug passthrough

		//unsigned char c = ((pdInputDataTravel[12] + 1.0)*0.5)*255.0;
		
		/*unsigned char c = FLOAT_TO_UCHAR(pdInputDataTravel[12]);
		
		//debug - passthrough
		m_pLabels[i*4+0] = c;
		m_pLabels[i*4+1] = c;
		m_pLabels[i*4+2] = c;
		m_pLabels[i*4+3] = c;*/

		//RESTORE
		if (3 == comp_num)
		{
			unsigned char luma_0 = Calc_Luma(high_res_data[ind_0],high_res_data[ind_0+1],high_res_data[ind_0+2]);
			unsigned char luma_1 = Calc_Luma(high_res_data[ind_1],high_res_data[ind_1+1],high_res_data[ind_1+2]);
			unsigned char luma_2 = Calc_Luma(high_res_data[ind_2],high_res_data[ind_2+1],high_res_data[ind_2+2]);
			unsigned char luma_3 = Calc_Luma(high_res_data[ind_3],high_res_data[ind_3+1],high_res_data[ind_3+2]);

			m_pOutputTravel[0] = UCHAR_TO_FLOAT_0_TO_1(luma_0);
			m_pOutputTravel[1] = UCHAR_TO_FLOAT_0_TO_1(luma_1);
			m_pOutputTravel[2] = UCHAR_TO_FLOAT_0_TO_1(luma_2);
			m_pOutputTravel[3] = UCHAR_TO_FLOAT_0_TO_1(luma_3);
		} else
		{
			m_pOutputTravel[0] = UCHAR_TO_FLOAT_0_TO_1(high_res_data[ind_0]);
			m_pOutputTravel[1] = UCHAR_TO_FLOAT_0_TO_1(high_res_data[ind_1]);
			m_pOutputTravel[2] = UCHAR_TO_FLOAT_0_TO_1(high_res_data[ind_2]);
			m_pOutputTravel[3] = UCHAR_TO_FLOAT_0_TO_1(high_res_data[ind_3]);
		}

		m_pInputTravel+= 5*5;
		m_pOutputTravel+= 4;
	}

	SAFE_DEL_ARRAY(high_res_data);	
	SAFE_DEL_ARRAY(low_res_data);
}



bool TrainingSet::Init_MNIST(const char* images, const char* labels)
{
	m_labels_per_entry = 1;

	unsigned char * pInputCases = NULL;
	unsigned char * pLabels = NULL;

	m_TestCasesNum = -1;
	m_TestCase_Input_Width = -1;
	m_TestCase_Input_Height = -1;

	FILE* f = fopen(images,"rb");

	ERROR_EXIT_IF(!f,"Can't load training set images! [%s]",images);

	//IDX format
	//0000     32 bit integer  0x00000801(2049) magic number (MSB first) 
	//first two are always zero
	//08 means element size is unsigned byte
	//03 means 3d (because this is a list of matrices...)
	unsigned int tok;
	fread(&tok,sizeof(unsigned int),1,f);
	FLIP_ENDIAN(tok);
	assert(0x00000803 == tok);

	//0004     32 bit integer  60000            number of items 
	fread(&m_TestCasesNum,sizeof(unsigned int),1,f);
	FLIP_ENDIAN(m_TestCasesNum);

	//0008     32 bit integer  28               number of rows 
	fread(&m_TestCase_Input_Width,sizeof(unsigned int),1,f);
	FLIP_ENDIAN(m_TestCase_Input_Width);
	
	//0012     32 bit integer  28               number of columns 
	fread(&m_TestCase_Input_Height,sizeof(unsigned int),1,f);
	FLIP_ENDIAN(m_TestCase_Input_Height);

	// from now on it's the images pixels
	pInputCases = new unsigned char[m_TestCasesNum*m_TestCase_Input_Width*m_TestCase_Input_Height];

	fread(pInputCases,1,m_TestCasesNum*m_TestCase_Input_Width*m_TestCase_Input_Height,f);
	fclose(f);

	m_pInputCases_Double = new double[m_TestCasesNum*m_TestCase_Input_Width*m_TestCase_Input_Height];
	for (int i=0;i<m_TestCasesNum*m_TestCase_Input_Width*m_TestCase_Input_Height;i++)
	{
		//map from [0,255] to [-1,1]
		//m_pInputCases_Double[i] = double(m_pInputCases[i]) / 255.0;
		m_pInputCases_Double[i] = UCHAR_TO_FLOAT_MIN_1_TO_1(pInputCases[i]);
	}

	f = fopen(labels,"rb");

	ERROR_EXIT_IF(!f,"Can't load training set labels! [%s]",images);

	////////////////////////
	// Now read the labels
	////////////////////////
	fread(&tok,sizeof(unsigned int),1,f);
	FLIP_ENDIAN(tok);	
	assert(0x00000801 == tok);

	//0004     32 bit integer  60000            number of items 
	fread(&tok,sizeof(unsigned int),1,f);
	FLIP_ENDIAN(tok);
	assert(tok == m_TestCasesNum);

	// from now on it's the images labels
	pLabels = new unsigned char[m_TestCasesNum];

	//m_labels_per_entry = 10; // since this is classification, we're gonna have 1.0 for desired class and 0.0 for the rest

	//m_labels_per_entry = 256; // since this is classification, we're gonna have 1.0 for desired class and 0.0 for the rest

	m_labels_per_entry = 124; // there are actually 118 right now


	m_pLabels_Double = new double[m_TestCasesNum*m_labels_per_entry];
	fread(pLabels,1,m_TestCasesNum,f);	

	fclose(f);

	for (int i=0;i<m_TestCasesNum;i++)
	{
		for (int j=0;j<m_labels_per_entry;j++)
		{
			if (j == pLabels[i])
			{
				m_pLabels_Double[(i*m_labels_per_entry)+j] = 1.0;
			} else
			{
				m_pLabels_Double[(i*m_labels_per_entry)+j] = 0.0;
			}
		}		 
	}

	m_InputSize = m_TestCase_Input_Width*m_TestCase_Input_Height;

	SAFE_DEL_ARRAY(pInputCases);
	SAFE_DEL_ARRAY(pLabels);

	m_bInitialized = true;
	return true;
}

TrainingSet::~TrainingSet()
{
	SAFE_DEL_ARRAY(m_pInputCases_Double);
	SAFE_DEL_ARRAY(m_pLabels_Double);

}

bool TrainingSet::GetTestCase( unsigned int ind, double*& inputs,double*& labels)
{
	assert(m_bInitialized);
	if (ind>m_TestCasesNum-1)
	{
		assert(0);
		return false;
	}

	inputs = &m_pInputCases_Double[ind*m_InputSize];
	labels = &m_pLabels_Double[ind*m_labels_per_entry];

	return true;
}



