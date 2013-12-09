/*******************************************************************************
Homography

Description
-----------
Defines a 2D projective warp

Revision History
----------------
December 1, 2001 (Shahzad Malik)
- Created

********************************************************************************/

#include "Homography.h"

#define NORMALIZEWARP

//***************************** DEFINES/CONSTANTS ******************************
#define JACOBI_ROTATE(a, i, j, k, l)	{ g = a[i][j]; h = a[k][l]; a[i][j] = g-s*(h+g*tau); a[k][l]=h+s*(g-h*tau); }

double* CHomography::vector(int nMinCol, int nMaxCol) {
	int nCols = nMaxCol + 1;

	double *pVector = new double[nCols];

	return pVector;
}

void CHomography::free_vector(double *pVector, int nMinCol, int nMaxCol)
{
	delete [] pVector;
}

double** CHomography::matrix(int nMinRow, int nMaxRow, int nMinCol, int nMaxCol)
{
	double **pMatrix;

	int nRows = nMaxRow + 1;

	pMatrix = new double*[nRows];
	if(pMatrix)
	{
		for(int ii = 0;ii < nRows;ii++)
		{
			pMatrix[ii] = vector(nMinCol, nMaxCol);
		}
	}

	return pMatrix;
}

void CHomography::free_matrix(double **pMatrix, int nMinRow, int nMaxRow, int nMinCol, int nMaxCol)
{
	int nRows = nMaxRow + 1;

	for(int ii = 0;ii < nRows;ii++)
	{
		free_vector(pMatrix[ii], nMinCol, nMaxCol);
	}

	delete [] pMatrix;
}

//**************************** CHomography methods *****************************
CHomography::CHomography()
{
	identity();
}

void CHomography::zero()
{
	m_dData[0][0] = 0.0;
	m_dData[0][1] = 0.0;
	m_dData[0][2] = 0.0;
	m_dData[1][0] = 0.0;
	m_dData[1][1] = 0.0;
	m_dData[1][2] = 0.0;
	m_dData[2][0] = 0.0;
	m_dData[2][1] = 0.0;
	m_dData[2][2] = 0.0;
}

void CHomography::identity()
{
	m_dData[0][0] = 1.0;
	m_dData[0][1] = 0.0;
	m_dData[0][2] = 0.0;
	m_dData[1][0] = 0.0;
	m_dData[1][1] = 1.0;
	m_dData[1][2] = 0.0;
	m_dData[2][0] = 0.0;
	m_dData[2][1] = 0.0;
	m_dData[2][2] = 1.0;
}

