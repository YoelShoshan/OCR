#ifndef NEURAL_NETWORK_EXECUTER_H
#define NEURAL_NETWORK_EXECUTER_H

//#include <Windows.h>
#include "Defines.h"

class NeuralNetwork; //predecl
class NN_Executer
{
public:
	NN_Executer();
	~NN_Executer();

	static void OutputResult_Upscaling(NeuralNetwork* pNN, const char* src, const char* dst);

	static void OutputResult_SparseAutoEncoder(NeuralNetwork* pNN, const char* src, const char* dst);

	static void OutputResult_Mnist(NeuralNetwork* pNN, const char* src, const char* dst, const char* results_file);

	//assuming a single component in the images
	static void UpscaleNearest(
		unsigned int src_width, unsigned int src_height,
		unsigned char* pSource,
		unsigned char* pDest);

	//assuming a single component in the images
	static void UpscaleLinear(
		unsigned int src_width, unsigned int src_height,
		unsigned char* pSource,
		unsigned char* pDest);

	//assuming 3 (rgb) components in the source, 1 component for dest
	static void GetImageComponent_Y(
		unsigned int half_res_width, unsigned int half_res_height,
		unsigned char* pSource,
		unsigned char* pDest);

	//assuming 3 (rgb) components in the source, 1 component for dest
	static void GetImageComponent_U(
		unsigned int half_res_width, unsigned int half_res_height,
		unsigned char* pSource,
		unsigned char* pDest);

	//assuming 3 (rgb) components in the source, 1 component for dest
	static void GetImageComponent_V(
		unsigned int half_res_width, unsigned int half_res_height,
		unsigned char* pSource,
		unsigned char* pDest);


	static void ConvertYUV_To_RGB(
		unsigned int width, unsigned int height,
		unsigned char* pSrcY,unsigned char* pSrcU,unsigned char* pSrcV,
		unsigned char* pDestBGR);

	static void Get_Kernel_Luma(
		ZED_IN unsigned char* half_res_data, ZED_IN unsigned int half_res_x,ZED_IN unsigned int half_res_y, 
		ZED_IN unsigned int half_res_width, ZED_IN unsigned int half_res_height,
		ZED_IN unsigned int comps_num, ZED_IN ZED_OUT double* pInputData);

};






#endif
