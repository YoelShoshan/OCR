#ifndef YOEL_BOX_TREE_H
#define YOEL_BOX_TREE_H
#include "../BoxLib/Box.h"
#include <vector>
#include <stdio.h>
using namespace std;

struct SimpleBox
{
	SimpleBox(): assigned(false) {}
	SimpleBox(int _x1, int _y1, int _x2, int _y2) : x1(_x1), y1(_y1), x2(_x2), y2(_y2), assigned(false) {}
	void Set(int _x1, int _y1, int _x2, int _y2) 
	{
		x1=_x1;
		y1=_y1;
		x2=_x2;
		y2=_y2;
	}

	void Print() const
	{
		printf("(%d,%d,%d,%d)\n",x1,y1,x2,y2);
	};

	bool doesIntersect(const SimpleBox &other) const
	{
		if (x1 > other.x2)
			return false;

		if (x2 < other.x1)
			return false;

		if (y1 > other.y2)
			return false;

		if (y2 < other.y1)
			return false;

		return true;	    
	}

	void mergeWith(const SimpleBox &other) 
	{
		x1 = min(x1,other.x1);
		x2 = max(x2,other.x2);
		y1 = min(y1,other.y1);
		y2 = max(y2,other.y2);
	}

	bool fullyInside(const SimpleBox &other) const
	{
		if ( (x2 >= other.x1) &&
			(x1 <= other.x2) &&
			(y2 >= other.y1) &&
			(y1 <= other.y2))
		{
			return true;
		}

		return false;
	}

	int x1,y1,x2,y2;
	bool assigned;
};

struct Node
{
	Node(int x1, int y1, int x2, int y2, Node* _parent): parent(_parent)
	{
		children[0] = children[1] = children[2] = children[3] = NULL;
		area.Set(x1,y1,x2,y2);
	}

	SimpleBox area;
	
	Node* parent;
	Node* children[4];

	//TODO: Change into BOX pointers
	//TODO: make a pool of BOX pointers
	vector<SimpleBox> boxes;
};


class BoxTree
{
public:
	BoxTree(int width, int height);
	~BoxTree();

	void Load(Box* boxes, int boxes_num);
	Box* GetBoxes(int& merged_boxes_num);
	int CountRayIntersects(float s_x, float s_y, float d_x, float d_y, int &min_y, int&max_y,bool mark_as_assigned);


private:

	void RecurseCountRayIntersections(Node* node,bool mark_as_assigned);

	void RecurseSplit(Node* node, int depth);
	SimpleBox AddBox(Node*node,SimpleBox b);

	SimpleBox AddToBoxes(vector<SimpleBox>&boxes, SimpleBox b);
	void CountRayBoxesIntersections(vector<SimpleBox>&boxes,bool mark_as_assigned);

	void GenerateFlatBoxList(Node* node);

	bool RaySimpleBoxIntersection(float s_x, float s_y, float d_x, float d_y, SimpleBox& b);
	void RayPoint(float s_x, float s_y, float d_x, float d_y, float t, float &res_x, float &res_y);	

	Node* m_pRoot;
	int m_iBoxesNum;
	int m_iCurrentBoxNum; // helper for the flatten step

	Box* m_pResultBoxes;

	int m_iWidth;
	int m_iHeight;

	float m_RayStart_x;
	float m_RayStart_y;
	float m_RayDir_x;
	float m_RayDir_y;

	int m_min_y_in_current_ray_search;
	int m_max_y_in_current_ray_search;

	int m_iRayBoxIntersectionsNum;
};







#endif