// Computes the homography based on the set of matches
void CHomography::compute(CVec2d *arrSrcPoints, CVec2d *arrDstPoints, int nPoints)
{
	int i, j, ii, jj, numberOfRotations;

#ifdef NORMALIZEWARP
	double T1[3][3];
	double T2[3][3];
#endif

	CVec2d *arrCopySrc, *arrCopyDst;

	double **A = matrix(0, 2*nPoints-1, 0, 8);
	double **ATA = matrix(1, 9, 1, 9);
	double **eigenVectors = matrix(1, 9, 1, 9);
	double *eigenValues = vector(1, 9);

#ifdef NORMALIZEWARP
	// Normalize data
	arrCopySrc = new CVec2d[nPoints];
	arrCopyDst = new CVec2d[nPoints];

	memcpy(arrCopySrc, arrSrcPoints, sizeof(CVec2d) * nPoints);
	memcpy(arrCopyDst, arrDstPoints, sizeof(CVec2d) * nPoints);

	normalizeData(arrCopySrc, arrCopyDst, nPoints, T1, T2);

#else
	arrCopySrc = arrSrcPoints;
	arrCopyDst = arrDstPoints;
#endif

	// Set up matrix A
	for(i = 0;i < nPoints;i++)
	{
		double x1 = arrCopySrc[i].x;
		double y1 = arrCopySrc[i].y;
		double x2 = arrCopyDst[i].x;
		double y2 = arrCopyDst[i].y;

		A[2*i][0] = x1;
		A[2*i][1] = y1;
		A[2*i][2] = 1;
		A[2*i][3] = 0;
		A[2*i][4] = 0;
		A[2*i][5] = 0;
		A[2*i][6] = -x2 * x1;
		A[2*i][7] = -x2 * y1;
		A[2*i][8] = -x2;
		A[2*i+1][0] = 0;
		A[2*i+1][1] = 0;
		A[2*i+1][2] = 0;
		A[2*i+1][3] = x1;
		A[2*i+1][4] = y1;
		A[2*i+1][5] = 1;
		A[2*i+1][6] = -y2 * x1;
		A[2*i+1][7] = -y2 * y1;
		A[2*i+1][8] = -y2;
	}

	// Compute A(transpose) * A
	for(i = 0;i < 9;i++)
		for(j = 0;j < 9;j++)
			ATA[j+1][i+1] = 0;

	for(i = 0;i < 2*nPoints;i++)
		for(ii = 0;ii < 9;ii++)
			for(jj = 0;jj < 9;jj++)
				ATA[ii+1][jj+1] += A[i][ii] * A[i][jj];

	// Compute the eigenvectors and eigenvalues using Jacobi
	jacobi((double **)ATA, 9, eigenValues, (double **)eigenVectors, &numberOfRotations);
	eigsrt(eigenValues, (double **)eigenVectors, 9);

	// Fill in the values for the homography
	m_dData[0][0] = eigenVectors[1][9];
	m_dData[0][1] = eigenVectors[2][9];
	m_dData[0][2] = eigenVectors[3][9];
	m_dData[1][0] = eigenVectors[4][9];
	m_dData[1][1] = eigenVectors[5][9];
	m_dData[1][2] = eigenVectors[6][9];
	m_dData[2][0] = eigenVectors[7][9];
	m_dData[2][1] = eigenVectors[8][9];
	m_dData[2][2] = eigenVectors[9][9];

	// Undo normalization
#ifdef NORMALIZEWARP
	undoNormalizationWarp(m_dData, T1, T2);
#endif

	// Free up the data
	free_matrix(A, 0, 2*nPoints-1, 0, 9);
	free_matrix(ATA, 1, 9, 1, 9);
	free_matrix(eigenVectors, 1, 9, 1, 9);
	free_vector(eigenValues, 1, 9);

#ifdef NORMALIZEWARP
	free(arrCopySrc);
	free(arrCopyDst);
#endif
}

// Transform the source point by the homography
void CHomography::transform(CVec2d *pSrcPoint, CVec2d *pDstPoint)
{
	double dSrcX = pSrcPoint->x;
	double dSrcY = pSrcPoint->y;

	pDstPoint->x = m_dData[0][0] * dSrcX
		+ m_dData[0][1] * dSrcY + m_dData[0][2];
	pDstPoint->y = m_dData[1][0] * dSrcX
		+ m_dData[1][1] * dSrcY + m_dData[1][2];

	double w = m_dData[2][0] * dSrcX
		+ m_dData[2][1] * dSrcY + m_dData[2][2];

	if(w != 0.0)
	{
		pDstPoint->x /= w;
		pDstPoint->y /= w;
	}
	else
	{
//		char strMessage[256];
//		sprintf(strMessage, "w problem (w = %.3lf)\n", w);
//		printf(strMessage);
	}
}

void CHomography::rescale(double dScale)
{
	for(int ii = 0;ii < 3;ii++)
		for(int jj = 0;jj < 3;jj++)
			m_dData[ii][jj] *= dScale;
}

double CHomography::computeDeterminant()
{
	double v11, v12, v13, v21, v22, v23, v31, v32, v33;

	v11 = m_dData[0][0]; v12 = m_dData[0][1]; v13 = m_dData[0][2];
	v21 = m_dData[1][0]; v22 = m_dData[1][1]; v23 = m_dData[1][2];
	v31 = m_dData[2][0]; v32 = m_dData[2][1]; v33 = m_dData[2][2];

	double dDeterminant = v11*v22*v33-v11*v23*v32-v21*v12*v33+v21*v13*v32+v31*v12*v23-v31*v13*v22;

	return dDeterminant;
}

