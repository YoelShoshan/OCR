#ifndef ZEDUS_NN_H
#define ZEDUS_NN_H

//#include <windows.h>


//When i build the NN dll, i define the export define.
//When a user of the lib uses it, it's not defined, and then the import is used instead.

#if defined(_MSC_VER)


#ifdef ZEDUS_NN_EXPORTS
#define ZEDUS_NN_API extern "C" __declspec(dllexport)
#else
#define ZEDUS_NN_API extern "C" __declspec(dllimport)
#endif

#elif defined(__GNUC__)

#define ZEDUS_NN_API

#endif

enum EProblemType
{
	PT_NOT_INIT = 0,
	PT_Mnist,
	PT_Upscale,
	PT_SparseAutoEncoder
};

#define ZNN_IN
#define ZNN_OUT

/////////////////////////////////////////////////
// For python binding i need:
// 1. load an existing neural network
// 2. Make it process mnist files
// 3. Make it process a specific test case
/////////////////////////////////////////////////

ZEDUS_NN_API int Test(int a, int b);


ZEDUS_NN_API const char* znnInit(EProblemType problem_type, bool deterministic_seed, const char* home_path);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loads a NN from file.
// For example: "session_winner_PID_5944_2001_score_171.123_1.0107_In_784_Hidden_1_Hidden_Size_200_Output_124.org"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZEDUS_NN_API bool znnLoadNN(const char* nn_file);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process a full database.
// output_path is optional - TODO: make sure what is the expected behavior with and w/o output_path
// output_path should be used only for score calculation and debug outputs.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ZEDUS_NN_API const char* znnProcess(const char* input_path, const char* output_path);

////////////////////////////
// Process a single case
////////////////////////////
ZEDUS_NN_API bool znnProcessSingleCase(unsigned int inputs_num, ZNN_IN double* inputs, unsigned int outputs_num, ZNN_OUT double* outputs); //perhaps add some validation of the length ?


////////////////////////////////
// Trains on a given database
////////////////////////////////
ZEDUS_NN_API bool znnTrainOnDB(const char* input_path, const char* output_path);

//////////////////////////////////////////////////////////////////////////
// Trains on a given database, starting from a given NN
//////////////////////////////////////////////////////////////////////////
ZEDUS_NN_API bool znnTrainOnDB_StartFromProvidedNN(const char* nn_file,const char* input_path, const char* output_path);


#endif
