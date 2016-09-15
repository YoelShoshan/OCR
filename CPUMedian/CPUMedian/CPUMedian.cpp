// CPUMedian.cpp : Defines the entry point for the console application.
//

#include "ctmf.h"
#include "bmp.h"
#include <assert.h>
#include <stdio.h>

//debug
//#include <Windows.h>

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("Usage: call with ""input.bmp"" ""output.bmp""\n");
		return 0;
	}

	//MessageBoxA(0,0,0,0);

	unsigned int width=0;
	unsigned int height=0;
	unsigned int comps=0;
	
	//src image on host
	unsigned char *src_data = LoadBMP(argv[1],width,height,comps);
	assert(1==comps);

	unsigned char *median_data = new unsigned char [width*height];	
	unsigned char *result_data = new unsigned char [width*height];

	//Note: change last param to the size of the l2 cache
	ctmf(src_data,median_data,width,height,width,width,5,1,512*1024);	


	for (unsigned int i=0;i<width*height;i++)
	{
		if (src_data[i] > median_data[i] - 5)
		{
			result_data[i] = 255;
		} else
		{
			result_data[i] = 0;
		}
	}

	SaveBMP_GreyScale(result_data,width,height,argv[2]);

	return 0;
}