// Determine the pose and camera parameters from the homography
// If bFixedFocalLength is true, use the passed-in focal lengths... otherwise compute them
void CHomography::computePose(bool bFixedFocalLength, double dFocalU, double dFocalV)
{
//	char strMessage[256];
	CHomography tempHomography;

	// Keep a copy of the matrix data since we will rescale it temporarily
	memcpy(tempHomography.m_dData, m_dData, sizeof(double) * 3 * 3);

	// Rescale the matrix
	double dDeterminant = computeDeterminant();
	double dScale = pow(fabs(1.0 / dDeterminant), 1.0 / 3.0);
	if(dDeterminant < 0.0)
		dScale *= -1.0;

//	sprintf(strMessage, "Det %f rescale %f\n", dDeterminant, dScale);
//	printf(strMessage);

	rescale(dScale);
	dDeterminant = computeDeterminant();
//	sprintf(strMessage, "New determinant %f\n", dDeterminant);
//	printf(strMessage);

	double h11, h12, h13, h21, h22, h23, h31, h32, h33;

	// Get all the values from the matrix
	h11 = m_dData[0][0]; h12 = m_dData[0][1]; h13 = m_dData[0][2];
	h21 = m_dData[1][0]; h22 = m_dData[1][1]; h23 = m_dData[1][2];
	h31 = m_dData[2][0]; h32 = m_dData[2][1]; h33 = m_dData[2][2];

	// Compute the focal lengths
	double d = h11*h12*(h21*h21 - h22*h22) - h21*h22*(h11*h11 - h12*h12);
	double bu = sqrt( fabs((-h31*h32*(h21*h21 - h22*h22) + h21*h22*(h31*h31 - h32*h32)) / d) );
	double bv = sqrt( fabs((h31*h32*(h11*h11 - h12*h12) - h11*h12*(h31*h31 - h32*h32)) / d) );

//	if(bu < 0.00001 || bv < 0.00001)
//		return;

	double au = 0.0;
	double av = 0.0;

	// See if we want fixed focal lengths
	if(bFixedFocalLength)
	{
		au = dFocalU;
		av = dFocalV;

	//	double dCurrentU = 1.0 / bu;
	//	double dCurrentV = 1.0 / bv;
///////////////////
//		sprintf(strMessage, "Focal Lengths: Fixed(%.2lf, %.2lf), Current(%.2lf, %.2lf)\n", au, av, dCurrentU, dCurrentV);
//		printf(strMessage);
///////////////////
	}
	else
	{
		au = 1.0 / bu;
		av = 1.0 / bv;
///////////////////
	//	sprintf(strMessage, "Focal Lengths: Current(%.2lf, %.2lf)\n", au, av);
	//	printf(strMessage);
///////////////////
	}

//	au=1296.3828823831873;av=1269.0840890337493;
	double dLamda = sqrt( 1.0 / ( (h11*h11)/(au*au) + (h21*h21)/(av*av) + (h31*h31) ) );
//	double dOtherLamda = sqrt( 1.0 / ( (h12*h12)/(au*au) + (h22*h22)/(av*av) + (h32*h32) ) );

	// Compute the rotation matrix
	double r11, r12, r13, r21, r22, r23, r31, r32, r33;

	r11 = dLamda * h11 / au; r12 = dLamda * h12 / au;
	r21 = dLamda * h21 / av; r22 = dLamda * h22 / av;
	r31 = dLamda * h31;      r32 = dLamda * h32;

	// Compute the cross product of the two known vectors
	r13 = r21*r32 - r31*r22; // AyBz - AzBy
	r23 = r31*r12 - r11*r32; // AzBx - AxBz
	r33 = r11*r22 - r21*r12; // AxBy - AyBx

	// Compute the translation
	double t1, t2, t3;

	t1 = dLamda * h13 / au;
	t2 = dLamda * h23 / av;
	t3 = dLamda * h33;

//	double dDot1 = r11*r11 + r21*r21 + r31*r31;
//	double dDot2 = r12*r12 + r22*r22 + r32*r32;
//	double dDot3 = r11*r12 + r21*r22 + r31*r32;

/////////////////////
	//sprintf(strMessage, "lamdas(%.2lf, %.2lf), d1:%.2lf, d2:%.2lf, d3:%.2lf, au %.2lf av %.2lf\n", dLamda, dOtherLamda, dDot1, dDot2, dDot3, au, av);
	//printf(strMessage);
/////////////////////

	// Store the values in matrix form (row-major)
	m_matTransformation[0][0] = r11; m_matTransformation[0][1] = r12; 
	m_matTransformation[0][2] = r13; m_matTransformation[0][3] = t1;
	m_matTransformation[1][0] = r21; m_matTransformation[1][1] = r22; 
	m_matTransformation[1][2] = r23; m_matTransformation[1][3] = t2;
	m_matTransformation[2][0] = r31; m_matTransformation[2][1] = r32; 
	m_matTransformation[2][2] = r33; m_matTransformation[2][3] = t3;
	m_matTransformation[3][0] = 0.0; m_matTransformation[3][1] = 0.0; 
	m_matTransformation[3][2] = 0.0; m_matTransformation[3][3] = 1.0;

	m_matProjection[0][0] = au;  m_matProjection[0][1] = 0.0; 
	m_matProjection[0][2] = 0.0; m_matProjection[0][3] = 0.0;
	m_matProjection[1][0] = 0.0; m_matProjection[1][1] = av;  
	m_matProjection[1][2] = 0.0; m_matProjection[1][3] = 0.0;
	m_matProjection[2][0] = 0.0; m_matProjection[2][1] = 0.0; 
	m_matProjection[2][2] = 1.0; m_matProjection[2][3] = 0.0;
	m_matProjection[3][0] = 0.0; m_matProjection[3][1] = 0.0; 
	m_matProjection[3][2] = 0.0; m_matProjection[3][3] = 1.0;

	// Recover the original homography matrix (unscaled)
	memcpy(m_dData, tempHomography.m_dData, sizeof(double) * 3 * 3);
}

