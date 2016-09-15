#ifndef YOEL_BOX_H
#define YOEL_BOX_H

#include <string>

class Box
{
public:
	Box();
	Box(float _x1, float _y1, float _x2, float _y2, std::string _character,int groupNum, int _wordNum);
	~Box();

	//void CopyFrom(Box &other);

	void Print();
	void MergeWith(Box &other);
	bool DoesIntersectWith(Box &other);  
	void StretchMyHeightTo(Box &stretch_to);


	float x1,y1,x2,y2;
	int groupNum;
	int wordNum;
	std::string character;
	bool assigned;
	int average_group_height;
	bool ignore;
	bool selected;
};



#endif
