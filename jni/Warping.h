#pragma once

#include <cv.h>
#include <cxcore.h>

#include "kpmtypes.h"

class CWarping
{
public:
	static void MakeTransformMatrix(double* theta, double* scale, double* tr, CvMat* dstH);

	static void PerspectiveTransform(IplImage* src, IplImage* dst, CvMat* matH);
	static void PerspectiveTransform2(IplImage* src, IplImage* dst, CvMat* matH);

	// overload for double array..
	static void PerspectiveTransform(double** src, double** dst, CvSize elem_size, CvMat* matH);

	static int FindArea(IplImage* img);
};
