#pragma once

double Calc_Luma(unsigned char r, unsigned char g, unsigned char b);
double Calc_Chroma_U(unsigned char r, unsigned char g, unsigned char b);
double Calc_Chroma_V(unsigned char r, unsigned char g, unsigned char b);

unsigned char YUV_TO_R(unsigned char y, unsigned char u, unsigned char v);
unsigned char YUV_TO_G(unsigned char y, unsigned char u, unsigned char v);
unsigned char YUV_TO_B(unsigned char y, unsigned char u, unsigned char v);

// allocates the image - freeing responsibility is on the caller.
unsigned char* LoadBMP(const char* image_name, unsigned int& width, unsigned int& height, unsigned int& components_num);

bool SaveBMP_GreyScale( unsigned char* Buffer, int width, int height, const char* bmpfile );

bool SaveBMP_Color( unsigned char* Buffer, int width, int height, const char *bmpfile );
//expects RGB
/*
bool SaveBMP_Color( unsigned char* Buffer, int width, int height, LPCTSTR bmpfile );
*/
