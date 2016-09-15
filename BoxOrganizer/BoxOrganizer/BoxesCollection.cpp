#include "BoxesCollection.h"
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <map>
#include <cstring> 

//#define SHOW_DEBUG_INFO

#ifdef SHOW_DEBUG_INFO
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif
//

//using namespace utf8util;
using namespace std;

//right to left
bool box_compare_func_Average_Height_RTL(const Box& a, const Box&b)
{
	if (a.groupNum == b.groupNum)
	{
		return a.x1 < b.x1;			
	}

	return a.average_group_height < b.average_group_height;		
}

//right to left
bool box_compare_func_Average_Height_LTR(const Box& a,const  Box&b)
{
	if (a.groupNum == b.groupNum)
	{
		return a.x1 > b.x1;			
	}

	return a.average_group_height < b.average_group_height;		
}

bool box_compare_func_X2(const Box& a,const  Box&b)
{
	//return a.x2 > b.x2;
	return a.x2 < b.x2;
}

BoxesCollection::BoxesCollection(const char* file_name,bool right_to_left) : Boxes(file_name)
{
	m_pTree = NULL;
	m_iCurrentGroup = 0;	
}

BoxesCollection::~BoxesCollection()
{
	delete [] m_pBoxes;
	m_pBoxes = NULL;

}

void BoxesCollection::SortBoxes()
{
	/*for (unsigned int i=0;i<m_uiBoxesNum;i++)
	{
		m_pBoxes[i].x1 += 10.f;
		m_pBoxes[i].x2 += 10.f;
	}*/

	m_pTree = new BoxTree(m_iWidth, m_iHeight);

	m_pTree->Load(m_pBoxes,m_uiBoxesNum);


	int count_line = 0;

	//while (FindNextBestLine())
	while (FindNextBestLine_Optimized())
	{
#ifdef SHOW_DEBUG_INFO
		printf("Line %d:\n",count_line);
#endif
		count_line++;
	}		

	//m_iCurrentGroup contains all of the groups that were created so far.
	// building an array that contains all of the average y height of the various groups

	int* sum = new int[m_iCurrentGroup]; //sums of box height centers
	int* num = new int[m_iCurrentGroup]; //number of boxes that were in this group

	for (int g=0;g<m_iCurrentGroup;g++)
	{
		sum[g] = 0;
		num[g] = 0;
	}

	vector<int> average;
	average.resize(m_iCurrentGroup);
	// combing both we can find the average height
	int group = -1;
	int sorted_num = m_SortedBoxes.size();
	for (int b=0;b<sorted_num/*m_uiBoxesNum*/;b++)
	{
		/*if (b >= m_SortedBoxes.size())
		{
			MessageBoxA(0,"if (b >= m_SortedBoxes.size())",0,0);
		}*/
		//assert(m_SortedBoxes[b].assigned);
		group = m_SortedBoxes[b].groupNum;

		if (group == 3)
		{
			int dbg=0;
		}

		assert(group < m_iCurrentGroup);

		/*if (group >= m_iCurrentGroup)
		{
			MessageBoxA(0,"if (group >= m_iCurrentGroup)",0,0);
		}

		if (b >= m_SortedBoxes.size())
		{
			MessageBoxA(0,"if (b >= m_SortedBoxes.size())",0,0);
		}*/

		//printf("group=%d, b=%d\n",group,b);

		sum[group] += m_SortedBoxes[b].y1 + ((m_SortedBoxes[b].y2 - m_SortedBoxes[b].y1) / 2);
		num[group]++;
	}
	
	//calc group averages
	for (int g=0;g<m_iCurrentGroup;g++)
	{
		average[g] = int(float(sum[g]) / float(num[g]));
	}

	//std::sort(average.begin(),average.end(),height_avg_group_num_pari_compare_func);

	//assert(m_uiBoxesNum == m_SortedBoxes.size());

	for (int b=0;b<sorted_num;b++)
	{
		m_SortedBoxes[b].average_group_height =  average[m_SortedBoxes[b].groupNum];		
	}


	if (m_bRightToleft)
	{
		std::sort(m_SortedBoxes.begin(),m_SortedBoxes.end(),box_compare_func_Average_Height_RTL);
	} else
	{
		std::sort(m_SortedBoxes.begin(),m_SortedBoxes.end(),box_compare_func_Average_Height_LTR);
	}

	if (m_uiBoxesNum!=m_SortedBoxes.size())
	{

		char msg[1024];
		sprintf(msg,"Error! sorted items are at different size from unsorted items! Sorted=%d and unsorted=%d\n", m_uiBoxesNum, m_SortedBoxes.size());
		printf(msg);
		printf("This might happen due to merges.\n");
		
		/*//std::vector<Box> m_SortedBoxes;
		for (std::vector<Box>::iterator it=m_SortedBoxes.begin(); it!=m_SortedBoxes.end();++it)
		{
			if (!it->assigned)
			{
				it->Print();
			}

		}
		
		MessageBoxA(0,msg,0,0);*/
	}
}

