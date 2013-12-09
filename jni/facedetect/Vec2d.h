/*******************************************************************************
Vec2d

Description
-----------  
A 2D vector or point

********************************************************************************/

#ifndef _VEC_2D_H
#define _VEC_2D_H

#include <math.h>

//************************ DEFINES/CONSTANTS/MACROS *****************************

//************************* CLASS/TYPE DEFINITIONS ******************************
class CVec2d
{
public:
	double x;
	double y;

public:
	CVec2d();

	void operator*=(double dScalar);
	void operator/=(double dDivisor);
	CVec2d operator*(double dScalar);
	CVec2d operator/(double dDivisor);

	void operator+=(CVec2d &otherVector);
	void operator-=(CVec2d &otherVector);
	CVec2d operator+(CVec2d &otherVector);
	CVec2d operator-(CVec2d &otherVector);

	double length();
	void normalize();

	static double dot(CVec2d &vecA, CVec2d &vecB)
	{
		return(vecA.x*vecB.x + vecA.y*vecB.y);
	}
	
};

#endif