/*

//Rotation around X
double GetPitch(void){return asin((_fLambda*_fLambda/_fFu *(_fH31*_fH12 - _fH11*_fH32))/cos(_fRoll));}

//Rotation around Y
double GetRoll(void){return asin(_fLambda*_fLambda * (_fH21*_fH32 - _fH31*_fH22) /_fFv);}

//Rotation around Z
double GetYaw(void){return asin((_fLambda*_fH12)/(_fFu*cos(_fRoll)));}

*/

void CHomography::computeInverse(double matOriginal[3][3], double matInverse[3][3])
{
	// Computing an inverse homography
	CHomography inverseHomography;
	double **tempForward, **tempInverse;

	tempForward = CHomography::matrix(1, 3, 1, 3);
	tempInverse = CHomography::matrix(1, 3, 1, 3);

	for(int row = 1;row <= 3;row++)
	{
		for(int col = 1;col <= 3;col++)
		{
			tempForward[row][col] = matOriginal[row - 1][col - 1];
		}
	}

	inverseHomography.luinvert(tempForward, tempInverse, 3);

	for(int row = 1;row <= 3;row++)
	{
		for(int col = 1;col <= 3;col++)
		{
			matInverse[row - 1][col - 1] = tempInverse[row][col];
		}
	}

	CHomography::free_matrix(tempForward, 1, 3, 1, 3);
	CHomography::free_matrix(tempInverse, 1, 3, 1, 3);
}

void CHomography::computeInverse(double** matOriginal, double** matInverse)
{
	// Computing an inverse homography
	CHomography inverseHomography;
	double **tempForward, **tempInverse;

	tempForward = CHomography::matrix(1, 3, 1, 3);
	tempInverse = CHomography::matrix(1, 3, 1, 3);

	for(int row = 1;row <= 3;row++)
	{
		for(int col = 1;col <= 3;col++)
		{
			tempForward[row][col] = matOriginal[row - 1][col - 1];
		}
	}

	inverseHomography.luinvert(tempForward, tempInverse, 3);

	for(int row = 1;row <= 3;row++)
	{
		for(int col = 1;col <= 3;col++)
		{
			matInverse[row - 1][col - 1] = tempInverse[row][col];
		}
	}

	CHomography::free_matrix(tempForward, 1, 3, 1, 3);
	CHomography::free_matrix(tempInverse, 1, 3, 1, 3);
}

// Private methods

