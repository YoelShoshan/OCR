#include "Boxes.h"
#include <vector>
#include <assert.h>
#include <algorithm>
#include <map>
#include <stdio.h>
//#include <string>
#include <cstring> 
//#include "BoxTree.h"


#define DEBUG_PRINT(...)
//#define DEBUG_PRINT(...) printf(__VA_ARGS__)

//using namespace utf8util;
using namespace std;

void tokenize_line(string& line, vector<string>& tokens)
{
	size_t length = line.length();
	vector<int> spaces;
	for (int i=0;i<length;i++)
	{
		if (line[i] == L' ' || line[i] == L'\r')
		{
			spaces.push_back(i);
		}
	}
	
	size_t spaces_num = spaces.size();
	for (int i=0;i<spaces_num;i++)
	{
		int start = 0;
		if (i>0)
		{
			start = spaces[i-1];
		}
		int len = spaces[i] - start;
		tokens.push_back( line.substr(start,len));
	}
}



Boxes::Boxes(const char* file_name) : m_pBoxes(NULL)
{
	assert(file_name);
	m_uiBoxesNum = 0;

	m_iWordsNum = 0;

	// get resolution from file name - "0_res2.box_333_890"

	string name(file_name);

	size_t last_dot_pos = name.find_last_of(".");
	assert(last_dot_pos);

	string extension = name.substr(last_dot_pos+1, name.length()-1);
	
	printf("extension=[%s]",extension.c_str());

	//string extension = name.substr(last_dot_pos+1+strlen(".boxUnsorted"), name.size() - last_dot_pos-1-strlen(".boxUnsorted"));

	size_t underline_pos = extension.find_first_of("_");
	
	string rest_of_extension = extension.substr(underline_pos+1,extension.length()-1);
	printf("rest_of_extension=[%s]",rest_of_extension.c_str());
	size_t second_underline_pos = rest_of_extension.find_first_of("_");

	string width_str = extension.substr(0,underline_pos);
	printf("width_str=[%s]",width_str.c_str());
	string height_str = rest_of_extension.substr(0,second_underline_pos);
	printf("height_str=[%s]",height_str.c_str());

	m_iWidth = atoi(width_str.c_str());
	m_iHeight = atoi(height_str.c_str());

	printf("Width=%d, Height=%d\n", m_iWidth, m_iHeight);

	if (m_iWidth == 0 || m_iHeight == 0)
	{
		//MessageBoxA(0,"error on extracting width/height!",0,0);
		
("Error","error on extracting width/height!");
	}

	m_pBoxFileName = new char[strlen(file_name)+1];
	strcpy(m_pBoxFileName,file_name);
	//FILE*f = _wfopen(file_name, L"rt, ccs=UTF-8");

	//FILE*f = _tfopen( _T("testo.txt"), _T("r") );
	//FILE*f = _tfopen( file_name, _T("r") );	
	FILE*f = fopen(file_name,"r");

	if (!f)
	{		
		char error_text[512];
		sprintf(error_text,"Error! can't open file [%s].", file_name);
		//MessageBoxA(0,temp,0,0);
		zedus_error_message_box("Error",error_text);

	}

	vector<Box> boxes;

	static const int lineChars = 1024;
	char line[ lineChars ];

	//for (int i=0;i<10;i++)
	while (fgets(line,lineChars,f))
	{		
		//convert from utf-8 (the way the file is stored) to utf-16 (the way that windows API works)
		string str_UTF8 = string(line);

		vector<string> tokens;

		tokenize_line(str_UTF8, tokens);
		
		Box box(
			atoi(tokens[1].c_str()),
			atoi(tokens[2].c_str()),//m_iHeight-atoi(tokens[2].c_str())-1,
			atoi(tokens[3].c_str()),
			atoi(tokens[4].c_str()),//m_iHeight-atoi(tokens[4].c_str())-1,//atoi(tokens[4].c_str()),
			tokens[0],
			atoi(tokens[5].c_str()),
			atoi(tokens[6].c_str()));

		/*Box box(
			atoi(tokens[1].c_str()),
			atoi(tokens[2].c_str()),
			atoi(tokens[3].c_str()),
			atoi(tokens[4].c_str()),
			tokens[0],
			9999);*/

		boxes.push_back(box);

		//wstring str_UTF16 = utf16_from_utf8(str_UTF8);
		//MessageBoxW(0,str_UTF16.c_str(),0,0);
	}
	
	fclose(f);

	m_uiBoxesNum = boxes.size();

	m_pBoxes = new Box[m_uiBoxesNum];
	unsigned int index = 0;
	for (vector<Box>::iterator it = boxes.begin(); it!= boxes.end(); ++it)
	{
		m_pBoxes[index] = *it;
		index++;
	}

}

