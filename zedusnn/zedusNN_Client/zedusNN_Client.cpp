// zedusNN_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "Trainer_GeneticAlgo.h"
#include <windows.h>
#include "../NeuralNetworkUpscaling.h"

//#include "../Trainer_BackProp.h"
#include <assert.h>
#include "../SFMT/SFMT.H"
//#include "../NN_Executer.h"
#include "Shlobj.h"
//#include "../Defines.h"
#include <conio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include "../zedusOS.h"

using namespace std;

EProblemType g_problem_type = PT_NOT_INIT;
char g_session_dir[1024];
//DWORD g_dwSeed;
//FILE* g_pLog = NULL;


//notice-  next are duplicates of code that exists inside zedusNN.dll

//#define FPRINTF(...) { printf(__VA_ARGS__); fprintf(g_pLog,__VA_ARGS__);/* fflush(g_pLog);*/ }
//#define LOG(...) { fprintf(g_pLog,__VA_ARGS__); /*fflush(g_pLog);*/ }
//#define FLUSH() { fflush(g_pLog); }

#define FPRINTF(...)
#define LOG(...)
#define FLUSH()

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
			zosExitProcess(0); \
		} \
	}

//type mnist mode evolve "C:\OCR\prev 3\for_bbox_detection\mnist_style.images" "C:\OCR\prev 3\for_bbox_detection\mnist_style.labels"
//type sparse-auto-encoder mode load "C:/ztemp/nn.org" "C:/db/3/half/0000.bmp" "C:/db/3/full"
//type mnist mode evolve "C:\db\ocr\1.images" "C:\db\ocr\1.labels"
//type mnist mode load  "C:\session_winners\PID_3936_0x0A404011\session_winner_PID_3936_0801_score_0.001_0.0061_In_784_Hidden_1_Hidden_Size_20_Output_10.org" "C:\db\ocr\1.images" "C:\db\ocr\1.labels"
//type mnist mode load-evolve "C:\session_winners\PREV_PID_2480_0x0A7D8FB7\session_winner_PID_2480_66301_score_270.720_7.8903_In_784_Hidden_1_Hidden_Size_20_Output_256.org" "C:\db\ocr\1.images" "C:\db\ocr\1.labels"

/*void PressAKeyToContinue()
{
	int c;
	printf( "\nPress a key to continue...\n" );
	c = getch();
	if (c == 0 || c == 224) getch();
}*/





	

void Print_Help()
{
	printf("NN.exe\n\n");
	printf("mode load [NN_file.org] [input_data] [output_data] //load NN and run a forward pass\n");
	//printf("mode load-score [NN_file.org] [input_data] [output_data] //load NN and calculate score\n");
	printf("mode load-evolve [NN_file.org] [input_data] [output_data] //load NN and evolve from that point\n");
	printf("mode evolve [input_data] [output_data] //evolve a new NN\n");
	printf("type [mnist/upscale/sparse-auto-encoder] \n");
	printf("for example: NN.exe type upscale mode load-evolve NN_file.org ""c:/db/4/half"" ""c:/db/4/full""  \n");
}

enum EConsoleCommand
{
	CC_NOT_INIT = 0,
	CC_Mode,
	CC_Type
};

enum EModeType
{
	MT_NOT_INIT = 0,
	MT_Load,
	//MT_LoadScore,
	MT_LoadEvolve,
	MT_Evolve
};



