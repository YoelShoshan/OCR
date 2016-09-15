#include "BoxTree.h"
#include <assert.h>

BoxTree::BoxTree(int width, int height)
{
	m_pRoot = NULL;
	m_iBoxesNum = 0;
	m_pResultBoxes = NULL;

	m_iWidth = width;
	m_iHeight = height;
}

BoxTree::~BoxTree()
{	


}

void BoxTree::Load(Box* boxes, int boxes_num)
{
	//build tree

	//first - create a simple quad split
	//Note: I'm not aabb aware, i split regardless of the data

	int max_x = -1;
	int max_y = -1;

	for (int i=0;i<boxes_num;i++)
	{
		if (boxes[i].x2 > max_x)
			max_x = boxes[i].x2;

		if (boxes[i].y2 > max_y)
			max_y = boxes[i].y2;
	}

	m_pRoot = new Node(0,0,max_x+1,max_y+1, NULL);

	RecurseSplit(m_pRoot,0);

	for (int i=0;i<boxes_num;i++)
	{
		SimpleBox b(boxes[i].x1,boxes[i].y1,boxes[i].x2,boxes[i].y2);

		bool bFirst = true;
		do
		{
			/*if (bFirst)
			{
				printf("Box: ");				
			} else
			{
				printf("-> ");				
			}

			b.Print();*/

			b = AddBox(m_pRoot,b);
			bFirst=false;
		} while (b.x1 != -1);
		//printf("\n");
		
	}	
}

Box* BoxTree::GetBoxes(int& merged_boxes_num)
{
	merged_boxes_num = m_iBoxesNum;

	// now generate a flat Box list	
	m_iCurrentBoxNum = 0;
	m_pResultBoxes = new Box[m_iBoxesNum];
	GenerateFlatBoxList(m_pRoot);

	return m_pResultBoxes;
}

void BoxTree::GenerateFlatBoxList(Node* node)
{
	if (!node)
		return;

	for (vector<SimpleBox>::iterator it=node->boxes.begin();it!=node->boxes.end();++it)
	{
		m_pResultBoxes[m_iCurrentBoxNum].x1 = it->x1;
		m_pResultBoxes[m_iCurrentBoxNum].y1 = it->y1;
		m_pResultBoxes[m_iCurrentBoxNum].x2 = it->x2;
		m_pResultBoxes[m_iCurrentBoxNum].y2 = it->y2;
		m_iCurrentBoxNum++;
	}
	
	for (int i=0;i<4;i++)
	{
		GenerateFlatBoxList(node->children[i]);
	}
}

#define HALF_X node->area.x1 + ( (node->area.x2-node->area.x1) / 2)
#define HALF_Y node->area.y1 + ( (node->area.y2-node->area.y1) / 2)

void BoxTree::RecurseSplit(Node* node, int depth)
{
	assert(node);
	if (depth == 4)
	{
		return;
	}

	//top left
	node->children[0] = new Node( 
		node->area.x1, 
		node->area.y1, 
		HALF_X, 
		HALF_Y,
		node);

	//top right
	node->children[1] = new Node( 
		HALF_X, 
		node->area.y1, 
		node->area.x2, 
		HALF_Y,
		node);

	//bottom left
	node->children[2] = new Node( 
		node->area.x1, 
		HALF_Y, 
		HALF_X, 
		node->area.y2,
		node);

	//bottom right
	node->children[3] = new Node( 
		HALF_X, 
		HALF_Y, 
		node->area.x2, 
		node->area.y2,
		node);

	for (int i=0;i<4;i++)
	{
		RecurseSplit(node->children[i],depth+1);
	}
}

SimpleBox BoxTree::AddToBoxes(vector<SimpleBox>&boxes, SimpleBox b)
{
	for (vector<SimpleBox>::iterator it=boxes.begin();it!=boxes.end();++it)
	{
		if (b.doesIntersect(*it))
		{
			SimpleBox merged_box(b);
			merged_box.mergeWith(*it);
			boxes.erase(it);
			m_iBoxesNum--;
			return merged_box;
		}
	}

	boxes.push_back(b);
	m_iBoxesNum++;
	return SimpleBox(-1,-1,-1,-1);
}