Boxes::~Boxes()
{
	delete [] m_pBoxes;
	m_pBoxes = NULL;

}

void Boxes::FindWord(Box* box)
{   
    //#this bbox already assigned a wordNumber
    if (box->assigned)
        return;
    
    box->wordNum = m_iWordsNum;
	box->assigned = true;
    
	for (int b=0;b<m_uiBoxesNum;b++)
	{
        if (!m_pBoxes[b].assigned)
		{
			if (AreBoxesClose_2(*box, m_pBoxes[b]))
			{
                FindWord(&m_pBoxes[b]);
			}
		}
	}
}

//#include <Windows.h>

void Boxes::FindWords(const char* extension)
{
	//MessageBoxA(0,0,0,0);
	while (true)
	{
		bool found_numberless_word = false;
		//for b in xrange(boxes_num):
		for (int b=0;b<m_uiBoxesNum;b++)
		{
			if (!m_pBoxes[b].assigned)
			{
				found_numberless_word = true;
				FindWord(&m_pBoxes[b]);
				//#print "Assigned %d wordNumber" % g_word_counter
				m_iWordsNum++;
			}
		}

		if (!found_numberless_word)
		{
			break;
		}
	}

	//vector<Box> word_boxes;
	//word_boxes.reserve(m_iWordsNum);

	int minimum_word_total_pixel_count = int(float(m_iWidth) / 15.f);
	printf("minimum pixel count to keep word bbox is %d\n",minimum_word_total_pixel_count);

	Box *words_boxes = new Box[m_iWordsNum];

	//for i in xrange(g_word_counter):
	for (int i=0;i<m_iWordsNum;i++)
	{
		//printf("i=%d\n", i);
        bool first = true;
        //merged_box = None
		Box word_box;
        //for b in boxes:
		for (int b=0;b<m_uiBoxesNum;b++) //OPTIMIZE: when assigning the words, already create word buckets
		{
			if (b==5)
			{
				int d = 0;
			}

			//printf("b=%d\n", b);
			Box* pCharBox = &m_pBoxes[b];
            if (pCharBox->wordNum == i)
			{
                if (first)
				{
					words_boxes[pCharBox->wordNum] = (*pCharBox);
                    //merged_box = b.clone()
                    first = false;
				}
                else
				{
                    //merged_box.merge_with(b);
					words_boxes[pCharBox->wordNum].MergeWith(*pCharBox);
				}
			}
		}
            
		//#discard word bbox if it's too small
		float area = (word_box.x2-word_box.x1+1)*(word_box.y2-word_box.y1+1);
		if (area < minimum_word_total_pixel_count)
		{
			printf("discarded for too small word pixel count (%f) min allowed is %d\n",area, minimum_word_total_pixel_count);
			word_box.Print();
			//print merged_box
			//mark_box_on_image(im_data,merged_box,(255,0,0))
			continue;
		}

		//word_boxes.push_back(word_box);
	 }


	 //for (vector<Box>::iterator it = word_boxes.begin(); it!=word_boxes.end();++it)
	for (int b=0;b<m_iWordsNum;b++)
	 {
		 if ( words_boxes[b].y1 > words_boxes[b].y2)
		 {
			 printf("Error! b.y1 > b.y2 !!! ");
			 words_boxes[b].Print();
		 }
	 }

	/*
	//Didn't port yet.
	for b in merged_boxes:
        find_chars_in_word(b,im_data,bw_im_with_read_write_access_data)
        #print "dbg"
	*/

	//zedusBoxes.save_box_file(box_file+".%d_%d_boxFullWords" % (im_width, im_height), merged_boxes, (im_width, im_height))

	SaveBoxesInternal(extension,words_boxes,m_iWordsNum,true);


	//Now, based on the word bboxes, discard char boxes that aren't inside the word bboxes, and also stretch certain cases
	
	for (int w=0;w<m_iWordsNum;w++)
	{
		Box *word_box = &words_boxes[w];
		for (int c=0;c<m_uiBoxesNum;c++)
		{
			Box* char_box = &m_pBoxes[c];
			if (char_box->selected)
			{
				continue;
			}

			//if check_intersection(w,c) == True:
			if (word_box->DoesIntersectWith(*char_box))
			{
				char_box->selected = true;
				//char_box->wordNum = word_box->wordNum;
				char_box->groupNum = word_box->groupNum;
                
				float height = char_box->y2-char_box->y1;
				if (height < float(word_box->y2-word_box->y1) * 0.65f)
				{
					//stretch_height(c, w)
					char_box->StretchMyHeightTo(*word_box);

					//if, because of the stretch I now intersect with another box, i need to merge into it

					for (int z=0;z<m_uiBoxesNum;z++)
					{
						Box* potential_collide_b = &m_pBoxes[z];
						if (potential_collide_b == char_box)
						{
							continue;
						}

						if (char_box->DoesIntersectWith(*potential_collide_b))
						{
							char_box->MergeWith(*potential_collide_b);
							potential_collide_b->ignore = true;
							//potential_collide_b->x1 = 0;
							//potential_collide_b->x2 = 1;
							//potential_collide_b->y1 = 0;
							//potential_collide_b->x2 = 1;
						}
					}		

				}
				//curr_word.append(c)                
			}
			//res.extend(curr_word)    
		}
	}

	//now merge for the cases that we stretch height of things that were already the same char

	/*BoxTree *box_tree = new BoxTree(GetWidth(), GetHeight());
	int merged_boxes_num = -1;
	box_tree->Load(m_pBoxes,m_uiBoxesNum);

	Box* merged_boxes = box_tree->GetBoxes(merged_boxes_num);
	SetBoxes(merged_boxes,merged_boxes_num);*/
}


