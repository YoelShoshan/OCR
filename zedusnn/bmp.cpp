#include "bmp.h"
#include "stdio.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <assert.h>

using namespace std;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#if defined(_MSC_VER)
// __declspec(align(1)) doesn't seem to have any effect @_@
 struct ZEDUS_BITMAPFILEHEADER {
        unsigned short    bfType;
        unsigned int   bfSize;
        unsigned short    bfReserved1;
        unsigned short    bfReserved2;
        unsigned int   bfOffBits;
};

struct ZEDUS_BITMAPINFOHEADER{
        unsigned int      biSize;
        unsigned long     biWidth;
        unsigned long       biHeight;
        unsigned short       biPlanes;
        unsigned short       biBitCount;
        unsigned int      biCompression;
        unsigned int      biSizeImage;
        unsigned long       biXPelsPerMeter;
        unsigned long       biYPelsPerMeter;
        unsigned int      biClrUsed;
        unsigned int      biClrImportant;
};

void size_sanity()
{
	if (sizeof(unsigned short) != 2)
	{
		printf("Error! unsigned short sizeof() expected to be 2!\n");
	}

	if (sizeof(unsigned int) != 4)
	{
		printf("Error! unsigned int sizeof() expected to be 4!\n");
	}

	if (sizeof(unsigned long) != 4)
	{
		printf("Error! unsigned long sizeof() expected to be 4!\n");
	}
}

#elif defined(__GNUC__)
 struct ZEDUS_BITMAPFILEHEADER {
        unsigned short    bfType;
        unsigned int   bfSize;
        unsigned short    bfReserved1;
        unsigned short    bfReserved2;
        unsigned int   bfOffBits;
};

struct ZEDUS_BITMAPINFOHEADER{
        unsigned int      biSize;
        unsigned int     biWidth;
        unsigned int       biHeight;
        unsigned short       biPlanes;
        unsigned short       biBitCount;
        unsigned int      biCompression;
        unsigned int      biSizeImage;
        unsigned int       biXPelsPerMeter;
        unsigned int       biYPelsPerMeter;
        unsigned int      biClrUsed;
        unsigned int      biClrImportant;
};


void size_sanity()
{
	if (sizeof(unsigned short) != 2)
	{
		printf("Error! unsigned short sizeof() expected to be 2!\n");
	}

	if (sizeof(unsigned int) != 4)
	{
		printf("Error! unsigned int sizeof() expected to be 4!\n");
	}
}

#else
#pragma error Define your compiler!
#endif

/*void print_bitmap_file_header(ZEDUS_BITMAPFILEHEADER* fileheader)
{
	printf("printing size of ZEDUS_BITMAPFILEHEADER:\n");
//	printf("bfType=	
}

void print_bitmap_info_header(ZEDUS_BITMAPINFOHEADER* infoheader)
{
	printf("printing content of ZEDUS_BITMAPINFOHEADER:\n");
//	printf("bfType=
}*/

/*
def yuv2rgb(y,u,v):
    y -= 16
    u -= 128
    v -= 128
    r = y+1.403*v
    g = y-0.344*u-0.714*v
    b = y+1.77*u
    return (r,g,b)
*/

#define CLAMP(x,min_val,max_val) \
	x = min(max_val,x);\
	x = max(min_val,x);

unsigned char YUV_TO_R(unsigned char y, unsigned char u, unsigned char v)
{
	double dY = double(y) - 16.0;
	double dU = double(u) - 128.0;
	double dV = double(v) - 128.0;

	double R = dY + 1.403*dV;
	CLAMP(R,0,255)
	return (unsigned char)R;
}

unsigned char YUV_TO_G(unsigned char y, unsigned char u, unsigned char v)
{
	double dY = double(y) - 16.0;
	double dU = double(u) - 128.0;
	double dV = double(v) - 128.0;

	double G = dY-0.344*dU-0.714*dV;
	CLAMP(G,0,255)
	return (unsigned char)G;
}

unsigned char YUV_TO_B(unsigned char y, unsigned char u, unsigned char v)
{
	double dY = double(y) - 16.0;
	double dU = double(u) - 128.0;
	double dV = double(v) - 128.0;

	double B = dY+1.77*dU;
	CLAMP(B,0,255)
	return (unsigned char)B;
}

/*
def rgb2yuv(r,g,b):
    y = 16.0+ (0.299*r+0.587*g+0.114*b)
    u = 128+ (-0.168935*r-0.331665*g+0.50059*b)
    v = 128+ (0.499813*r-0.418531*g-0.081282*b)
    return (y,u,v)
*/


double Calc_Luma(unsigned char r, unsigned char g, unsigned char b)
{
	double Y = 16.0+0.299*double(r) + 0.587*double(g) + 0.114*double(b);		
	CLAMP(Y,0,255)
	return Y;
}