void BoxesCollection::SaveBoxes(const char* extension,bool withGroupNums)
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
	for (std::vector<Box>::iterator it = m_SortedBoxes.begin(); it!= m_SortedBoxes.end(); ++it)
	{		
		int groupNum = 0;
		int wordNum = 0;
		if (withGroupNums)
		{
			groupNum = it->groupNum;
			wordNum = it->wordNum;
		}


		sprintf(line,"%s %d %d %d %d %d %d\r\n",
			//m_pBoxes[i].character.c_str(),
			it->character.c_str(),
			int(it->x1),						
			int(it->y1),//m_iHeight-int(it->y1)-1,
			int(it->x2),
			int(it->y2),//m_iHeight-int(it->y2)-1,			
			groupNum,
			wordNum
			); 

		//without the ending NULL
		fwrite(line,sizeof(char),strlen(line),f);
		//.x1 += 10.f;
		//m_pBoxes[i].x2 += 10.f;
	}
	

	fclose(f);
}



//rotates a point around (0,0)
void BoxesCollection::RotatePoint(float x, float y, float rad,float &x_out, float &y_out)
{
	 x_out = (x*cos(rad)) - (y*sin(rad));
     y_out = (x*sin(rad)) + (y*cos(rad));        
}

void BoxesCollection::RayPoint(float s_x, float s_y, float d_x, float d_y, float t, float &res_x, float &res_y)
{
	res_x = s_x + (t*d_x);
    res_y = s_y + (t*d_y);
}

bool BoxesCollection::RayAabbIntersection(float s_x, float s_y, float d_x, float d_y, Box& b)
{
	float t = 0;
	float col_x, col_y; // collision point
	if (d_y != 0.0)
	{
		t = (b.y1 - s_y) / d_y; //top    
		if (t > 0)
		{
			RayPoint(s_x,s_y,d_x,d_y, t, col_x, col_y);
			if (col_x >= b.x1 && col_x <= b.x2)
			{
				return true;
			}
		}
	}
	if (d_y != 0.0)
	{
		t = (b.y2 - s_y) / d_y; //bottom
		if (t > 0)
		{
			RayPoint(s_x,s_y,d_x,d_y, t, col_x, col_y);
			if (col_x >= b.x1 && col_x <= b.x2)
			{
				return true;
			}
		}
	}
	if (d_x != 0.0)
	{
		t = (b.x1 - s_x) / d_x; //left
		if (t > 0)
		{
			RayPoint(s_x,s_y,d_x,d_y, t, col_x, col_y);
			if (col_y >= b.y1 && col_y <= b.y2)
			{
				return true;
			}
		}
	}
	if (d_x != 0.0)
	{
		t = (b.x2 - s_x) / d_x; //right
		if (t > 0)
		{
			RayPoint(s_x,s_y,d_x,d_y, t, col_x, col_y);
			if (col_y >= b.y1 && col_y <= b.y2)
			{
				return true;         
			}
		}
	}
	return false;
}