float Boxes::CalcBoxesDistance(IN Box &b1, IN Box &b2, IN OUT Vec2D &dir)
{
	if (b2.x1 > b1.x2) //:#if b2 is completely on the right side of b1
	{
        if (b2.y1 > b1.y2) //: #and under
		{
			dir.Set(b2.x1 - b1.x2 , b2.y1 - b1.y2);
			dir.Normalize();
			//dir_x = 
            return sqrt( float((b2.x1-b1.x2)*(b2.x1-b1.x2) + (b2.y1-b1.y2)*(b2.y1-b1.y2)));
		}
        if (b2.y2 < b1.y1) //: #and above
		{
			dir.Set(b2.x1 - b1.x2 , b2.y2 - b1.y1);
			dir.Normalize();
            return sqrt( float((b2.x1-b1.x2)*(b2.x1-b1.x2) + (b2.y2-b1.y1)*(b2.y2-b1.y1)) );
		}

		dir.Set(b2.x1 - b1.x2 , 0.0);
		dir.Normalize();
        return sqrt (float( (b2.x1-b1.x2)*(b2.x1-b1.x2) )); 
	}
        
    //#if b2 is completely on the left side of b1
    if (b2.x2 < b1.x1) //:
	{
        if (b2.y1 > b1.y2) //: #and under
		{
			dir.Set(b2.x2 - b1.x1 , b2.y1 - b1.y2);
            dir.Normalize();
            return sqrt(float( (b2.x2-b1.x1)*(b2.x2-b1.x1) + (b2.y1-b1.y2)*(b2.y1-b1.y2)) ); 
		}
        if (b2.y2 < b1.y1) //: #and above
		{
			dir.Set(b2.x2 - b1.x1 , b2.y2 - b1.y1);
            dir.Normalize();
            return sqrt(float( (b2.x2-b1.x1)*(b2.x2-b1.x1) + (b2.y2-b1.y1)*(b2.y2-b1.y1)) ); 
		}
        
		dir.Set(b2.x2 - b1.x1 , 0.0);
        dir.Normalize();
        return  sqrt (float( (b2.x2-b1.x1)*(b2.x2-b1.x1) ));
	}
        
    if (b2.y2 < b1.y1) //: #above but not completely to the left or right
	{
		dir.Set(0.0 , b2.y2-b1.y1);
        dir.Normalize();
        return sqrt (float( (b2.y2-b1.y1)*(b2.y2-b1.y1)) ); 
	}
        
    if (b2.y1 > b1.y2) //: #below but not completely to the left or right
	{
		dir.Set(0.0 , b2.y1-b1.y2);
        dir.Normalize();
        return sqrt (float( (b2.y1-b1.y2)*(b2.y1-b1.y2) )); 
	}
    
    //#if we reached here, it means the boxes intersect!
	dir.Set(0.f,0.f);
    return 0.f;

}