int main(int argc, char* argv[])
{				
	//g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	
	//zosShowMessageBox(0,"restore different random seed!",0,0);
	//init_gen_rand(0x12345678);


	//printf("NN_DoubleImage beta - notice - use only even dimensions.\n");

	CreateDirectory("C:/session_winners",NULL);
	
	

	EConsoleCommand console_cmd = CC_NOT_INIT;
	EModeType mode_type = MT_NOT_INIT;
	

	const char* NN_Path = NULL;
	const char* input_path = NULL;
	const char* output_path = NULL;	


	int i = 0;

	while(1)
	{
		i++;

		if (i ==  argc)
			break;

		if (i > argc)
		{
			printf("WTF shouldn't be here (Console command parsing).\n");
			zosExitProcess(0);
		}

		
		if (!strcmp(argv[i],"type"))
		{
			console_cmd = CC_Type;

			i++;

			if (!strcmp(argv[i],"mnist"))
			{
				g_problem_type = PT_Mnist;
				continue;
			}

			if (!strcmp(argv[i],"upscale"))
			{
				g_problem_type = PT_Upscale;
				continue;
			}

			if (!strcmp(argv[i],"sparse-auto-encoder"))
			{
				g_problem_type = PT_SparseAutoEncoder;
				continue;
			}

			printf("[%s] is an improper type! Must be one of [mnist/upscale/sparse-auto-encoder]\n",argv[i]);
			zosExitProcess(0);
			continue;
		}
				
		if (!strcmp(argv[i],"mode"))
		{
			console_cmd = CC_Mode;
			i++;

			if (!strcmp(argv[i],"load")) // NN.org input_path output_path
			{
				mode_type = MT_Load;

				if (! (i+3 < argc))
				{
					printf("Not enough args provided for load mode! Must provide [NN.org] [input_path] [output_path]\n");
					zosExitProcess(0);
				}

				i++;
				NN_Path = argv[i];

				i++;
				input_path = argv[i];

				i++;
				output_path = argv[i];

				continue;
			}

			if (!strcmp(argv[i],"load-evolve")) // NN.org input_path output_path
			{
				mode_type = MT_LoadEvolve;

				if (! (i+3 < argc))
				{
					printf("Not enough args provided for load-evolve mode! Must provide [NN.org] [input_path] [output_path]\n");
					zosExitProcess(0);
				}

				i++;
				NN_Path = argv[i];

				i++;
				input_path = argv[i];

				i++;
				output_path = argv[i];

				continue;
			}

			if (!strcmp(argv[i],"evolve")) // input_path output_path
			{
				mode_type = MT_Evolve;

				if (! (i+2 < argc))
				{
					printf("Not enough args provided for evolve mode! Must provide [input_path] [output_path]\n");
					zosExitProcess(0);
				}

				i++;
				input_path = argv[i];

				i++;
				output_path = argv[i];

				continue;
			}

			printf("[%s] is an improper mode! Must be one of [load/load-evolve/load-score/evolve]\n",argv[i]);
			zosExitProcess(0);
			continue;
		}		

	}


	if (MT_NOT_INIT == mode_type)
	{
		printf("There was no provided mode!");
		Print_Help();
		zosExitProcess(0);
	}

	if (PT_NOT_INIT == g_problem_type)
	{
		printf("There was no provided problem type!");
		Print_Help();
		zosExitProcess(0);
	}
	
	znnInit(g_problem_type, false, "C:/google_code");

	if (MT_Load == mode_type)
	{
		ERROR_EXIT_IF(!zosDoesFileExist(NN_Path), "Can't find the provided NN file [%s]!\n",NN_Path);		
		ERROR_EXIT_IF(!zosDoesFileExist(input_path), "Can't find the provided input file [%s]!\n",input_path);		

		znnLoadNN(NN_Path);
		znnProcess(input_path, output_path);
		
		return 0;
	}
			
	if ( (PT_Upscale == g_problem_type) ||
		(PT_SparseAutoEncoder == g_problem_type))
	{
		ERROR_EXIT_IF(!zosDoesDirectoryExist(input_path), "Invalid input directory [%s]!\n",input_path);	
		ERROR_EXIT_IF(!zosDoesDirectoryExist(output_path), "Invalid output directory [%s]!\n",output_path);
	} else if (PT_Mnist == g_problem_type)
	{
		ERROR_EXIT_IF(!zosDoesFileExist(input_path), "Invalid input file [%s]!\n",input_path);	
		ERROR_EXIT_IF(!zosDoesFileExist(output_path), "Invalid output file [%s]!\n",output_path);
	}		
	
	if (MT_LoadEvolve == mode_type)
	{
		bool res = znnTrainOnDB_StartFromProvidedNN(NN_Path,input_path, output_path);
		assert(res);
	}
			
	if (MT_Evolve == mode_type)
	{
		bool res = znnTrainOnDB(input_path, output_path);
		assert(res);
	}			

	zosPressAnyKeyToContinue();
	//PressAKeyToContinue();

	return 0;
}

