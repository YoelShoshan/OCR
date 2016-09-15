#ifndef YOEL_BOXES_COLLECTION_H
#define YOEL_BOXES_COLLECTION_H

#include "../BoxLib/Box.h"
#include "../BoxLib/Boxes.h"
#include <vector>
#include "../BoxLib/BoxTree.h"


//FIXME: Code duplication with the BoxLib "Boxes" class.
class BoxesCollection : public Boxes
{
public:
	BoxesCollection(const char* file_name, bool right_to_left);
	~BoxesCollection();

	void SortBoxes();
	void SaveBoxes(const char* extension,bool withGroupNums=true);

protected:

	std::vector<Box> m_SortedBoxes;
	bool m_bRightToleft;
	int m_iCurrentGroup;

	void RotatePoint(float x, float y, float rad,float &x_out, float &y_out);
	void RayPoint(float s_x, float s_y, float d_x, float d_y, float t, float &res_x, float &res_y);

	bool RayAabbIntersection(float s_x, float s_y, float d_x, float d_y, Box& b);

	bool FindNextBestLine();

	bool FindNextBestLine_Optimized();


	BoxTree* m_pTree;
};



#endif