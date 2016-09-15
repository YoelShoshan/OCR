#include "NN_Executer.h"
#include "bmp.h"
#include "NeuralNetwork.h"
#include "Training_Set.h"
#include <string.h>
#include <stdio.h>
#include "zedusOS.h"
extern FILE* g_pLog;




void NN_Executer::OutputResult_Mnist(NeuralNetwork* pNN, const char* src, const char* dst, const char* results_file)
{
	TrainingSet* ts = new TrainingSet();
	ts->Init_MNIST(src,dst);


	

	/*char ai_result_file_name[MAX_PATH];
	sprintf(ai_result_file_name, "%s_AI", dst);*/
	FILE* pAI_ResultsFile = fopen(results_file,"wb");

	double dScore = 0.0;

	unsigned int inputs_num = pNN->m_InputsNum;
	unsigned int outputs_num = pNN->m_OutputsNum;

	double* pCurrentInputs = NULL;
	double* pCurrentOutputs = new double[outputs_num+1];
	double* pLabels = NULL;

	unsigned char desired = 0;
	unsigned char classified = 0;
	double max = -9999.0;

	unsigned int errors = 0;

	for (int i=0;i<ts->GetTestCasesNum();i++)
	{
		ts->GetTestCase(i,pCurrentInputs, pLabels);

		pNN->Forward_Pass(
			inputs_num,
			pCurrentInputs,
			outputs_num,
			pCurrentOutputs);

		max = -9999.0;

		for (int j=0;j<outputs_num;j++)
		{
			dScore += (pCurrentOutputs[j]-pLabels[j]) * (pCurrentOutputs[j]-pLabels[j]);

			if (pCurrentOutputs[j] > max)
			{
				max = pCurrentOutputs[j];
				classified = j;
			}

			if (pLabels[j] == 1.0)
			{
				desired = j;
			}						
		}

		if (desired != classified)
		{
			errors++;
			LOG("*** mimstach! *** ");
		}		

		fwrite(&classified, sizeof(unsigned char), 1, pAI_ResultsFile);

		LOG("Desired was %d, NN output was %d\n", desired, classified);

	}

	double error_percent = 100.0*(double(errors) / double(ts->GetTestCasesNum()));

	FPRINTF("Score=%f\n %d errors out of %d cases. %f%% errors.\n", 
		dScore,
		errors,
		ts->GetTestCasesNum(),
		error_percent
		);

	SAFE_DEL_ARRAY(pCurrentOutputs);

	SAFE_DEL(ts);


	fclose(pAI_ResultsFile);
}

