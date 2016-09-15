// NeuralNetworkUpscaling.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
//#include <windows.h>
#include "NeuralNetworkUpscaling.h"
#include "Trainer_BackProp.h"
#include <assert.h>
#include <SFMT.h>
#include "NN_Executer.h"
//#include "Shlobj.h"
#include "Defines.h"
//#include <conio.h>
#include <stdio.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <string.h>
#include "zedusOS.h"

bool g_bInitializedLib = false;
char g_session_dir[1024];
//EProblemType g_problem_type = PT_NOT_INIT;
bool g_dbg = false;
FILE* g_pLog = NULL;
unsigned int g_dwSeed;

NeuralNetwork* g_pNN = NULL;
EProblemType g_prob_type = PT_NOT_INIT;

char g_String[1024];// <- lol

std::string dirnameOf(const std::string& fname)
{
     size_t pos = fname.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : fname.substr(0, pos);
}

/*bool CheckIfFileExists(const char* path)
{
	struct stat s;
	if( stat(path,&s) == 0 )
	{
		if( s.st_mode & S_IFREG )
		{
			return true;
		}
		return false;
	}

	return false;
}*/

bool CheckIfLibIsInitialized()
{
	assert(g_bInitializedLib);
	if (!g_bInitializedLib)
	{
		zosShowMessageBox(0,"You must initialized the lib by calling ""znnInit"" first!");
		return false;
	}

	return true;
}

ZEDUS_NN_API int Test(int a, int b)
{
	return a+b;
}

ZEDUS_NN_API const char* znnInit(EProblemType problem_type, bool deterministic_seed, const char* home_path)
{
	printf("znnInit().\n");
	//zosShowMessageBox(0,"znnInit",0,0);
	//assert(session_dir);

	g_prob_type = problem_type;

	//

	/*char* dir_prefix = "";
	if (MT_Load == mode_type)
	{
		dir_prefix = "load_";
	} else if (MT_Evolve == mode_type)
	{
		dir_prefix = "evolve_";
	} else if (MT_LoadEvolve == mode_type)
	{
		dir_prefix = "load_evolve_";
	}*/	


	//TODO: create C:/session_winners if it doesn't exist!!

	static unsigned int simulated_time = 0;
	simulated_time++;

/*#if defined(_MSC_VER)
	const char* winners_root_path = "C:/session_winners";
#elif defined(__GNUC__)
	const char* winners_root_path = "/home/yoel/session_winners";
#else
#error define your copiler
#endif*/
	char winners_root_path[1024];
	sprintf(winners_root_path, "%s/session_winners",home_path);

	printf("About to create dir [%s]\n",winners_root_path);
	zosCreateDirectory(winners_root_path/*,NULL*/);

	sprintf(g_session_dir,"%s/PID_%d_0x%08X", winners_root_path,zosGetProcessID(),simulated_time/*GetTickCount()*/);
	printf("About to create dir [%s]\n",g_session_dir);
	zosCreateDirectory(g_session_dir/*,NULL*/);

	
	std::string log_file_name(g_session_dir);
	log_file_name+="/NN_Log.txt";
	g_pLog = fopen(log_file_name.c_str(),"w");
	
	g_bInitializedLib = true;

	g_dwSeed = simulated_time;//GetTickCount();

	if (deterministic_seed)
	{
		g_dwSeed = 0x12345678;
		zosShowMessageBox(0,"Notice! Fixed seed used! Should be used for debugging only.");
	}


	FPRINTF("Initialized rand with seed 0x%08X\n", g_dwSeed);
	//printf("Initialized rand with seed 0x%08X\n", g_dwSeed);

	init_gen_rand(g_dwSeed);

	printf("Reached znnInit() end.\n");
	return g_session_dir;
}


ZEDUS_NN_API bool znnLoadNN(const char* nn_file)
{
	printf("znnLoadNN()\n");
	if (!CheckIfLibIsInitialized())
		return false;
	delete g_pNN;
	g_pNN = new NeuralNetwork(nn_file);
	printf("Reached znnLoadNN() end.\n");		
	return true;
}