// Computes all eigenvalues and eigenvectors of a real symmetric matrix a[1..n][1..n]. On
// output, elements of a above the diagonal are destroyed. d[1..n] returns the eigenvalues of a.
// v[1..n][1..n] is a matrix whose columns contain, on output, the normalized eigenvectors of
// a. nrot returns the number of Jacobi rotations that were required.
int CHomography::jacobi(double **a, int n, double d[], double **v, int *nrot)
{
	int j, iq, ip, i;
	double tresh, theta, tau, t, sm, s, h, g, c;

	double b[1024];
	double z[1024];

	for(ip = 1;ip <= n;ip++)
	{
		for(iq = 1;iq <= n;iq++)
			v[ip][iq] = 0.0;

		v[ip][ip] = 1.0;
	}
	for (ip = 1;ip <= n;ip++)
	{
		b[ip] = d[ip] = a[ip][ip];
		z[ip] = 0.0;
	}

	*nrot = 0;

	for(i = 1;i <= 50;i++)
	{
		sm=0.0;

		for(ip = 1;ip <= n-1;ip++)
		{
			for(iq = ip + 1;iq <= n;iq++)
				sm += fabs(a[ip][iq]);
		}

		if(sm == 0.0)
		{
			return 1;
		}

		if(i < 4)
			tresh = 0.2*sm/(n*n);
		else
			tresh = 0.0;

		for(ip = 1;ip <= n-1;ip++)
		{
			for(iq = ip + 1;iq <= n;iq++)
			{
				g = 100.0*fabs(a[ip][iq]);
				if(i > 4 && fabs(d[ip]) + g == fabs(d[ip]) && fabs(d[iq]) + g == fabs(d[iq]))
					a[ip][iq] = 0.0;
				else if (fabs(a[ip][iq]) > tresh)
				{
					h = d[iq]-d[ip];
					if(fabs(h) + g == fabs(h))
						t = (a[ip][iq]) / h;
					else
					{
						theta = 0.5*h/(a[ip][iq]);
						t = 1.0 / (fabs(theta) + sqrt(1.0+theta*theta));
						if(theta < 0.0)
							t = -t;
					}

					c = 1.0/sqrt(1+t*t);
					s = t*c;
					tau = s/(1.0+c);
					h = t*a[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					a[ip][iq]=0.0;

					for(j = 1; j <= ip - 1;j++)
					{
						JACOBI_ROTATE(a, j, ip, j, iq)
					}
					for(j = ip + 1;j <= iq - 1;j++)
					{
						JACOBI_ROTATE(a, ip, j, j, iq)
					}
					for(j = iq+1;j <= n;j++)
					{
						JACOBI_ROTATE(a, ip, j, iq, j)
					}
					for(j = 1;j <= n;j++)
					{
						JACOBI_ROTATE(v, j, ip, j, iq)
					}

					++(*nrot);
				}
			}
		}

		for(ip = 1;ip <= n;ip++)
		{
			b[ip] += z[ip];
			d[ip] = b[ip];
			z[ip] = 0.0;
		}
	}

//	char strMessage[256];
//	sprintf("Too many iterations in routine JACOBI");
//	printf(strMessage);

	return 0;
}

// Given the eigenvalues d[1..n] and eigenvectors v[1..n][1..n] as output from jacobi,
// this routine sorts the eigenvalues into descending order, and rearranges
// the columns of v correspondingly. The method is straight insertion.
void CHomography::eigsrt(double d[], double **v, int n)
{
	int k, j, i;
	double p;

	for(i = 1;i < n;i++)
	{
		p = d[k=i];

		for(j = i+1;j <= n;j++)
		{
			if(d[j] >= p)
				p = d[k=j];
		}

		if(k != i)
		{
			d[k] = d[i];
			d[i] = p;

			for(j = 1;j <= n;j++)
			{
				p = v[j][i];
				v[j][i] = v[j][k];
				v[j][k] = p;
			}
		}
	}
}

// Normalize data (destroys the contents of matches, so ensure that you
// have passed in a copy)
void CHomography::normalizeData(CVec2d *arrSrcPoints, CVec2d *arrDstPoints, int nPoints, double T1[3][3], double T2[3][3])
{
	double scale1,scale2;
	double centroid[4]; //[x1,y1,x2,y2]
	int i, nRow, nCol;

	scale1 = scale2 = 0;

	centroid[0] = centroid[1] = centroid[2] = centroid[3] = 0;

	for(nRow = 0;nRow < 3;nRow++)
	{
		for(nCol = 0;nCol < 3;nCol++)
		{
			T1[nRow][nCol] = 0;
			T2[nRow][nCol] = 0;
		}
	}

	for(i = 0;i < nPoints;i++)
	{
		centroid[0] += arrSrcPoints[i].x;
		centroid[1] += arrSrcPoints[i].y;
		centroid[2] += arrDstPoints[i].x;
		centroid[3] += arrDstPoints[i].y;
		scale1 += arrSrcPoints[i].x*arrSrcPoints[i].x + arrSrcPoints[i].y*arrSrcPoints[i].y;
		scale2 += arrDstPoints[i].x*arrDstPoints[i].x + arrDstPoints[i].y*arrDstPoints[i].y;
	}

	centroid[0] /= nPoints;
	centroid[1] /= nPoints;
	centroid[2] /= nPoints;
	centroid[3] /= nPoints;

	scale1 = sqrt(scale1/(2*nPoints));
	scale2 = sqrt(scale2/(2*nPoints));

	T1[0][0] = 1/scale1;
	T1[1][1] = 1/scale1;
	T1[2][2] = 1;
	T1[0][2] = -centroid[0]/scale1;
	T1[1][2] = -centroid[1]/scale1;

	T2[0][0] = 1/scale2;
	T2[1][1] = 1/scale2;
	T2[2][2] = 1;
	T2[0][2] = -centroid[2]/scale2;
	T2[1][2] = -centroid[3]/scale2;

	for(i = 0;i < nPoints;i++)
	{
		arrSrcPoints[i].x -= centroid[0];
		arrSrcPoints[i].y -= centroid[1];
		arrDstPoints[i].x -= centroid[2];
		arrDstPoints[i].y -= centroid[3];
		arrSrcPoints[i].x /= scale1;
		arrSrcPoints[i].y /= scale1;
		arrDstPoints[i].x /= scale2;
		arrDstPoints[i].y /= scale2;

	}
}


// undoNormalizationWarp, changes fMat to be the new unNormalized version
void CHomography::undoNormalizationWarp(double fMat[3][3], double T1[3][3], double T2[3][3])
{
	int i,j,k;
	double T2I[3][3];
	double fMatTemp[3][3];

	// Compute T2 inverse
	for (i=0; i < 3; i++)
		for (j=0; j < 3; j++)
			T2I[j][i] = 0;

	// Inverse of similarity transform
	computeInverse(T2, T2I);
	/*
	T2I[0][0] = 1/T2[0][0];
	T2I[1][1] = 1/T2[1][1];
	T2I[2][2] = 1.0;
	T2I[0][2] = -T2[0][2]/T2[0][0];
	T2I[1][2] = -T2[1][2]/T2[1][1];
	*/
#ifdef PRINTDATA
	printf("\n\nRIGHT NORMALIZATION TRANSFORMATION MATRIX\n");
	for (i=0; i < 3; i++) {
		for (j=0; j < 3; j++)
			printf("%lf ",T2[i][j]);
		printf("\n");
	}
	printf("\n\nLEFT NORMALIZATION TRANSFORMATION MATRIX\n");
	for (i=0; i < 3; i++) {
		for (j=0; j < 3; j++)
			printf("%lf ",T1[i][j]);
		printf("\n");
	}
#endif

	// Compute T2(transpose)*F*T1
	for (i=0; i < 3; i++)
		for (j=0; j < 3; j++)
		{
			fMatTemp[i][j] = 0;
			for (k=0;k<3; k++)
				fMatTemp[i][j] += T2I[i][k] * fMat[k][j];
		}

	for (i=0; i < 3; i++)
		for (j=0; j < 3; j++)
		{
			fMat[i][j] = 0;
			for (k=0;k<3; k++)
				fMat[i][j] += fMatTemp[i][k] * T1[k][j];
		}

#ifdef PRINTDATA
	printf("\n\nFinal Adjusted fund MATRIX\n");
	for (i=0; i < 3; i++)
	{
		for (j=0; j < 3; j++)
			printf("%lf ",fMat[i][j]);
		printf("\n");
	}

#endif
}

/*
Given a matrix a[1..n][1..n], this routine replaces it by the LU decomposition of a rowwise
permutation of itself. a and n are input. a is output, arranged as in equation (2.3.14) above;
indx[1..n] is an output vector that records the row permutation effected by the partial
pivoting; d is output as . 1 depending on whether the number of row interchanges was even
or odd, respectively. This routine is used in combination with lubksb to solve linear equations
or invert a matrix.
*/
void CHomography::ludcmp(double **a, int n, int *indx, double *d)
{
	int i, imax, j, k;
	double big, dum, sum, temp;
	double *vv; // vv stores the implicit scaling of each row.

	vv = vector(1,n);

	*d = 1.0; // No row interchanges yet.

	for(i = 1;i <= n;i++)
	{
		// Loop over rows to get the implicit scaling information.
		big = 0.0;

		for(j = 1;j <= n;j++)
		{
			if ((temp = fabs(a[i][j])) > big)
				big = temp;
		}

		if(big == 0.0)
			return; //nrerror("Singular matrix in routine ludcmp");

		// No nonzero largest element.
		vv[i] = 1.0 / big; // Save the scaling.
	}

	for(j = 1;j <= n;j++)
	{
		// This is the loop over columns of Crout's method.
		for(i = 1;i < j;i++)
		{
			// This is equation (2.3.12) except for i = j.
			sum = a[i][j];

			for(k = 1;k < i;k++)
				sum -= a[i][k]*a[k][j];

			a[i][j]=sum;
		}

		big=0.0; // Initialize for the search for largest pivot element.

		for(i = j;i <= n;i++)
		{
			// This is i = j of equation (2.3.12) and i = j +1: ::N of equation (2.3.13).
			sum = a[i][j];
			for(k = 1;k < j;k++)
				sum -= a[i][k]*a[k][j];

			a[i][j] = sum;
			if ( (dum = vv[i]*fabs(sum)) >= big)
			{
				// Is the figure of merit for the pivot better than the best so far?
				big=dum;
				imax=i;
			}
		}

		if(j != imax)
		{
			// Do we need to interchange rows?
			for(k = 1;k <= n;k++)
			{
				// Yes, do so...
				dum = a[imax][k];
				a[imax][k] = a[j][k];
				a[j][k] = dum;
			}

			*d = -(*d); // ...and change the parity of d.
			vv[imax] = vv[j]; // Also interchange the scale factor.
		}

		indx[j] = imax;

		if(a[j][j] == 0.0)
			a[j][j] = 0.00001; //TINY;

		// If the pivot element is zero the matrix is singular (at least to the precision of the
		// algorithm). For some applications on singular matrices, it is desirable to substitute
		// TINY for zero.
		if(j != n)
		{
			// Now, finally, divide by the pivot element.
			dum = 1.0/(a[j][j]);
			for(i = j+1;i <= n;i++)
				a[i][j] *= dum;
		}
	}

	free_vector(vv,1,n);
}


/*
Solves the set of n linear equations A . X = B. Herea[1..n][1..n] is input, not as the matrix
A but rather as its LU decomposition, determined by the routine ludcmp. indx[1..n] is input
as the permutation vector returned by ludcmp. b[1..n] is input as the right-hand side vector
B, and returns with the solution vector X. a, n, andindx are not modified by this routine
and can be left in place for successive calls with difierent right-hand sides b. This routine takes
into account the possibility that b will begin with many zero elements, so it is e.cient for use
in matrix inversion.
*/
void CHomography::lubksb(double **a, int n, int *indx, double b[])
{
	int i,ii=0,ip,j;
	double sum;

	for(i = 1;i <= n;i++)
	{
		// When ii is set to a positive value, it will become the
		// index of the first nonvanishing element of b. Wenow
		// do the forward substitution, equation (2.3.6). The
		// only new wrinkle is to unscramble the permutation
		// as we go.
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];

		if(ii)
		{
			for(j=ii;j<=i-1;j++)
			{
				sum -= a[i][j]*b[j];
			}
		}
		else if(sum)
		{
			ii = i;	// A nonzero element was encountered, so from now on we
						// will have to do the sums in the loop above.
		}

		b[i] = sum;
	}

	for (i = n;i >= 1;i--)
	{
		// Now we do the backsubstitution, equation (2.3.7).
		sum = b[i];
		for(j = i + 1;j <= n;j++)
			sum -= a[i][j]*b[j];

		b[i] = sum / a[i][i]; // Store a component of the solution vector X.
	}
}

void CHomography::luinvert(double **a, double **y, int n)
{
  double col[100], d;
  int i, j, indx[100];

  ludcmp(a, n, indx, &d);

  for(j = 1;j <= n;j++)
  {
    for(i = 1;i <= n;i++)
		 col[i] = 0;

	 col[j] = 1.0;

	 lubksb(a, n, indx, col);

	 for(i = 1;i <= n;i++)
		 y[i][j] = col[i];
  }

}




