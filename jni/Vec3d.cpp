/*******************************************************************************
Vec3d

Description
-----------
A 3D vector or point

********************************************************************************/

#include "Vec3d.h"

//****************************** CVec2d methods ********************************
CVec3d::CVec3d()
{
	x = 0;
	y = 0;
	z = 0;
}

void CVec3d::operator*=(double dScalar)
{
	x = x * dScalar;
	y = y * dScalar;
	z = z * dScalar;
}

void CVec3d::operator/=(double dDivisor)
{
	x = x / dDivisor;
	y = y / dDivisor;
	z = z / dDivisor;
}

CVec3d CVec3d::operator*(double dScalar)
{
	CVec3d result;

	result.x = x * dScalar;
	result.y = y * dScalar;
	result.z = z * dScalar;

	return result;
}

CVec3d CVec3d::operator/(double dDivisor)
{
	CVec3d result;

	result.x = x / dDivisor;
	result.y = y / dDivisor;
	result.z = z / dDivisor;

	return result;
}

void CVec3d::operator+=(CVec3d &otherVector)
{
	x = x + otherVector.x;
	y = y + otherVector.y;
	z = z + otherVector.z;
}

void CVec3d::operator-=(CVec3d &otherVector)
{
	x = x - otherVector.x;
	y = y - otherVector.y;
	z = z - otherVector.z;
}

CVec3d CVec3d::operator+(CVec3d &otherVector)
{
	CVec3d result;

	result.x = x + otherVector.x;
	result.y = y + otherVector.y;
	result.z = z + otherVector.z;

	return result;
}

CVec3d CVec3d::operator-(CVec3d &otherVector)
{
	CVec3d result;

	result.x = x - otherVector.x;
	result.y = y - otherVector.y;
	result.z = z - otherVector.z;

	return result;
}

double CVec3d::length()
{
	return(sqrt(x*x + y*y + z*z));
}

void CVec3d::normalize()
{
	double dLength = length();

	x /= dLength;
	y /= dLength;
	z /= dLength;
}

void CVec3d::coordinate_normalize()
{
	x /= z;
	y /= z;
	z /= z;
}