void NN_Executer::OutputResult_SparseAutoEncoder(NeuralNetwork* pNN, const char* src, const char* dst)
{
	// load half_image

	unsigned int src_width	= 0;
	unsigned int src_height	= 0;
	unsigned int src_comps_num = 0;

	unsigned char* half_res_data = LoadBMP(src, src_width, src_height, src_comps_num);
	unsigned char* half_res_yuv[3];
	half_res_yuv[ZED_Y] = new unsigned char[src_width*src_height];
	half_res_yuv[ZED_U] = new unsigned char[src_width*src_height];
	half_res_yuv[ZED_V] = new unsigned char[src_width*src_height];
		
	GetImageComponent_Y(
		src_width, src_height,
		half_res_data,
		half_res_yuv[ZED_Y]);

	GetImageComponent_U(
		src_width, src_height,
		half_res_data,
		half_res_yuv[ZED_U]);

	GetImageComponent_V(
		src_width, src_height,
		half_res_data,
		half_res_yuv[ZED_V]);

	unsigned int inputs_num = pNN->m_InputsNum;
	unsigned int outputs_num = pNN->m_OutputsNum;

	double* pCurrentInputs = new double[inputs_num+1];
	double* pCurrentOutputs = new double[outputs_num+1];	


	unsigned char* full_res_yuv[3];
	
	full_res_yuv[ZED_Y] = new unsigned char[(src_width)*(src_height)];
	full_res_yuv[ZED_U] = new unsigned char[(src_width)*(src_height)];
	full_res_yuv[ZED_V] = new unsigned char[(src_width)*(src_height)];

	unsigned char* full_res_rgb = new unsigned char[(src_width)*(src_height)*3];


	//TODO: No real need to do the kernels overlap.
	
	for (int j=0;j<src_height;j++)
	{
		for (int i=0;i<src_width;i++)
		{
			if (i<2 || i> src_width-3 ||
				j<2 || j> src_height-3)
			{
				//TODO: change to entire kernel
				pCurrentOutputs[0] = 0;
				pCurrentOutputs[1] = 0;
				pCurrentOutputs[2] = 0;
				pCurrentOutputs[3] = 0;
			} else
			{
				/*m_pTrainingSet->GetInputs(m_pHalfTestImage,
					i,j,
					m_HalfResImage_Width,m_HalfResImage_Height,m_HalfResImage_CompsNum,
					m_pCurrentInputs);*/

				Get_Kernel_Luma(
					half_res_data, 
					i,j,
					src_width,src_height,
					src_comps_num, 
					pCurrentInputs);


				// TODO: i need to refactor and store this size inside the NN
				pCurrentInputs[5 * 5] = -1.0; //for bias
	
				pNN->Forward_Pass(
					inputs_num,
					pCurrentInputs,
					outputs_num,
					pCurrentOutputs);

				for (unsigned int o=0;o<25;o++) //overriding over and over ... (kernels moving over the same places)
				{
					unsigned int src_x = o%5;
					unsigned int src_y = o/5;
					unsigned int dest_index = (i-2+src_x)+((j-2+src_y)*src_width);				
					full_res_yuv[ZED_Y][dest_index] = pCurrentOutputs[o]*255.0;;					

					//full_res_yuv[Y][dest_index] = half_res_yuv[Y][dest_index];
					full_res_yuv[ZED_U][dest_index] = half_res_yuv[ZED_U][dest_index];
					full_res_yuv[ZED_V][dest_index] = half_res_yuv[ZED_V][dest_index];
				}		

			}						
			
		}
	}
		
	char file[512];

/*	if (0)
	{
		sprintf(file,"%s_Y.bmp", dst);
		SaveBMP_GreyScale(m_pUpscaledImage[Y],m_HalfResImage_Width*2, m_HalfResImage_Height*2,file);	

		TrainingSet::ConvertYUV_To_RGB(m_HalfResImage_Width*2, m_HalfResImage_Height*2, 
			m_pUpscaledImage[Y],
			m_pUpscaledImage_NEAREST[U], m_pUpscaledImage_NEAREST[V], m_pUpscaledImage_FULL);
		sprintf(file,"%s_ChromaNearest.bmp",dst);
		SaveBMP_Color(m_pUpscaledImage_FULL,m_HalfResImage_Width*2,m_HalfResImage_Height*2,file);
	}*/


	/////////////////////////
	// upscale u,v linearly
	/////////////////////////

	/*UpscaleLinear(
		src_width,src_height,
		half_res_yuv[U],
		full_res_yuv[U]);

	UpscaleLinear(
		src_width,src_height,
		half_res_yuv[V],
		full_res_yuv[V]);*/
	
	ConvertYUV_To_RGB(src_width, src_height, 
		full_res_yuv[ZED_Y],
		full_res_yuv[ZED_U], full_res_yuv[ZED_V], full_res_rgb);
	
	//sprintf(file,"%s_ChromaLinear.bmp",dst);
	sprintf(file,"%s",dst);
	SaveBMP_Color(full_res_rgb,src_width,src_height,file);

	SAFE_DEL_ARRAY(half_res_data);
	SAFE_DEL_ARRAY(half_res_yuv[ZED_Y]);
	SAFE_DEL_ARRAY(half_res_yuv[ZED_U]);
	SAFE_DEL_ARRAY(half_res_yuv[ZED_V]);
	
	SAFE_DEL_ARRAY(pCurrentInputs);
	SAFE_DEL_ARRAY(pCurrentOutputs);
	SAFE_DEL_ARRAY(full_res_yuv[ZED_Y]);
	SAFE_DEL_ARRAY(full_res_yuv[ZED_U]);
	SAFE_DEL_ARRAY(full_res_yuv[ZED_V]);
	SAFE_DEL_ARRAY(full_res_rgb);
}