ZEDUS_NN_API const char* znnProcess(const char* input_path, const char* output_path)
{
	printf("znnProcess()\n");
	if (!CheckIfLibIsInitialized())
	{
		printf("znnProcess:: Not initialized!\n");
		return NULL;
	}
	if (!g_pNN)
	{
		zosShowMessageBox(0,"You must load a neural network by calling ""znnLoadNN"" before calling  ""znnProcess"" !");		
		return NULL;
	}
	
	switch (g_prob_type)
	{
		case PT_Upscale:
			NN_Executer::OutputResult_Upscaling(g_pNN, input_path, output_path);
			printf("Reached znnProcess end.\n");
			return NULL;
		break;

		case PT_SparseAutoEncoder:
			NN_Executer::OutputResult_SparseAutoEncoder(g_pNN, input_path, output_path);
			printf("Reached znnProcess end.\n");
			return NULL;
		break;

		case PT_Mnist:
			{
				//ERROR_EXIT_IF(1,"mnist isn't supported yet!");
				//assert(0);
				//DebugBreak(); //didn't fill the results file name yet.
				//NN_Executer::OutputResult_Mnist(pNN, input_path, output_path, "C:/temp/tmp");

				//sprintf(result_image,"%s/PID_0x%08X - loaded_org.AI_RESULT",g_session_dir,zosGetProcessID());

				//zosShowMessageBox(0,"Got here!",0,0);
				std::string only_path_from_src_file = dirnameOf(input_path);
				sprintf(g_String,"%s/PID_0x%08X - loaded_org.AI_RESULT",only_path_from_src_file.c_str(),zosGetProcessID());		

				NN_Executer::OutputResult_Mnist(g_pNN, input_path, output_path, g_String);

				printf("Reached znnProcess end.\n");
				return &g_String[0];
			}

		break;

		default:
			ERROR_EXIT_IF(1,"Unkown problem type! Supported options are [upscale/sparse_auto_encoder/mnist]");
	}

	printf("Reached znnProcess end. (But didn't enter the switch!\n");
	return NULL;
}

ZEDUS_NN_API bool znnProcessSingleCase(unsigned int inputs_num, double* inputs, unsigned int outputs_num,double* outputs)
{
	printf("znnProcessSingleCase()\n");
	if (!CheckIfLibIsInitialized())
	{
		printf("znnProcessSingleCase:: Lib it not initialized!\n");
		return false;
	}	
	g_pNN->Forward_Pass(inputs_num, inputs,outputs_num,outputs);
	return true;
}

ZEDUS_NN_API bool znnTrainOnDB(const char* input_path, const char* output_path)
{
	printf("znnTrainOnDB()\n");
	if (!CheckIfLibIsInitialized())
	{
		printf("znnTrainOnDB:: Lib it not initialized!\n");
		return false;
	}
	Trainer_BackProp* pTrainer = new Trainer_BackProp(g_prob_type,input_path, output_path);					
	pTrainer->Evolve();	
	delete pTrainer;
	pTrainer = NULL;
	printf("Reached znnTrainOnDB() end.\n");
	return true;
}

ZEDUS_NN_API bool znnTrainOnDB_StartFromProvidedNN(const char* nn_file,const char* input_path, const char* output_path)
{
	printf("znnTrainOnDB_StartFromProvidedNN()\n");
	if (!CheckIfLibIsInitialized())
	{
		printf("znnTrainOnDB_StartFromProvidedNN:: Lib it not initialized!\n");
		return false;
	}
	if (!zosDoesFileExist(nn_file))
	{
		char msg[1024];
		sprintf(msg,"Can't find the provided NN file [%s]!\n",nn_file);
		zosShowMessageBox(0,msg);
		printf("znnTrainOnDB_StartFromProvidedNN:: file doesn't exist!\n");
		return false;
	}
	
	Trainer_BackProp* pTrainer = new Trainer_BackProp(g_prob_type,input_path, output_path);	

	double score = pTrainer->LoadFromFile(nn_file,true, NULL);

	printf("Loaded organism [%s] - score on data:[%s] labels:[%s] is %.3f\n",
		nn_file, input_path, output_path, score);

	pTrainer->Evolve();	
	delete pTrainer;
	pTrainer = NULL;
	printf("Reached znnTrainOnDB_StartFromProvidedNN end.\n");
	return true;
}
