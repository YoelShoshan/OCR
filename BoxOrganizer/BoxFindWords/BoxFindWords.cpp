// BoxFindWords.cpp : Defines the entry point for the console application.
//

#include "../BoxLib/Boxes.h"
#include "../BoxLib/BoxTree.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Error! Usage requires input box file and output extension!\n");
		return 0;
	}

	Boxes* pBC = new Boxes(argv[1]);
		
	char words_box_file_extension[1024];
	sprintf(words_box_file_extension, "%d_%d_boxFullWords", pBC->GetWidth(), pBC->GetHeight());

	pBC->FindWords(words_box_file_extension);
		
	pBC->SaveBoxes(argv[2]);

	return 0;
}