bool Boxes::AreBoxesClose_2(Box &b1, Box &b2)
{
	//simple y distance first:
	if (fabs( ((b1.y1+b1.y2)/2) - ((b2.y1+b2.y2)/2)) > (float(m_iWidth) / 10.f))
	{
		return false;
	}

	Vec2D direction;
	float dist = CalcBoxesDistance(b1,b2,direction);
       
	Vec2D left(-1.0,0.0);
    float ang_deg = left.GetAngleWith(direction);
	
	float factor_based_on_box_height = 1.0;

	int max_distance = int(float(m_iWidth) * 0.015f * factor_based_on_box_height);

	if (dist > max_distance)
	{
        return false;
	}

	float miftah = 32.0;
    
    if (ang_deg > miftah && ang_deg < 180.0 - miftah)
	{
        return false;
	}

	Vec2D center_b1( b1.x1 + float(b1.x2-b1.x1)/2, b1.y1 + float(b1.y2-b1.y1)/2);
    Vec2D center_b2( b2.x1 + float(b2.x2-b2.x1)/2, b2.y1 + float(b2.y2-b2.y1)/2);

	Vec2D centers_dir(center_b2.x-center_b1.x, center_b2.y-center_b1.y);
	centers_dir.Normalize();
	    

    float center_vec_ang_deg = left.GetAngleWith(centers_dir);
		
	float miftah_2 = 38.0;
    
    if (center_vec_ang_deg > miftah_2 && center_vec_ang_deg < 180.0 - miftah_2)
	{
        return false;
	}

	return true;
}

void Boxes::SaveBoxesInternal(const char* extension, Box* boxes, int boxes_num, bool withGroupNums)
{
	//m_pBoxFileName
	
	//20120913_221054.jpg_cropped.boxUnsorted_693_1553
	
	string file_name(m_pBoxFileName);

	size_t last_dot_pos = file_name.find_last_of(".");
	string before_extension = file_name.substr(0,last_dot_pos+1);
	before_extension += extension;//".boxFillMe";

	//FILE*f = fopen("C:/temp/1.box","w");
	FILE*f = fopen(before_extension.c_str(),"w");

	assert(f);
	char line[1024];
	//for (unsigned int i=0;i<m_uiBoxesNum;i++)
	//for (std::vector<Box>::iterator it = m_SortedBoxes.begin(); it!= m_SortedBoxes.end(); ++it)
	for (int i=0;i<boxes_num;i++)
	{
		Box* b = &boxes[i];
		if (b->ignore)
		{
			continue;
		}

		int groupNum = 0;
		if (withGroupNums)
		{
			groupNum = b->groupNum;
		}

		sprintf(line,"%s %d %d %d %d %d %d\r\n",
			//m_pBoxes[i].character.c_str(),
			b->character.c_str(),
			int(b->x1),						
			int(b->y1),//m_iHeight-int(it->y1)-1,
			int(b->x2),
			int(b->y2),//m_iHeight-int(it->y2)-1,			
			groupNum,
			b->wordNum
			); 

		//without the ending NULL
		fwrite(line,sizeof(char),strlen(line),f);
		//.x1 += 10.f;
		//m_pBoxes[i].x2 += 10.f;
	}
	

	fclose(f);
}

void Boxes::SaveBoxes(const char* extension,bool withGroupNums)
{
	SaveBoxesInternal(extension,m_pBoxes, m_uiBoxesNum, withGroupNums);	
}