SimpleBox BoxTree::AddBox(Node*node,const SimpleBox b)
{
	if (!node->children[0])
	{
		return AddToBoxes(node->boxes,b);
	}
	//General concept:
	//Add a box into our tree structure.
	
	//1. Check the childern first, if fully inside one child, recurse into it
	//2. If touching one of the childs edges, simple compare against the list of the child area that touches it
	//3. If no children, add to current.

	// If fully inside - recuse into it
	for (int i=0;i<4;i++)
	{
		if (b.fullyInside(node->children[i]->area))
		{			
			return AddBox(node->children[i],b);
		}
	}

	// If we are here, then we are not fully inside any of the children.
	// check with which node child we intersect, and compare against it.

	for (int i=0;i<4;i++)
	{
		if (b.doesIntersect(node->children[i]->area))
		{
			return AddToBoxes(node->children[i]->boxes,b);			
		}
	}

	//add to current node - check for potential merges

	return AddToBoxes(node->boxes,b);			
}

int BoxTree::CountRayIntersects(float s_x, float s_y, float d_x, float d_y, int &min_y, int&max_y,bool mark_as_assigned)
{
	m_RayStart_x = s_x;
	m_RayStart_y = s_y;
	m_RayDir_x = d_x;
	m_RayDir_y = d_y;

	m_iRayBoxIntersectionsNum = 0;

	m_min_y_in_current_ray_search = 0xFFFF;
	m_max_y_in_current_ray_search = -0xFFFF;

	RecurseCountRayIntersections(m_pRoot, mark_as_assigned);

	min_y = m_min_y_in_current_ray_search;
	max_y = m_max_y_in_current_ray_search;

	return m_iRayBoxIntersectionsNum;
}

void BoxTree::CountRayBoxesIntersections(vector<SimpleBox>&boxes,bool mark_as_assigned)
{
	for (vector<SimpleBox>::iterator it=boxes.begin();it!=boxes.end();++it)
	{
		if (it->assigned)
		{
			continue;
		}

		if (RaySimpleBoxIntersection(m_RayStart_x,m_RayStart_y,m_RayDir_x,m_RayDir_y,*it))
		{
			m_iRayBoxIntersectionsNum++;			
			
			if (it->y1 < m_min_y_in_current_ray_search)
			{
				m_min_y_in_current_ray_search = it->y1;
			}

			if (it->y2 > m_max_y_in_current_ray_search)
			{
				m_max_y_in_current_ray_search = -0xFFFF;
			}			

			if (mark_as_assigned)
			{
				it->assigned = true;
			}
		}		
	}
}

void  BoxTree::RecurseCountRayIntersections(Node* node,bool mark_as_assigned)
{
	if (!node)
		return;
	if (!node->children[0])
	{
		CountRayBoxesIntersections(node->boxes,mark_as_assigned);
		return;
	}

	for (int i=0;i<4;i++)
	{
		//if (b.doesIntersect(node->children[i]->area))
		if (RaySimpleBoxIntersection(m_RayStart_x,m_RayStart_y,m_RayDir_x,m_RayDir_y,node->children[i]->area))
		{
			//CountRayBoxesIntersectionsCount(node->children[i]->boxes);
			RecurseCountRayIntersections(node->children[i],mark_as_assigned);
			//return;
		}
	}

	//check on current node boxes as well

	CountRayBoxesIntersections(node->boxes,mark_as_assigned);
	return;
}

void BoxTree::RayPoint(float s_x, float s_y, float d_x, float d_y, float t, float &res_x, float &res_y)
{
	res_x = s_x + (t*d_x);
    res_y = s_y + (t*d_y);
}





bool BoxTree::RaySimpleBoxIntersection(float s_x, float s_y, float d_x, float d_y, SimpleBox& b)
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