void NN_Executer::OutputResult_Upscaling(NeuralNetwork* pNN, const char* src, const char* dst)
{
	// load half_image

	unsigned int src_width	= 0;
	unsigned int src_height	= 0;
	unsigned int src_comps_num = 0;

	unsigned char* half_res_data = LoadBMP(src, src_width, src_height, src_comps_num);
	unsigned char* half_res_yuv[3];
	half_res_yuv[ZED_Y] = new unsigned char[src_width*src_height];
	half_res_yuv[ZED_U] = new unsigned char[src_width*src_height];
	half_res_yuv[ZED_V] = new unsigned char[src_width*src_height];
		
	GetImageComponent_Y(
		src_width, src_height,
		half_res_data,
		half_res_yuv[ZED_Y]);

	GetImageComponent_U(
		src_width, src_height,
		half_res_data,
		half_res_yuv[ZED_U]);

	GetImageComponent_V(
		src_width, src_height,
		half_res_data,
		half_res_yuv[ZED_V]);

	unsigned int inputs_num = pNN->m_InputsNum;
	unsigned int outputs_num = pNN->m_OutputsNum;

	double* pCurrentInputs = new double[inputs_num+1];
	double* pCurrentOutputs = new double[outputs_num+1];	


	unsigned char* full_res_yuv[3];
	
	full_res_yuv[ZED_Y] = new unsigned char[(src_width*2)*(src_height*2)];
	full_res_yuv[ZED_U] = new unsigned char[(src_width*2)*(src_height*2)];
	full_res_yuv[ZED_V] = new unsigned char[(src_width*2)*(src_height*2)];

	unsigned char* full_res_rgb = new unsigned char[(src_width*2)*(src_height*2)*3];


	//TODO: - implement this again
	
	for (int j=0;j<src_height;j++)
	{
		for (int i=0;i<src_width;i++)
		{
			if (i<2 || i> src_width-3 ||
				j<2 || j> src_height-3)
			{
				pCurrentOutputs[0] = 0;
				pCurrentOutputs[1] = 0;
				pCurrentOutputs[2] = 0;
				pCurrentOutputs[3] = 0;
			} else
			{
				/*m_pTrainingSet->GetInputs(m_pHalfTestImage,
					i,j,
					m_HalfResImage_Width,m_HalfResImage_Height,m_HalfResImage_CompsNum,
					m_pCurrentInputs);*/

				Get_Kernel_Luma(
					half_res_data, 
					i,j,
					src_width,src_height,
					src_comps_num, 
					pCurrentInputs);


				// TODO: i need to refactor and store this size inside the NN
				pCurrentInputs[5 * 5] = -1.0; //for bias
	
				pNN->Forward_Pass(
					inputs_num,
					pCurrentInputs,
					outputs_num,
					pCurrentOutputs);
			}		

			full_res_yuv[ZED_Y][(i*2)   + ((j*2)*src_width*2)]	= pCurrentOutputs[0]*255.0;
			full_res_yuv[ZED_Y][(i*2+1) + ((j*2)*src_width*2)]	= pCurrentOutputs[1]*255.0;
			full_res_yuv[ZED_Y][(i*2)   + ((j*2+1)*src_width*2)]= pCurrentOutputs[2]*255.0;
			full_res_yuv[ZED_Y][(i*2+1) + ((j*2+1)*src_width*2)]= pCurrentOutputs[3]*255.0;
		}
	}
		
	char file[512];

/*	if (0)
	{
		sprintf(file,"%s_Y.bmp", dst);
		SaveBMP_GreyScale(m_pUpscaledImage[Y],m_HalfResImage_Width*2, m_HalfResImage_Height*2,file);	

		TrainingSet::ConvertYUV_To_RGB(m_HalfResImage_Width*2, m_HalfResImage_Height*2, 
			m_pUpscaledImage[Y],
			m_pUpscaledImage_NEAREST[U], m_pUpscaledImage_NEAREST[V], m_pUpscaledImage_FULL);
		sprintf(file,"%s_ChromaNearest.bmp",dst);
		SaveBMP_Color(m_pUpscaledImage_FULL,m_HalfResImage_Width*2,m_HalfResImage_Height*2,file);
	}*/


	/////////////////////////
	// upscale u,v linearly
	/////////////////////////

	UpscaleLinear(
		src_width,src_height,
		half_res_yuv[ZED_U],
		full_res_yuv[ZED_U]);

	UpscaleLinear(
		src_width,src_height,
		half_res_yuv[ZED_V],
		full_res_yuv[ZED_V]);
	
	ConvertYUV_To_RGB(src_width*2, src_height*2, 
		full_res_yuv[ZED_Y],
		full_res_yuv[ZED_U], full_res_yuv[ZED_V], full_res_rgb);
	
	//sprintf(file,"%s_ChromaLinear.bmp",dst);
	sprintf(file,"%s",dst);
	SaveBMP_Color(full_res_rgb,src_width*2,src_height*2,file);

	SAFE_DEL_ARRAY(half_res_data);
	SAFE_DEL_ARRAY(half_res_yuv[ZED_Y]);
	SAFE_DEL_ARRAY(half_res_yuv[ZED_U]);
	SAFE_DEL_ARRAY(half_res_yuv[ZED_V]);
	
	SAFE_DEL_ARRAY(pCurrentInputs);
	SAFE_DEL_ARRAY(pCurrentOutputs);
	SAFE_DEL_ARRAY(full_res_yuv[ZED_Y]);
	SAFE_DEL_ARRAY(full_res_yuv[ZED_U]);
	SAFE_DEL_ARRAY(full_res_yuv[ZED_V]);
	SAFE_DEL_ARRAY(full_res_rgb);
}