bool BoxesCollection::FindNextBestLine_Optimized()
{
	//int width = 333;
	//int height = 890;

	int best_line_boxes_num = 0;
    int best_line = 0;
    float best_line_rad = 0.f;
    int count_line_boxes = 0;
	int range = 1; //how many angle "ticks" per side
	float tick_size = 0.5f;
	int intersecting_boxes_num = 0;
	float curr_ang = 0.f;
	float curr_rad = 0.f;
	int best_line_max_y = -1;
	int best_line_min_y = 0xFFFF;		
	float ray_dir_x,ray_dir_y;
	//for j in xrange(current_resolution[1])
	for (int j=0;j<m_iHeight;j++)
	{
        //#status = int ((100.0 * j) / float(current_resolution[1]))            
        //#if j%50 == 0:
        //#    print "%d %%" % status
        //#for i in reversed(xrange(current_resolution[0])):
        
        if (j%100 == 0)
		{
            DEBUG_PRINT("Line %d\n",j);
		}
		        
		if (j==16)
		{
			int dbg = 0;
		}

		for (int a=0;a<range*2;a++)
		//int a = range;
		{
            intersecting_boxes_num = 0;
            
			curr_ang = float(a - range);
			curr_ang *= tick_size;

            curr_rad = curr_ang * 0.0174532925f;       
			RotatePoint(-1.f,0.f,curr_rad,ray_dir_x,ray_dir_y);

			int max_y = -1;
			int min_y = 0xFFFF;


			//RESTORE - for tree optimization
			intersecting_boxes_num = m_pTree->CountRayIntersects(float(m_iWidth-1),float(j) , ray_dir_x, ray_dir_y, min_y, max_y, false);

			//RESTORE - for naive implementation
			/*intersecting_boxes_num = 0;
			for (int b=0;b<m_uiBoxesNum;b++)
			{
				Box* box = &m_pBoxes[b];
				assert(box);


				if (box->x1 == 212 &&
						box->y1 == 6 &&
						box->x2 == 217 &&
						box->y2 == 21)
				{
					if (box->assigned)
					{
						int k =0;
					}
					int dbg=0;
				}

                if (true == box->assigned)
				{
                    continue;
				}
				
                //create a ray, shooting from the right edge of the screen
                //sweep a ray like a flash light, searching for the best angled ray                

				//(68,892),(101,904)
				if (box->x1 == 212 &&
						box->y1 == 6 &&
						box->x2 == 217 &&
						box->y2 == 21)
				{
					if (box->assigned)
					{
						int k =0;
					}
					int dbg=0;
				}



                //ray_dir = rotate_point((-1.0,0.0), curr_rad)				
                if (RayAabbIntersection( float(m_iWidth-1),float(j) , ray_dir_x, ray_dir_y,*box))
				{
					if (box->x1 == 106 &&
						box->y1 == 485 &&
						box->x2 == 159 &&
						box->y2 == 496)
					{
						int dbg=0;
					}
                    intersecting_boxes_num += 1;
					if (box->y1 < min_y)
					{
						min_y = box->y1;
					}

					if (box->y2 > max_y)
					{
						max_y = box->y2;
					}
				}				
			}
			*/
			

            if (intersecting_boxes_num > best_line_boxes_num || (intersecting_boxes_num == best_line_boxes_num && curr_rad < best_line_rad))
			{
                best_line_boxes_num = intersecting_boxes_num;
                best_line = j;
                best_line_rad = curr_rad;
				
				best_line_max_y = max_y;
				best_line_min_y = min_y;
			}
		}
	}

	if (0 == best_line_boxes_num)
	{
		printf("Didn't find any new line. Sort ended.\n");

		for (int b=0;b<m_uiBoxesNum;b++)
		{
			if (!m_pBoxes[b].assigned)
			{
				printf("Didn't assign: box num %d ",b);
				m_pBoxes[b].Print();
			}
		}

		return false;
	}
	
	DEBUG_PRINT("Found %d boxes for group %d\n",best_line_boxes_num, m_iCurrentGroup);

	float ray_start_x = float(m_iWidth-1);
	float ray_start_y = float(best_line);	

	DEBUG_PRINT("Used ray - Start (%.3f,%.3f) Direction (%.3f,%.3f)\n",
		ray_start_x,ray_start_y,
		ray_dir_x,ray_dir_y);
	
	std::vector<Box> line_boxes;

	RotatePoint(-1.f,0.f,best_line_rad,ray_dir_x,ray_dir_y);

	//mark as assigned on the treeview
	intersecting_boxes_num = m_pTree->CountRayIntersects(ray_start_x, ray_start_y , ray_dir_x, ray_dir_y, best_line_min_y, best_line_max_y, true);

	//now, after we have chosen the right line, collect the boxes
	//potentially some double-work here.

	//we also add bboxes that are completely contained in the entire line bbox y range.

	map<int,int> fully_contained_boxes;

	for (int b=0;b<m_uiBoxesNum;b++)
	{
		Box* box = &m_pBoxes[b];		
		assert(box);


		/*
		//REMOVED the swallowing

		//If it's fully contained inside the line AABB, add it as well
		//this is relevant for sentences like "Hi! this, is ' ' a test , . "
		//in such sentence we might only get the top ' " or instead just the bottom , . 
		//but we want both.
		if (box->y1 >= best_line_min_y &&
			box->y2 <= best_line_max_y)
		{
			box->Print();
			line_boxes.push_back(*box);
            box->assigned = true;	
			fully_contained_boxes[b]=b;
			continue;
		}
		*/


        if (true == box->assigned)
		{
            continue;
		}
		
		bool add_box = false;

		if (RayAabbIntersection( float(m_iWidth-1),float(best_line) , ray_dir_x, ray_dir_y,*box))
		{
			add_box = true;
		}

		if (box->y1 >= best_line_min_y &&
			box->y2 <= best_line_max_y)
		{
			add_box = true;
		}


		// if the bbox intersect with the chosen line, add it
		if (add_box)
		{
			if (box->x1 == 416 &&
				box->y1 == 0 &&
				box->x2 == 425 &&
				box->y2 == 15-1)
				{
					printf("Debug box - ");
					box->Print();
					int dbg=0;
				}



			//box->Print();

#ifdef SHOW_DEBUG_INFO
			printf("Adding box \n");
			box->Print();
#endif

			line_boxes.push_back(*box);
            box->assigned = true;			
            //b.groupColor = groupColor
            //box->groupNum = m_iCurrentGroup;
			continue;
		}
	}
	
	//REMOVED the swallowing
	/*//if a line "swallowed" a bbox, we need to remove it from the previous assignment (if it existed)
	for (std::vector<Box>::iterator it = m_SortedBoxes.begin(); it!=m_SortedBoxes.end();++it)
	{
		if (it->y1 >= best_line_min_y &&
			it->y2 <= best_line_max_y)
		{
			it = m_SortedBoxes.erase(it);
		}		
	}*/

	// sort

	// using function as comp
	std::sort (line_boxes.begin(), line_boxes.end(), box_compare_func_X2);
	
	for (std::vector<Box>::iterator it = line_boxes.begin(); it!=line_boxes.end();++it)
	{		
		it->groupNum = m_iCurrentGroup;
		m_SortedBoxes.push_back(*it);		
	}
		
	//vector<int> line_min_y;
	//vector<int> line_max_y;

	//m_SortedBoxes.insert(m_SortedBoxes.end(), line_boxes.begin(), line_boxes.end());

	m_iCurrentGroup++;
	
	return true;
}



