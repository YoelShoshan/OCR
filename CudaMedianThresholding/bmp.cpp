#include "bmp.h"
#include "stdio.h"

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

BYTE YUV_TO_R(BYTE y, BYTE u, BYTE v)
{
	double dY = double(y) - 16.0;
	double dU = double(u) - 128.0;
	double dV = double(v) - 128.0;

	double R = dY + 1.403*dV;
	CLAMP(R,0,255)
	return R;
}

BYTE YUV_TO_G(BYTE y, BYTE u, BYTE v)
{
	double dY = double(y) - 16.0;
	double dU = double(u) - 128.0;
	double dV = double(v) - 128.0;

	double G = dY-0.344*dU-0.714*dV;
	CLAMP(G,0,255)
	return G;
}

BYTE YUV_TO_B(BYTE y, BYTE u, BYTE v)
{
	double dY = double(y) - 16.0;
	double dU = double(u) - 128.0;
	double dV = double(v) - 128.0;

	double B = dY+1.77*dU;
	CLAMP(B,0,255)
	return B;
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
unsigned char* LoadBMP(const char* image_name, UINT& width, UINT& height, UINT& components_num)
{
	char caError[200];
	HBITMAP hBMP;														// Handle Of The Bitmap
	BITMAP	BMP;														// Bitmap Structure


	hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL), 
		image_name, IMAGE_BITMAP, 
		0, 
		0, 
		LR_CREATEDIBSECTION | LR_LOADFROMFILE );

	if (!hBMP)															// Does The Bitmap Exist?
	{
		sprintf(caError,"Unable to load BMP File:%s",image_name);
		MessageBoxA(0,caError,0,0);
		return NULL;
	}

	GetObject(hBMP, sizeof(BMP), &BMP);									// Get The Object
	// hBMP:        Handle To Graphics Object
	// sizeof(BMP): Size Of Buffer For Object Information
	// &BMP:        Buffer For Object Information
	
	width = BMP.bmWidth;
	height = BMP.bmHeight;
	
	//pImage->data  = BMP->bmBits;

	components_num = BMP.bmBitsPixel/8;

	/*
	//padding

	 // some preliminaries
 
	 double dBytesPerPixel = ( (double) BitDepth ) / 8.0;
	 double dBytesPerRow = dBytesPerPixel * (Width+0.0);
	 dBytesPerRow = ceil(dBytesPerRow);
  
	 int BytePaddingPerRow = 4 - ( (int) (dBytesPerRow) )% 4;
	 if( BytePaddingPerRow == 4 )
	 { BytePaddingPerRow = 0; } 
 
	 double dActualBytesPerRow = dBytesPerRow + BytePaddingPerRow;
	*/

	unsigned int line_bytes = BMP.bmWidth * components_num;

	unsigned int padding = line_bytes%4;

	unsigned int line_bytes_on_file = line_bytes;
	if (padding > 0)
	{
		line_bytes_on_file += 4-padding;
	}


	unsigned char* orig_data = (unsigned char*) BMP.bmBits;
	unsigned char* data = new unsigned char [BMP.bmWidth * BMP.bmHeight * components_num];

	unsigned char* pTravel_Data = data;

	// flip height
	for (int i=0;i<BMP.bmHeight;i++)
	{
		/*if (i==BMP.bmHeight-1)
		{
			char d = 9;
		}
		printf("About to do line %d\n", i);*/
		//memcpy(pTravel_Data, &orig_data[ (BMP.bmHeight-1-i)*BMP.bmWidthBytes]  , BMP.bmWidthBytes);
		memcpy(pTravel_Data, &orig_data[ (BMP.bmHeight-1-i)*line_bytes_on_file]  , line_bytes);

		if (3 == components_num )
		{
			// convert from bgr to rgb
			for (int j=0;j<BMP.bmWidth;j++)
			{
				unsigned char temp;
				temp = pTravel_Data[j*3];
				pTravel_Data[j*3] = pTravel_Data[j*3+2];
				pTravel_Data[j*3+2] = temp;			
			}
		}


		pTravel_Data+= line_bytes;
		//pTravel_Data+= BMP.bmWidthBytes;
	}

	//memcpy(data,BMP.bmBits,BMP.bmWidthBytes * BMP.bmHeight);

	DeleteObject(hBMP);													// Delete The Object

	return data;
}