// gets the kernel 
void NN_Executer::Get_Kernel_Luma(ZED_IN unsigned char* half_res_data, ZED_IN unsigned int half_res_x,ZED_IN unsigned int half_res_y, 
		ZED_IN unsigned int half_res_width, ZED_IN unsigned int half_res_height,
		ZED_IN unsigned int comps_num, ZED_IN ZED_OUT double* pInputData)
{			
	ERROR_EXIT_IF(half_res_x<2 || half_res_x> half_res_width-3, "Get_Kernel_Luma:: illegal half_res_x!");	
	ERROR_EXIT_IF(half_res_y<2 || half_res_y> half_res_height-3, "Get_Kernel_Luma:: illegal half_res_y!");	

	for (int x = 0; x<5; x++)
	{
		for (int y = 0; y<5; y++)
		{
			int ind = /*(i*5*5)+*/ x+(y*5);
			int ind_image = (half_res_x-2+x)*comps_num  + (half_res_y-2+y)*half_res_width*comps_num;

			double luma = 0.f;

			if (1 == comps_num)
			{
				luma = half_res_data[ind_image];

			} else
			{
				//assert(0);
				luma = Calc_Luma(
					half_res_data[ind_image],
					half_res_data[ind_image+1],
					half_res_data[ind_image+2]);
			}				

			//pInputData[ind] = luma;
			//pInputData[ind] = -1.0 + luma / (255.0 * 0.5);
			//pInputData[ind] = luma / 255.0;

			pInputData[ind] = UCHAR_TO_FLOAT_MIN_1_TO_1(luma);	
		}
	}
}

//assuming a single component in the images
void NN_Executer::UpscaleNearest(
	unsigned int src_width, unsigned int src_height,
	unsigned char* pSource,
	unsigned char* pDest)
{		
	for (int j=0;j<src_height;j++)
	{
		for (int i=0;i<src_width;i++)
		{
			int ind = (j*src_width) + (i);

			unsigned char col = pSource[ind];

			pDest[((j*2)*src_width*2) + ((i*2)+0)] = col;
			pDest[((j*2)*src_width*2) + ((i*2+1)+0)] = col;
			pDest[((j*2+1)*src_width*2) + ((i*2)+0)] = col;
			pDest[((j*2+1)*src_width*2) + ((i*2+1)+0)] = col;
		}

	}
}

