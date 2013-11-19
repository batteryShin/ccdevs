/*******************************************************************************
Vec3d

Description
-----------  
A 3D vector or point

********************************************************************************/

#ifndef _VEC_3D_H
#define _VEC_3D_H

#include <math.h>

//************************ DEFINES/CONSTANTS/MACROS *****************************

//************************* CLASS/TYPE DEFINITIONS ******************************
class CVec3d
{
public:
	double x;
	double y;
	double z;

public:
	CVec3d();

	void operator*=(double dScalar);
	void operator/=(double dDivisor);
	CVec3d operator*(double dScalar);
	CVec3d operator/(double dDivisor);

	void operator+=(CVec3d &otherVector);
	void operator-=(CVec3d &otherVector);
	CVec3d operator+(CVec3d &otherVector);
	CVec3d operator-(CVec3d &otherVector);

	double length();
	void normalize();
	void coordinate_normalize();

	static double dot(CVec3d &vecA, CVec3d &vecB)
	{
		return(vecA.x*vecB.x + vecA.y*vecB.y + vecA.z*vecB.z);
	}

	static CVec3d cross(CVec3d &vecA, CVec3d &vecB)
	{
		CVec3d res;
		res.x = vecA.y*vecB.z - vecA.z*vecB.y;
		res.y = vecA.z*vecB.x - vecA.x*vecB.z;
		res.z = vecA.x*vecB.y - vecA.y*vecB.x;

		return res;
	}
};

#endif
