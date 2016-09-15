#include "Box.h"
#include "Defines.h"
#include <stdio.h>

#ifndef MY_MIN
#define MY_MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MY_MAX
#define MY_MAX(X,Y) ((X) >(Y) ? (X) : (Y))
#endif

Box::Box() : x1(0.f), y1(0.f), x2(0.f), y2(0.f),character("^"), groupNum(0), assigned(false), wordNum(-1), ignore(false), selected(false)
{
	

}


Box::Box(float _x1, float _y1, float _x2, float _y2, std::string _character, int _groupNum, int _wordNum) :
	x1(_x1),
	y1(_y1),
	x2(_x2),
	y2(_y2),
	groupNum(_groupNum),
	character(_character) , 
	assigned(false), 
	wordNum(_wordNum),
	ignore(false), 
	selected(false)
{

}

/*void Box::CopyFrom(Box &other)
{
	x1 = other.x1;
	y1 = other.y1;
	x2 = other.x2;
	y2 = other.y2;
	groupNum = other.groupNum;
	character = other.character;
	wordNum = other.wordNum;
	average_group_height = other.average_group_height;
	assigned = other.assigned;
}*/

void Box::Print()
{
	printf("Box: (%d,%d),(%d,%d)\n",
		int(x1),
		int(y1),
		int(x2),
		int(y2));
}

void Box::MergeWith(Box &other)
{
    x1 = MY_MIN(x1,other.x1);
    y1 = MY_MIN(y1,other.y1);
    x2 = MY_MAX(x2,other.x2);
    y2 = MY_MAX(y2,other.y2) ; 
}

bool Box::DoesIntersectWith(Box &other)
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

void Box::StretchMyHeightTo(Box &stretch_to)
{
	//TODO: add check intersection

	//#if out top is below it - stretch up to it
/*    if (y1 > stretch_to.y1)
	{
        y1 = stretch_to.y1
	}
        
    //#if out bottom is above it - stretch down to it
    if (y2 < stretch_to.y2)
	{
        y2 = stretch_to.y2
	}*/
    
    y1 = stretch_to.y1;
    y2 = stretch_to.y2;
}

Box::~Box()
{

}