//assuming a single component in the images
void NN_Executer::UpscaleLinear(
	unsigned int src_width, unsigned int src_height,
	unsigned char* pSource,
	unsigned char* pDest)
{		
	for (int j=0;j<src_height;j++)
	{
		for (int i=0;i<src_width;i++)
		{
			if ( (i<1 || i>src_width-3) || (j<1 || j>src_height-3) )
			{
				//top left
				pDest[((j*2)*src_width*2) + ((i*2)+0)] = 0;

				//top right
				pDest[((j*2)*src_width*2) + ((i*2+1)+0)] = 0;

				//bottom left
				pDest[((j*2+1)*src_width*2) + ((i*2)+0)] = 0;

				//bottom right
				pDest[((j*2+1)*src_width*2) + ((i*2+1)+0)] = 0;

				continue;
			}

			int ind_self = (j*src_width) + (i);
			int ind_left = (j*src_width) + (i-1);
			int ind_right = (j*src_width) + (i+1);
			int ind_up = ((j-1)*src_width) + (i);
			int ind_down = ((j+1)*src_width) + (i);


			double col_self = pSource[ind_self];
			double col_left = pSource[ind_left];
			double col_right = pSource[ind_right];
			double col_up = pSource[ind_up];
			double col_down = pSource[ind_down];
			

			//top left
			pDest[((j*2)*src_width*2) + ((i*2)+0)] = 
				0.5*col_self+0.25*col_up+0.25*col_left;

			//top right
			pDest[((j*2)*src_width*2) + ((i*2+1)+0)] =
				0.5*col_self+0.25*col_up+0.25*col_right;

			//bottom left
			pDest[((j*2+1)*src_width*2) + ((i*2)+0)] =
				0.5*col_self+0.25*col_down+0.25*col_left;

			//bottom right
			pDest[((j*2+1)*src_width*2) + ((i*2+1)+0)] =
				0.5*col_self+0.25*col_down+0.25*col_right;
		}

	}
}

void NN_Executer::ConvertYUV_To_RGB(
	unsigned int width, unsigned int height,
	unsigned char* pSrcY,unsigned char* pSrcU,unsigned char* pSrcV,
	unsigned char* pDestBGR)
{
	for (int j=0;j<height;j++)
	{
		for (int i=0;i<width;i++)
		{
			int ind_src = (j*width) + (i);
			int ind_dest = (j*width*3) + (i*3);

			unsigned char R = YUV_TO_R(pSrcY[ind_src],pSrcU[ind_src],pSrcV[ind_src]);
			unsigned char G = YUV_TO_G(pSrcY[ind_src],pSrcU[ind_src],pSrcV[ind_src]);
			unsigned char B = YUV_TO_B(pSrcY[ind_src],pSrcU[ind_src],pSrcV[ind_src]);

			pDestBGR[ind_dest+0] = R;
			pDestBGR[ind_dest+1] = G;
			pDestBGR[ind_dest+2] = B;
		}
	}
}

//assuming 3 (rgb) components in the source, 1 component for dest
void NN_Executer::GetImageComponent_Y(
	unsigned int half_res_width, unsigned int half_res_height,
	unsigned char* pSource,
	unsigned char* pDest)
{
	for (int j=0;j<half_res_height;j++)
	{
		for (int i=0;i<half_res_width;i++)
		{
			int ind_src = (j*half_res_width*3) + (i*3);
			int ind_dest = (j*half_res_width) + (i);

			double y = Calc_Luma(
				pSource[ind_src],pSource[ind_src+1],pSource[ind_src+2]);

			pDest[ind_dest] = y;
		}
	}
}

//assuming 3 (rgb) components in the source, 1 component for dest
void NN_Executer::GetImageComponent_U(
	unsigned int half_res_width, unsigned int half_res_height,
	unsigned char* pSource,
	unsigned char* pDest)
{
	for (int j=0;j<half_res_height;j++)
	{
		for (int i=0;i<half_res_width;i++)
		{
			int ind_src = (j*half_res_width*3) + (i*3);
			int ind_dest = (j*half_res_width) + (i);

			double y = Calc_Chroma_U(
				pSource[ind_src],pSource[ind_src+1],pSource[ind_src+2]);

			pDest[ind_dest] = y;
		}
	}
}

//assuming 3 (rgb) components in the source, 1 component for dest
void NN_Executer::GetImageComponent_V(
	unsigned int half_res_width, unsigned int half_res_height,
	unsigned char* pSource,
	unsigned char* pDest)
{
	for (int j=0;j<half_res_height;j++)
	{
		for (int i=0;i<half_res_width;i++)
		{
			int ind_src = (j*half_res_width*3) + (i*3);
			int ind_dest = (j*half_res_width) + (i);

			double y = Calc_Chroma_V(
				pSource[ind_src],pSource[ind_src+1],pSource[ind_src+2]);

			pDest[ind_dest] = y;
		}
	}
}