bool BoxesCollection::FindNextBestLine()
{
	return true;
	/*
	//int width = 333;
	//int height = 890;

	int best_line_boxes_num = 0;
    int best_line = 0;
    float best_line_rad = 0.f;
    int count_line_boxes = 0;
	int range = 1; //how many angle "ticks" per side
	float tick_size = 0.5f;
	int intersecting_boxes_num = 0;
	float curr_ang = 0.f;
	float curr_rad = 0.f;
	int best_line_max_y = -1;
	int best_line_min_y = 0xFFFF;		
	float ray_dir_x,ray_dir_y;
	//for j in xrange(current_resolution[1])
	for (int j=0;j<m_iHeight;j++)
	{
        //#status = int ((100.0 * j) / float(current_resolution[1]))            
        //#if j%50 == 0:
        //#    print "%d %%" % status
        //#for i in reversed(xrange(current_resolution[0])):
        
        if (j%100 == 0)
		{
            DEBUG_PRINT("Line %d\n",j);
		}
        
		if (j==900)
		{
			int dbg = 0;
		}

		for (int a=0;a<range*2;a++)
		//int a = range;
		{
            intersecting_boxes_num = 0;
            
			curr_ang = float(a - range);
			curr_ang *= tick_size;

            curr_rad = curr_ang * 0.0174532925f;       
			RotatePoint(-1.f,0.f,curr_rad,ray_dir_x,ray_dir_y);

			int max_y = -1;
			int min_y = 0xFFFF;

			for (int b=0;b<m_uiBoxesNum;b++)
			{
				Box* box = &m_pBoxes[b];
				assert(box);
                if (true == box->assigned)
				{
                    continue;
				}
				
                //create a ray, shooting from the right edge of the screen
                //sweep a ray like a flash light, searching for the best angled ray                

				//(68,892),(101,904)
				if (box->x1 == 106 &&
						box->y1 == 485 &&
						box->x2 == 159 &&
						box->y2 == 496)
				{
					if (box->assigned)
					{
						int k =0;
					}
					int dbg=0;
				}



                //ray_dir = rotate_point((-1.0,0.0), curr_rad)				
                if (RayAabbIntersection( float(m_iWidth-1),float(j) , ray_dir_x, ray_dir_y,*box))
				{
					if (box->x1 == 106 &&
						box->y1 == 485 &&
						box->x2 == 159 &&
						box->y2 == 496)
					{
						int dbg=0;
					}
                    intersecting_boxes_num += 1;
					if (box->y1 < min_y)
					{
						min_y = box->y1;
					}

					if (box->y2 > max_y)
					{
						max_y = box->y2;
					}
				}
			}
            if (intersecting_boxes_num > best_line_boxes_num || (intersecting_boxes_num == best_line_boxes_num && curr_rad < best_line_rad))
			{
                best_line_boxes_num = intersecting_boxes_num;
                best_line = j;
                best_line_rad = curr_rad;
				
				best_line_max_y = max_y;
				best_line_min_y = min_y;
			}
		}
	}

	if (0 == best_line_boxes_num)
	{
		printf("Didn't find any new line. Sort ended.\n");

		for (int b=0;b<m_uiBoxesNum;b++)
		{
			if (!m_pBoxes[b].assigned)
			{
				printf("Didn't assign: box num %d ",b);
				m_pBoxes[b].Print();
			}
		}

		return false;
	}

	DEBUG_PRINT("Found %d boxes for group %d\n",best_line_boxes_num, m_iCurrentGroup);
	
	std::vector<Box> line_boxes;

	RotatePoint(-1.f,0.f,best_line_rad,ray_dir_x,ray_dir_y);

	//now, after we have chosen the right line, collect the boxes
	//potentially some double-work here.

	//we also add bboxes that are completely contained in the entire line bbox y range.

	map<int,int> fully_contained_boxes;

	for (int b=0;b<m_uiBoxesNum;b++)
	{
		Box* box = &m_pBoxes[b];		
		assert(box);


        if (true == box->assigned)
		{
            continue;
		}
		
		bool add_box = false;

		if (RayAabbIntersection( float(m_iWidth-1),float(best_line) , ray_dir_x, ray_dir_y,*box))
		{
			add_box = true;
		}

		if (box->y1 >= best_line_min_y &&
			box->y2 <= best_line_max_y)
		{
			add_box = true;
		}


		// if the bbox intersect with the chosen line, add it
		if (add_box)
		{
			if (box->x1 == 106 &&
				box->y1 == 485 &&
				box->x2 == 159 &&
				box->y2 == 496)
				{
					int dbg=0;
				}

			//box->Print();

			line_boxes.push_back(*box);
            box->assigned = true;			
            //b.groupColor = groupColor
            //box->groupNum = m_iCurrentGroup;
			continue;
		}
	}
	// sort

	// using function as comp
	std::sort (line_boxes.begin(), line_boxes.end(), box_compare_func_X2);
	
	for (std::vector<Box>::iterator it = line_boxes.begin(); it!=line_boxes.end();++it)
	{		
		it->groupNum = m_iCurrentGroup;
		m_SortedBoxes.push_back(*it);		
	}
		
	//vector<int> line_min_y;
	//vector<int> line_max_y;

	//m_SortedBoxes.insert(m_SortedBoxes.end(), line_boxes.begin(), line_boxes.end());

	m_iCurrentGroup++;
	
	return true;
	*/
}
