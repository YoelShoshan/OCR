// BoxOrganizer.cpp : Defines the entry point for the console application.
//

#include "BoxesCollection.h"
#include <stdio.h>
#include <stdlib.h> 

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		printf("Error! Usage requires input box file and output extension and 1/0!\n");
		printf("for example ""dummy.boxUnsorted_400_1200 boxFillMe 1"" means left-to-right\n");
		return 0;
	}

	int right_to_left = atoi(argv[3]);

	BoxesCollection* pBC = new BoxesCollection(argv[1], right_to_left==1);

	pBC->SortBoxes();
	pBC->SaveBoxes(argv[2]);

	return 0;
}

