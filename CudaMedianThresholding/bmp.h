#pragma once

#include <Windows.h>

double Calc_Luma(unsigned char r, unsigned char g, unsigned char b);
double Calc_Chroma_U(unsigned char r, unsigned char g, unsigned char b);
double Calc_Chroma_V(unsigned char r, unsigned char g, unsigned char b);

BYTE YUV_TO_R(BYTE y, BYTE u, BYTE v);
BYTE YUV_TO_G(BYTE y, BYTE u, BYTE v);
BYTE YUV_TO_B(BYTE y, BYTE u, BYTE v);


// allocates the image - freeing responsibility is on the caller.
unsigned char* LoadBMP(const char* image_name, UINT& width, unsigned int& height, UINT& components_num);

bool SaveBMP_GreyScale( BYTE* Buffer, int width, int height, LPCTSTR bmpfile );
//expects RGB
bool SaveBMP_Color( BYTE* Buffer, int width, int height, LPCTSTR bmpfile );