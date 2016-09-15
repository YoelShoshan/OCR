#ifndef TRAINING_SET_H
#define TRAINING_SET_H

//#include <Windows.h>
#include <map>
#include <string>

class TrainingSet
{
public:
	
	TrainingSet();
	virtual ~TrainingSet();

	//////////// upscale - digit detection

	bool Init_MNIST(const char* images, const char* labels); // digit recognition
	
	//////////// upscale

	bool Init_Upscale(
		std::map<std::string,int>& low_res_images,
		std::map<std::string,int>& high_res_images,
		bool randomize_order, 
		unsigned int requested_test_cases_num_per_image=0); // 0 means use all of it

	bool Init_SparseAutoEncoder(
		std::map<std::string,int>& images,
		bool randomize_order, 
		unsigned int requested_test_cases_num_per_image=0); // 0 means use all of it
				
	bool  GetTestCase( unsigned int ind, double*& inputs, double*& labels);
		
	unsigned int GetTestCasesNum() const { return m_TestCasesNum;}
	
	unsigned int GetTestCase_Input_Width() const { return m_TestCase_Input_Width;}
	unsigned int GetTestCase_Input_Height() const { return m_TestCase_Input_Height;}

	unsigned int GetOutputsNum() const { return m_labels_per_entry;}
		
private:

	bool AddSingleImage_Upscale(
		const char* low_res_image,
		const char* high_res_image,
		bool randomize_order,
		unsigned int requested_test_cases_num_per_image); // 0 means use all of it

	bool AddSingleImage_PassThrough(
		const char* image,
		bool randomize_order,
		unsigned int requested_test_cases_num_per_image); // 0 means use all of it

	bool m_bInitialized;

	//unsigned char* m_pInputCases;
	double* m_pInputCases_Double;

	unsigned int m_labels_per_entry;
	//unsigned char* m_pLabels;
	double* m_pLabels_Double;

	//////////////////////////////////
	// for loading
	double* m_pInputTravel;
	double* m_pOutputTravel;
	//////////////////////////////////

	unsigned int m_TestCasesNum;
	unsigned int m_TestCase_Input_Width;
	unsigned int m_TestCase_Input_Height;
	unsigned int m_InputSize;

	unsigned int m_src_width;
	unsigned int m_src_height;
	unsigned int m_src_comps_num;

	unsigned int m_dest_width;
	unsigned int m_dest_height;
	unsigned int m_dest_comps_num;
	

};





#endif
