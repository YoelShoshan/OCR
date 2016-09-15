// BoxMerger.cpp : Defines the entry point for the console application.
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

	int boxes_num = -1;
	Box* pBoxes = pBC->GetBoxes(boxes_num);
	
	//Note:leaks.
	BoxTree *box_tree = new BoxTree(pBC->GetWidth(), pBC->GetHeight());
	int merged_boxes_num = -1;
	box_tree->Load(pBoxes,boxes_num);

	Box* merged_boxes = box_tree->GetBoxes(merged_boxes_num);

	pBC->SetBoxes(merged_boxes,merged_boxes_num);
		
	pBC->SaveBoxes(argv[2]);

	return 0;
}