double Calc_Chroma_U(unsigned char r, unsigned char g, unsigned char b)
{
	double U = 128.0-0.16893*double(r) -0.331665*double(g) +0.50059*double(b);	
	CLAMP(U,0,255)
	return U;
}

double Calc_Chroma_V(unsigned char r, unsigned char g, unsigned char b)
{
	double V = 128.0+0.499813*double(r) -0.418531*double(g) -0.081282*double(b);	
	CLAMP(V,0,255)
	return V;
}

// allocates the image - freeing responsibility is on the caller.
unsigned char* LoadBMP(const char* image_name, unsigned int& width, unsigned int& height, unsigned int& components_num)
{
	size_sanity();
	ifstream file_in (image_name,ios::binary);

	if (!file_in)
	{
		printf("Error loading file [%s] !!\n", image_name);
	}

	printf("Loaded file [%s].\n", image_name);

	ZEDUS_BITMAPFILEHEADER bmfh;
	ZEDUS_BITMAPINFOHEADER info;

	size_t sz = sizeof (ZEDUS_BITMAPFILEHEADER );
	sz = sizeof (ZEDUS_BITMAPINFOHEADER );
		
	//tightly packed on file, can't read directly due to alignment issues
	//file_in.read((char*)&bmfh, sizeof (ZEDUS_BITMAPFILEHEADER ));

	file_in.read((char*)&bmfh.bfType, sizeof (bmfh.bfType ));	
	file_in.read((char*)&bmfh.bfSize, sizeof (bmfh.bfSize ));
	file_in.read((char*)&bmfh.bfReserved1, sizeof (bmfh.bfReserved1 ));
	file_in.read((char*)&bmfh.bfReserved2, sizeof (bmfh.bfReserved2 ));
	file_in.read((char*)&bmfh.bfOffBits, sizeof (bmfh.bfOffBits ));

	//print_bitmap_file_header(&bmfh);
	printf("size of short = %d\n", sizeof(short));
	printf("size of int = %d\n", sizeof(int));
	printf("size of long = %d\n", sizeof(long));

	//alignment issues - see comment above
	//file_in.read((char*)&info, sizeof (ZEDUS_BITMAPINFOHEADER ));

	file_in.read((char*)&info.biSize, sizeof (info.biSize ));
	file_in.read((char*)&info.biWidth, sizeof (info.biWidth ));
	file_in.read((char*)&info.biHeight, sizeof (info.biHeight ));
	file_in.read((char*)&info.biPlanes, sizeof (info.biPlanes ));
	file_in.read((char*)&info.biBitCount, sizeof (info.biBitCount ));
	file_in.read((char*)&info.biCompression, sizeof (info.biCompression ));
	file_in.read((char*)&info.biSizeImage, sizeof (info.biSizeImage ));
	file_in.read((char*)&info.biXPelsPerMeter, sizeof (info.biXPelsPerMeter ));
	file_in.read((char*)&info.biYPelsPerMeter, sizeof (info.biYPelsPerMeter ));
	file_in.read((char*)&info.biClrUsed, sizeof (info.biClrUsed ));
	file_in.read((char*)&info.biClrImportant, sizeof (info.biClrImportant ));
	
	//print_bitmap_info_header(&info);

	width = info.biWidth;
	height = info.biHeight;

	components_num = info.biBitCount/8;

	unsigned int line_bytes = width * components_num;

	unsigned int padding = line_bytes%4;

	unsigned int line_bytes_on_file = line_bytes;
	if (padding > 0)
	{
		line_bytes_on_file += 4-padding;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// In the BMP definition, when you use 8 bit per pixel, you are using pallete.
	// For greyscale, a pallete of f(X) = (X,X,X) is used.
	// Since we only care about greyscale for now, we abandon the pallete internal data when loading.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	if (components_num == 1) 
	{
		char pallete[256*4];
		file_in.read(pallete, 256*4);
		printf(".\n");
	}


	// read the data
	unsigned char* orig_data = new unsigned char[line_bytes_on_file * height];
	file_in.read((char*)orig_data, line_bytes_on_file * height);
	
	// the output data will not have any padding.
	unsigned char* data = new unsigned char [width * height * components_num];

	unsigned char* pTravel_Data = data;

	// flip height
	for (unsigned int i=0;i<height;i++)
	{
		/*if (i==BMP.bmHeight-1)
		{
			char d = 9;
		}
		printf("About to do line %d\n", i);*/
		//memcpy(pTravel_Data, &orig_data[ (BMP.bmHeight-1-i)*BMP.bmWidthunsigned chars]  , BMP.bmWidthunsigned chars);
		memcpy(pTravel_Data, &orig_data[ (height-1-i)*line_bytes_on_file]  , line_bytes);

		if (3 == components_num )
		{
			// convert from bgr to rgb
			for (unsigned int j=0;j<width;j++)
			{
				unsigned char temp;
				temp = pTravel_Data[j*3];
				pTravel_Data[j*3] = pTravel_Data[j*3+2];
				pTravel_Data[j*3+2] = temp;			
			}
		}


		pTravel_Data+= line_bytes;
		//pTravel_Data+= BMP.bmWidthunsigned chars;
	}

	delete [] orig_data;

	printf("Done loading.\n");

	return data;
}

// BMP format defines that greyscale BMPs are actually lookup tables which map f(X)=(X,X,X)
bool SaveBMP_GreyScale( unsigned char* Buffer, int width, int height, const char* bmpfile )
{
	ofstream out_file;
	out_file.open(bmpfile,ios::binary);

	if (!out_file.is_open())
	{
		printf("Error! could not open [%s] for writing.\n", bmpfile);

	}

	unsigned int line_bytes = width;
	unsigned int padding = line_bytes%4;

	unsigned int line_bytes_on_file = line_bytes;

	unsigned int extra_bytes = (4-padding)%4;

	if (padding > 0)
	{
		line_bytes_on_file += extra_bytes;
	}	

//Buffer is an array that contains the image data, width and height are the dimensions of the image to save, and paddedsize is the size of Buffer in unsigned chars. bmpfile is the filename to save to.
//First we declare the header structs and clear them:

	//RGBQUAD pallete[256];
	char pallete[256][4];
	for (int i=0;i<256;i++)
	{
		//NOTE: what to do with the reserved value?
		pallete[i][0]=i;
		pallete[i][1]=i;
		pallete[i][2]=i;
		pallete[i][3]=i;		
	}


	ZEDUS_BITMAPFILEHEADER bmfh;
	ZEDUS_BITMAPINFOHEADER info;
	memset ( &bmfh, 0, sizeof (ZEDUS_BITMAPFILEHEADER ) );
	memset ( &info, 0, sizeof (ZEDUS_BITMAPINFOHEADER ) );

//Next we fill the file header with data:

	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = 0;//sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width*height;
	size_t palette_size = sizeof(pallete);
	
	//bmfh.bfOffBits = sizeof(bmfh)+sizeof(info)+sizeof(pallete);//0x36; // offset to the beginning of the data

	// can't use sizeof due to alignment issues
	bmfh.bfOffBits = 14+40+sizeof(pallete);//0x36; // offset to the beginning of the data

//and the info header:

	info.biSize = sizeof(ZEDUS_BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;	
	//info.biBitCount = 24;
	info.biBitCount = 8;
	info.biCompression = 0;//BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0;//0x0ec4;  
	info.biYPelsPerMeter = 0;//0x0ec4;     
	
	//info.biClrUsed = 0;	
	//info.biClrImportant = 0; 

	info.biClrUsed = 256;
	info.biClrImportant = 256;

	// headers
	//alignment issues
	//out_file.write((const char*)&bmfh, sizeof ( ZEDUS_BITMAPFILEHEADER ));
	out_file.write((const char*)&bmfh.bfType, sizeof (bmfh.bfType  ));
	out_file.write((const char*)&bmfh.bfSize, sizeof (bmfh.bfSize  ));
	out_file.write((const char*)&bmfh.bfReserved1, sizeof (bmfh.bfReserved1  ));
	out_file.write((const char*)&bmfh.bfReserved2, sizeof (bmfh.bfReserved2  ));
	out_file.write((const char*)&bmfh.bfOffBits, sizeof (bmfh.bfOffBits  ));

	//alignment issues
	//out_file.write((const char*)&info, sizeof ( ZEDUS_BITMAPINFOHEADER ));	
	out_file.write((const char*)&info.biSize, sizeof ( info.biSize ));
	out_file.write((const char*)&info.biWidth, sizeof ( info.biWidth ));
	out_file.write((const char*)&info.biHeight, sizeof ( info.biHeight ));
	out_file.write((const char*)&info.biPlanes, sizeof ( info.biPlanes ));
	out_file.write((const char*)&info.biBitCount, sizeof ( info.biBitCount ));
	out_file.write((const char*)&info.biCompression, sizeof ( info.biCompression ));
	out_file.write((const char*)&info.biSizeImage, sizeof ( info.biSizeImage ));
	out_file.write((const char*)&info.biXPelsPerMeter, sizeof ( info.biXPelsPerMeter ));
	out_file.write((const char*)&info.biYPelsPerMeter, sizeof ( info.biYPelsPerMeter ));
	out_file.write((const char*)&info.biClrUsed, sizeof ( info.biClrUsed ));
	out_file.write((const char*)&info.biClrImportant, sizeof ( info.biClrImportant ));

	//now the pallete (seems that in 8bit / greyscale BMPs you have to add this)
	out_file.write((const char*)pallete, sizeof ( pallete ));

	//and finally the image data:

	static unsigned char* pData = NULL;
	static size_t s_data_size = 0;

	size_t required_data_size = line_bytes_on_file*height;//width*height*3;

	if (!pData || s_data_size < required_data_size)
	{
		delete [] pData;
		pData = new unsigned char[required_data_size];
		s_data_size = required_data_size;
	}

	unsigned char* pTravel = pData;

	for (int j=0;j<height;j++)
	{
		for (int i=0;i<width;i++)
		{			
			//pTravel[3*i] = Buffer[ ((height-1)*width) - (j*width) + i];
			//pTravel[3*i+1] = pTravel[3*i];
			//pTravel[3*i+2] = pTravel[3*i];
			pTravel[i] = Buffer[ ((height-1)*width) - (j*width) +i];
		}

		

		pTravel+= (width)+extra_bytes;
	}

	out_file.write((const char*)pData, line_bytes_on_file*height);

	delete [] pData;
	return true;
}

bool SaveBMP_Color( unsigned char* Buffer, int width, int height, const char *bmpfile )
{
	assert(0);
	printf("SaveBMP_Color is not supported yet!\n");
	return false;
}

/*
bool SaveBMP_Color( unsigned char* Buffer, int width, int height, LPCTSTR bmpfile )
{

//Buffer is an array that contains the image data, width and height are the dimensions of the image to save, and paddedsize is the size of Buffer in unsigned chars. bmpfile is the filename to save to.
//First we declare the header structs and clear them:

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	memset ( &bmfh, 0, sizeof (BITMAPFILEHEADER ) );
	memset ( &info, 0, sizeof (BITMAPINFOHEADER ) );

//Next we fill the file header with data:

	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + 
		sizeof(BITMAPINFOHEADER) + width*height*3;
	bmfh.bfOffBits = 0x36;

//and the info header:

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;	
	info.biBitCount = 24;
	//info.biBitCount = 8;
	info.biCompression = BI_RGB;	
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0x0ec4;  
	info.biYPelsPerMeter = 0x0ec4;     
	info.biClrUsed = 0;	
	info.biClrImportant = 0; 

//Some explanations: we want to save as a 24 bit RGB image, so we have to set biCompression to BI_RGB, biBitCount to 24 and biPlanes to 1.
//In 24 bit images we can set the biSizeImage value to 0 since it is ignored.
//For PelsPerMeter i simply use the values that Paint uses when saving bitmaps.
//Since we have no palette, we set the biClrUsed to 0, and biClrImportant being zero means that all colors are important.

//Now we can open a file to save to ( again i'm using windows functions but it doesnt matter what file I/O functions you use of course)

	HANDLE file = CreateFile ( bmpfile , GENERIC_WRITE, FILE_SHARE_READ,
		 NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( NULL == file )
	{
		CloseHandle ( file );
		return false;
	}

//Now we write the file header and info header:

	unsigned long bwritten;
	if ( WriteFile ( file, &bmfh, sizeof ( BITMAPFILEHEADER ), 
		&bwritten, NULL ) == false )
	{	
		CloseHandle ( file );
		return false;
	}

	if ( WriteFile ( file, &info, sizeof ( BITMAPINFOHEADER ), 
		&bwritten, NULL ) == false )
	{	
		CloseHandle ( file );
		return false;
	}

//and finally the image data:

	static unsigned char* pData = NULL;
	static size_t s_data_size = 0;

	size_t required_data_size = width*height*3;
	
	size_t line_size_on_file = width*3;

	size_t pad = required_data_size%4;
	if (pad > 0)
	{
		required_data_size+= 4-pad;
		line_size_on_file+= 4-pad;
	}

	if (!pData || s_data_size < required_data_size)
	{
		delete [] pData;
		pData = new unsigned char[required_data_size];
		s_data_size = required_data_size;
	}

	unsigned char* pTravel = pData;

	for (int j=0;j<height;j++)
	{
		for (int i=0;i<width;i++)
		{
			//unsigned int ind = ((height-1)*width) - (j*width) + i;
			pTravel[3*i] = Buffer[ ((height-1)*width*3) - (j*width*3) + i*3+2];
			pTravel[3*i+1] = Buffer[ ((height-1)*width*3) - (j*width*3) + i*3+1];
			pTravel[3*i+2] = Buffer[ ((height-1)*width*3) - (j*width*3) + i*3];
		}

		//pTravel+= width*3;
		pTravel+= line_size_on_file;		
	}

	//memcpy(pData, Buffer, required_data_size);

	if ( WriteFile ( file,pData, required_data_size, &bwritten, NULL ) == false )
	{	
		CloseHandle ( file );
		//delete [] pData;
		return false;
	}

//Now we can close our function with

	CloseHandle ( file );
	//delete [] pData;
	return true;
}
*/
