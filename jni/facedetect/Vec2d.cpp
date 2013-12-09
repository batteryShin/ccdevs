/*******************************************************************************
Vec2d

Description
-----------
A 2D vector or point

********************************************************************************/

#include "Vec2d.h"

//****************************** CVec2d methods ********************************
CVec2d::CVec2d()
{
	x = 0;
	y = 0;
}

void CVec2d::operator*=(double dScalar)
{
	x = x * dScalar;
	y = y * dScalar;
}

void CVec2d::operator/=(double dDivisor)
{
	x = x / dDivisor;
	y = y / dDivisor;
}

CVec2d CVec2d::operator*(double dScalar)
{
	CVec2d result;

	result.x = x * dScalar;
	result.y = y * dScalar;

	return result;
}

CVec2d CVec2d::operator/(double dDivisor)
{
	CVec2d result;

	result.x = x / dDivisor;
	result.y = y / dDivisor;

	return result;
}

void CVec2d::operator+=(CVec2d &otherVector)
{
	x = x + otherVector.x;
	y = y + otherVector.y;
}

void CVec2d::operator-=(CVec2d &otherVector)
{
	x = x - otherVector.x;
	y = y - otherVector.y;
}

CVec2d CVec2d::operator+(CVec2d &otherVector)
{
	CVec2d result;

	result.x = x + otherVector.x;
	result.y = y + otherVector.y;

	return result;
}

CVec2d CVec2d::operator-(CVec2d &otherVector)
{
	CVec2d result;

	result.x = x - otherVector.x;
	result.y = y - otherVector.y;

	return result;
}

double CVec2d::length()
{
	return(sqrt(x*x + y*y));
}

void CVec2d::normalize()
{
	double dLength = length();

	x /= dLength;
	y /= dLength;
}

