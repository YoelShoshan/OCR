#ifndef ANN_UPSCALING_DEFINES_H
#define ANN_UPSCALING_DEFINES_H

#if defined(_MSC_VER)
#include <Windows.h>
#define ZEDUS_NN_API 
#elif defined(__GNUC__)

/*//typedef unsigned long       DWORD;
typedef unsigned int       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int        UINT;
*/


#else
#error define your copiler
#endif

#ifndef ZED_IN
#define ZED_IN
#endif

#ifndef ZED_OUT
#define ZED_OUT
#endif


#define FLIP_ENDIAN(x) \
	x = ((0x000000FF & x) << 24) | ((0x0000FF00 & x) << 8) | ((0x00FF0000 & x) >> 8) | ((0xFF000000 & x) >> 24)

#define SFMT_DOUBLE (((double)gen_rand32()/(double)((unsigned int)~((unsigned int)0))))
#define SFMT_IN_RANGE(start,end) ((SFMT_DOUBLE*((end)-(start)))+(start))

// between [a,b)

#define SFMT_IN_RANGE_UINT(start,end) (((gen_rand32())%((end)-(start)))+start)

#define TEST_CASES_PER_IMAGE 2000
//#define TEST_CASES_PER_IMAGE 50





/////////////////////////////////
// TODO list:
// pre-process the data!
/////////////////////////////////

//////////////////////////////////////////////////////////
// Neural Network structure 
// (input and output layer match the problem definition)

//#define HIDDEN_LAYER_SIZE 500
#define HIDDEN_LAYER_SIZE 200
//#define HIDDEN_LAYER_SIZE 5

#define HIDDEN_LAYERS_NUM 1


/////////////////////////
// Training process

#define MINI_BATCH_SIZE 200
//#define MINI_BATCH_SIZE 5
#define BACKPROP_WEIGHT_UPDATE_STEP 0.01

#define USE_GRADIENT_DESCEND
//#define USE_MOMENTUM
//#define USE_RPROP

////////////////////////////////
//  rprop params
#define RPROP_INITIAL_STEP_SIZE 0.0000001

#define RPROP_INCREASE_STEP_FACTOR 1.2
#define RPROP_DECREASE_STEP_FACTOR 0.5

#define RPROP_MAX_ALLOWED_STEP 0.03
#define RPROP_MIN_ALLOWED_STEP 0.000000001
////////////////////////////////

////////////////////////////////
// Momentum params
#define MOMENTUM_VISCOSITY_A 0.5
#define MOMENTUM_VISCOSITY_B 0.9
/////////////////////////////////

#ifdef USE_MOMENTUM
#define MUL_WITH_VISCOSITY * dViscosity
#else
#define MUL_WITH_VISCOSITY
#endif

////////////////////////////////////////////////////////////
// Few checks of improper combinations

#if defined(USE_MOMENTUM) && defined(USE_RPROP)
#pragma message("NN Configuration ERROR: Momentum and rprop cannot be combined.")
tralala = woopsydoo
#endif

#if defined(USE_RPROP) && defined(USE_GRADIENT_DESCEND)
#pragma message("NN Configuration ERROR: rprop and gradient-descend cannot be combined.")
tralala = woopsydoo
#endif

//////////////////////////
// Debug stuff

//#define ONLY_FEW_TEST_CASES_DEBUG
//#define DETERMINSTIC_SEED

#define DOUBLE_QNAN0 0x000000007FF80000
#define DOUBLE_IND00 0x00000000FFF80000
#define DOUBLE_INF00 0x000000007ff00000



#define UCHAR_TO_FLOAT_MIN_1_TO_1(c) ( (double(c) / (255.0 / 2.0)) - 1.0)
#define UCHAR_TO_FLOAT_0_TO_1(c) ( (double(c) / (255.0) ) )
#define FLOAT_TO_UCHAR(f) ( (unsigned char)(((f + 1.0)*0.5) * 255.0))

#define ZED_Y 0
#define ZED_U 1
#define ZED_V 2

#define SAFE_DEL(x) delete x; x=NULL;
#define SAFE_DEL_ARRAY(x) delete [] x; x=NULL;

#define ERROR_IF(cond, ...) \
	{ \
		if (cond) \
		{ \
			char text[1024]; \
			sprintf(text, __VA_ARGS__); \
			FPRINTF(text);\
			zosShowMessageBox(0,text);\
		} \
	}

#define ERROR_RETURN_FALSE_IF(cond, ...) \
	{ \
		if (cond) \
		{ \
			char text[1024]; \
			sprintf(text, __VA_ARGS__); \
			FPRINTF(text);\
			zosShowMessageBox(0,text);\
			return false; \
		} \
	}


#define ERROR_EXIT_IF(cond, ...) \
	{ \
		if (cond) \
		{ \
			char text[1024]; \
			sprintf(text, __VA_ARGS__); \
			FPRINTF(text);\
			zosShowMessageBox(0,text);\
			zosExitProcess(0);\
		} \
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// output only into file
/*#define ALLOC_LOG char log[1024*2];
#define SPRINTF(...) sprintf(__VA_ARGS__)
#define ODS(x,...) { fprintf(g_pLog,x,__VA_ARGS__); fflush(g_pLog);}
#define OUTPUT(...) { fprintf(g_pLog, __VA_ARGS__); fflush(g_pLog);}*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////
#define FPRINTF(...) { printf(__VA_ARGS__); fprintf(g_pLog,__VA_ARGS__);/* fflush(g_pLog);*/ }
#define LOG(...) { fprintf(g_pLog,__VA_ARGS__); /*fflush(g_pLog);*/ }
#define FLUSH() { fflush(g_pLog); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// no output
//#define SPRINTF(...) 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
