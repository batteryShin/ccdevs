/*******************************************************************************
Homography

Description
-----------
A homography matrix defining a 2D projective warp

********************************************************************************/

#ifndef _HOMOGRAPHY_H
#define _HOMOGRAPHY_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "Vec2d.h"

//************************ DEFINES/CONSTANTS/MACROS *****************************


//************************* CLASS/TYPE DEFINITIONS ******************************
class CHomography
{
public:
	double m_dData[3][3];

	double m_matTransformation[4][4], m_matProjection[4][4];

public:
	CHomography();

	void zero();
	void identity();

	void compute(CVec2d *arrSrcPoints, CVec2d *arrDstPoints, int nPoints);

	void transform(CVec2d *pSrcPoint, CVec2d *pDstPoint);

	void rescale(double dScale);
	double computeDeterminant();
	void computePose(bool bFixedFocalLength, double dFocalU, double dFocalV);

	static void computeInverse(double matOriginal[3][3], double matInverse[3][3]);
	void computeInverse(double** matOriginal, double** matInverse);

	void luinvert(double **a, double **y, int n);

	static double *vector(int nMinCol, int nMaxCol);
	static void free_vector(double *pVector, int nMinCol, int nMaxCol);
	static double **matrix(int nMinRow, int nMaxRow, int nMinCol, int nMaxCol);
	static void free_matrix(double **pMatrix, int nMinRow, int nMaxRow, 
		int nMinCol, int nMaxCol);

private:
	int jacobi(double **a, int n, double d[], double **v, int *nrot);
	void eigsrt(double d[], double **v, int n);

	void normalizeData(CVec2d *arrSrcPoints, CVec2d *arrDstPoints, int nPoints, 
		double T1[3][3], double T2[3][3]);
	void undoNormalizationWarp(double fMat[3][3], double T1[3][3], double T2[3][3]);

	static void ludcmp(double **a, int n, int *indx, double *d);
	static void lubksb(double **a, int n, int *indx, double b[]);
};

#endif