bool SaveBMP_GreyScale( BYTE* Buffer, int width, int height, LPCTSTR bmpfile )
{
	unsigned int line_bytes = width;
	unsigned int padding = line_bytes%4;

	unsigned int line_bytes_on_file = line_bytes;

	unsigned int extra_bytes = (4-padding)%4;

	if (padding > 0)
	{
		line_bytes_on_file += extra_bytes;
	}	

//Buffer is an array that contains the image data, width and height are the dimensions of the image to save, and paddedsize is the size of Buffer in bytes. bmpfile is the filename to save to.
//First we declare the header structs and clear them:

	RGBQUAD pallete[256];
	for (int i=0;i<256;i++)
	{
		pallete[i].rgbRed=i;
		pallete[i].rgbGreen=i;
		pallete[i].rgbBlue=i;
		pallete[i].rgbReserved=0;
	}

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	memset ( &bmfh, 0, sizeof (BITMAPFILEHEADER ) );
	memset ( &info, 0, sizeof (BITMAPINFOHEADER ) );

//Next we fill the file header with data:

	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = 0;//sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width*height;
	bmfh.bfOffBits = sizeof(bmfh)+sizeof(info)+sizeof(pallete);//0x36; // offset to the beginning of the data

//and the info header:

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;	
	//info.biBitCount = 24;
	info.biBitCount = 8;
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0;//0x0ec4;  
	info.biYPelsPerMeter = 0;//0x0ec4;     
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

//now the pallete (seems that in 8bit / greyscale BMPs you have to add this)

	if ( WriteFile ( file, pallete, sizeof ( pallete ), 
		&bwritten, NULL ) == false )
	{	
		CloseHandle ( file );
		return false;
	}

//and finally the image data:

	static BYTE* pData = NULL;
	static size_t s_data_size = 0;

	size_t required_data_size = line_bytes_on_file*height;//width*height*3;

	if (!pData || s_data_size < required_data_size)
	{
		delete [] pData;
		pData = new BYTE[required_data_size];
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

	/*for (int i=0;i<width*height;i++)
	{
		pData[3*i] = Buffer[i];
		pData[3*i+1] = Buffer[i];
		pData[3*i+2] = Buffer[i];
		//pData[3*i+1] = 0;
		//pData[3*i+2] = 0;
	}*/

	if ( WriteFile ( file, /*Buffer*/pData, /*width*height*3*/line_bytes_on_file*height, &bwritten, NULL ) == false )
	{	
		DWORD last_error = GetLastError();
		printf("Last error=0x%08X\n", last_error);
		CloseHandle ( file );
		//delete [] pData;
		return false;
	}

//Now we can close our function with

	CloseHandle ( file );
	//delete [] pData;
	return true;
}

bool SaveBMP_Color( BYTE* Buffer, int width, int height, LPCTSTR bmpfile )
{

//Buffer is an array that contains the image data, width and height are the dimensions of the image to save, and paddedsize is the size of Buffer in bytes. bmpfile is the filename to save to.
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

	static BYTE* pData = NULL;
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
		pData = new BYTE[required_data_size];
		s_data_size = required_data_size;
	}

	unsigned char* pTravel = pData;

	for (int j=0;j<height;j++)
	{
		for (int i=0;i<width;i++)
		{
			//UINT ind = ((height-1)*width) - (j*width) + i;
			pTravel[3*i] = Buffer[ ((height-1)*width*3) - (j*width*3) + i*3+2];
			pTravel[3*i+1] = Buffer[ ((height-1)*width*3) - (j*width*3) + i*3+1];
			pTravel[3*i+2] = Buffer[ ((height-1)*width*3) - (j*width*3) + i*3];
		}

		//pTravel+= width*3;
		pTravel+= line_size_on_file;		
	}

	//memcpy(pData, Buffer, required_data_size);

	if ( WriteFile ( file, /*Buffer*/pData, required_data_size/*width*height*3*/, &bwritten, NULL ) == false )
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