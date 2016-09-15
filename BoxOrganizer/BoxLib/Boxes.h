#ifndef YOEL_BOXES_H
#define YOEL_BOXES_H

#include "Box.h"
#include <vector>
#include <string>
#include "Defines.h"
#include <math.h>

struct Vec2D
{
	Vec2D()
	{
	}
	Vec2D(float _x, float _y) : x(_x), y(_y)
	{

	};

	void Set(float _x, float _y)
	{
		x = _x;
		y = _y;
	}

	void Normalize()
	{
		float vec_len = sqrt(x*x+y*y);
		x = x / vec_len;
		y = y / vec_len;
	}

	float GetAngleWith(const Vec2D& other) const
	{
		float cos_ang = x*other.x + y*other.y;
		float rad = acos(cos_ang);
		//#print "rad: %f" % rad
		return rad * 57.2957795f;
	}

	float x,y;
};

void tokenize_line(std::string& line, std::vector<std::string>& tokens);

class Boxes
{
public:
	Boxes(const char* file_name);
	~Boxes();

	void SetBoxes(Box* boxes, int boxes_num) { delete [] m_pBoxes; m_pBoxes = boxes; m_uiBoxesNum=boxes_num;}
	void SaveBoxes(const char* extension,bool withGroupNums=true);	

	Box* GetBoxes(int &boxes_num) 
	{ 
		boxes_num = m_uiBoxesNum; 
		return m_pBoxes;
	}

	int GetWidth() const { return m_iWidth;};
	int GetHeight() const { return m_iHeight;};
	

	void FindWords(const char* extension);

protected:	

	void SaveBoxesInternal(const char* extension, Box* boxes, int boxes_num, bool withGroupNums=true);

	void FindWord(Box* b);

	int m_iWordsNum;

	//For word finding
	float CalcBoxesDistance(IN Box& b1, IN Box & b2, IN OUT Vec2D &dir);
	bool AreBoxesClose_2(Box &a, Box &b);

	unsigned int m_uiBoxesNum;
	Box* m_pBoxes;

	char* m_pBoxFileName;
	int m_iWidth, m_iHeight;
};



#endif
