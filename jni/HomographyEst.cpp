#include "HomographyEst.h"

void CHomographyEst::HomograhyEstimation(std::vector<corsp2i>& inlierMap, CvMat* H)
{
	int i;
	int num = (int)inlierMap.size();

	point2f* pt1 = new point2f[num];
	point2f* pt2 = new point2f[num];

	for( i = 0 ; i < num ; i++ )
	{
		pt1[i].x = (float)inlierMap[i].x1;
		pt1[i].y = (float)inlierMap[i].y1;
		pt2[i].x = (float)inlierMap[i].x2;
		pt2[i].y = (float)inlierMap[i].y2;
	}

	// compute the homography
	//ComputeHomography(pt2, pt1, num, H);
	ComputeHomographyOpenCV(pt2, pt1, num, H);

	delete [] pt1;
	delete [] pt2;
}

void CHomographyEst::RansacHomograhyEstimation(std::vector<corsp2i>& corspMap, std::vector<corsp2i>& inlierMap, CvMat* H)
{
	const int num = 4;
	point2f pt1[num];
	point2f pt2[num];

	int i, pos; 

	int numOfInliers;
	int totalNumOfPoints = (int)corspMap.size();
	int maxNumOfInliers = 1;

	std::vector<corsp2i> tempInlierMap;

	CvMat* Htmp = cvCreateMat(3, 3, CV_32FC1);

	float p = 0.99f;
	float e = 0.5f;
	float N = 1000.f;
	float outlierProb;

	int badCount = 0;
	int sampleCount = 0;

	while( badCount < 50 && (float)sampleCount < N )
	{
		// pick 4 corresponding points
		for( i = 0 ; i < num ; i++ )
		{
			pos = rand() % (int)corspMap.size(); // select random positions

			pt1[i].x = (float)corspMap[pos].x1;
			pt1[i].y = (float)corspMap[pos].y1;
			pt2[i].x = (float)corspMap[pos].x2;
			pt2[i].y = (float)corspMap[pos].y2;
		}

		// check whether samples are good or not.
		// if the selected samples are good, then do homography estimation
		// else reselect samples.
		if( IsGoodSample(pt1, num) && IsGoodSample(pt2, num) )
		{
			// compute the homography
			//ComputeHomography(pt2, pt1, num, Htmp);
			ComputeHomographyOpenCV(pt2, pt1, num, Htmp);

			// calculate the distance for each correspondences
			// compute the number of inliers
			// InitializeCorspMap(&tempInlierMap);
			//memset(&tempInlierMap, 0, sizeof(CorspMap));
			tempInlierMap.clear();
			
			CalculateDistance(Htmp, corspMap, tempInlierMap);

			// choose H with the largest number of inliears
			numOfInliers = (int)tempInlierMap.size();
			if( numOfInliers >= maxNumOfInliers )
			{
				maxNumOfInliers = numOfInliers;
				//CopyCorspMap(inlierMap, &tempInlierMap);
				//memcpy(inlierMap, &tempInlierMap, sizeof(CorspMap));
				inlierMap = tempInlierMap;
				cvCopy(Htmp, H, 0);
			}

			// adaptive algorithm for determining the number of RANSAC samples
			// textbook algorithm 4.6

			totalNumOfPoints = (int)corspMap.size();
			outlierProb = 1 - ((float)maxNumOfInliers / (float)totalNumOfPoints);
			e = (e < outlierProb) ? e : outlierProb;
			N = log(1 - p) / log(1 - pow((1 - e), num));
			sampleCount++;
		}
		else
		{
			badCount++;
		}
	}

	cvReleaseMat(&Htmp);
}

void CHomographyEst::ComputeHomographyOpenCV(point2f* pt1, point2f* pt2, int num, CvMat* H)
{
	//
	// opencv를 이용하는 Homography 계산으로 수정 by 동철
	CvMat *srcMat, *dstMat;
	srcMat = cvCreateMat(3, num, CV_32FC1);
	dstMat = cvCreateMat(3, num, CV_32FC1);

	int i;	
	for(i=0; i<num; i++)
	{
		cvmSet(srcMat, 0, i, pt1[i].x );
		cvmSet(srcMat, 1, i, pt1[i].y );
		cvmSet(srcMat, 2, i, 1.f );
		cvmSet(dstMat, 0, i, pt2[i].x );
		cvmSet(dstMat, 1, i, pt2[i].y );
		cvmSet(dstMat, 2, i, 1.f );
	}

	cvFindHomography( srcMat, dstMat, H );

	cvReleaseMat(&srcMat);
	cvReleaseMat(&dstMat);
}

//
// function : ComputeHomography
// usage : ComputeHomography(domainPositions, rangePositions,
// numOfCorresp, H);
// -----------------------------------------------------------
// This function calculate the homography, H, using the set of
// given pairs of correspondences.
// Before computing the homography, data normalization will be
// performed. Then, it solve Ah = 0 using SVD to get H.
//
void CHomographyEst::ComputeHomography(point2f* pt1, point2f* pt2, int num, CvMat* H)
{
	int col = 9, row;
	float x1, y1, w1, x2, y2, w2;
	int i, ii, jj;
	float h[9];

	if( num == 4 )
	{
		row = 3;	// eq 4.1 : make a thin matrix to solve SVD in opencv
	}
	else if( num > 4 )
	{
		row = 2;	//eq 4.3
	}
	else
	{
		printf("Need more correspondence points! for computing H.\n");
		exit(0);
	}

	float* a = new float[row * col];

	// normalization
	CvMat* T1 = cvCreateMat(3, 3, CV_32FC1);
	CvMat* T2 = cvCreateMat(3, 3, CV_32FC1);

	DataNormalization(num, pt1, T1);
	DataNormalization(num, pt2, T2);

	// set A
	CvMat* A = cvCreateMat(num * row, col, CV_32FC1);

	for( i = 0 ; i < num ; i++ )
	{
		x1 = pt1[i].x;
		y1 = pt1[i].y;
		w1 = 1;

		x2 = pt2[i].x;
		y2 = pt2[i].y;
		w2 = 1;

		// set Ai
		// [0, 0, 0, -w2*x1, -w2*y1, -w2*w1, y2*x1, y2*y1, y2*w1]
		// [w2*x1, w2*y1, w2*w1, 0, 0, 0, -x2*x1, -x2*y1, -x2*w1]
		a[0] = 0;       a[1] = 0;       a[2] = 0;
		a[3] = -w2*x1;  a[4] = -w2*y1;  a[5] = -w2*w1;
		a[6] = y2*x1;   a[7] = y2*y1;   a[8] = y2*w1;

		a[9] = w2*x1;   a[10] = w2*y1;  a[11] = w2*w1;
		a[12] = 0;      a[13] = 0;      a[14] = 0;
		a[15] = -x2*x1; a[16] = -x2*y1; a[17] = -x2*w1;

		if( row == 3 ) // eq 4.1 : make a thin matrix to solve SVD in opencv
		{
			a[18] = -y2*x1; a[19] = -y2*y1; a[20] = -y2*w1;
			a[21] = x2*x1;  a[22] = x2*y1;  a[23] = x2*w1;
			a[24] = 0;      a[25] = 0;      a[26] = 0;
		}

		// assemble Ai into a matrix A
		for( jj = 0 ; jj < row    ; jj++ )
		for( ii = 0 ; ii < col ; ii++ )
		{
			cvmSet(A, jj + i*row, ii, a[jj*col + ii]);
		}
	}

	// calculate H
	CvMat* Htmp = cvCreateMat(3, 3, CV_32FC1);
	CvMat* D = cvCreateMat(num*row, col, CV_32FC1);
	CvMat* U = cvCreateMat(num*row, num*row, CV_32FC1);
	CvMat* V = cvCreateMat(col, col, CV_32FC1);

	cvSVD(A, D, U, V, CV_SVD_U_T|CV_SVD_V_T); // A = U^T D V : opencv setting

	// take last column of V
	for( i = 0 ; i < col ; i++)
		h[i] = (float)cvmGet(V, col-1, i);

	Array2CvMat(h, Htmp, 3, 3);

	// denormalization : H = invT2 * Htmp * T1 <- Htmp = T2 * H * invT1
	CvMat* invT2 = cvCreateMat(3, 3, CV_32FC1);
	CvMat* temp  = cvCreateMat(3, 3, CV_32FC1);

	cvInvert(T2, invT2);

	cvMatMul(invT2, Htmp, temp);
	cvMatMul(temp, T1, H);

	// release matrices
	cvReleaseMat(&T1); cvReleaseMat(&T2);
	cvReleaseMat(&A);  cvReleaseMat(&Htmp);
	cvReleaseMat(&D);  cvReleaseMat(&U);  cvReleaseMat(&V);
	cvReleaseMat(&invT2); cvReleaseMat(&temp);

	delete [] a;
}

//
// function : DataNormalization
// usage : DataNormalization(numOfx, x, T);
// ------------------------------------------------------
// This function normalizes x and returns the similarity
// transform, T.
// The centroid of x will be transformed into (0,0).
// The average distance of normalized x will be sqrt(2).
//
void CHomographyEst::DataNormalization(int num, point2f* points, CvMat* T)
{
	int i;

	//-------------------------------------------------------------------------
	// calculate the centroid
	//-------------------------------------------------------------------------

	float sumX, sumY, meanX, meanY;

	sumX = sumY = 0;
	for( i = 0 ; i < num ; i++ ) 
	{
		sumX += points[i].x;
		sumY += points[i].y;
	}

	meanX = sumX / num;
	meanY = sumY / num;

	//-------------------------------------------------------------------------
	// calculate the mean distance
	//-------------------------------------------------------------------------

	float squareDist, sumDist, meanDist;

	squareDist = sumDist = meanDist = 0;
	for( i = 0 ; i < num ; i++ )
	{
		squareDist = (points[i].x - meanX)*(points[i].x - meanX) + 
			(points[i].y - meanY)*(points[i].y - meanY);
		sumDist += sqrt(squareDist);
	}

	meanDist = sumDist / num;

	//-------------------------------------------------------------------------
	// set the similarity transform
	//-------------------------------------------------------------------------

	float scale = 1.f / meanDist;

	float t[9] = {scale, 0, -scale * meanX,
		0, scale, -scale * meanY,
		0, 0, 1};

	Array2CvMat(t, T, 3, 3);

	//-------------------------------------------------------------------------
	// data normalization
	//-------------------------------------------------------------------------

	float x, y, xx, yy, ww;
	for( i = 0 ; i < num ; i++ )
	{
		x = points[i].x;
		y = points[i].y;

		xx = t[0] * x + t[1] * y + t[2];
		yy = t[3] * x + t[4] * y + t[5];
		ww = t[6] * x + t[7] * y + t[8];

		xx /= ww;
		yy /= ww;

		points[i].x = xx;
		points[i].y = yy;
	}
}

//
// function : CalculateDistance
// usage : CalculateDistance(H, corspMap, inlierMap);
// ---------------------------------------------------
// This function calculates distance of data using
// symmetric transfer error. Then, compute inliers
// that consist with H.
//
void CHomographyEst::CalculateDistance(CvMat *H, std::vector<corsp2i>& corspMap, std::vector<corsp2i>& inlierMap)
{
	int i;
	int x1, y1, x2, y2;
	double x1Trans, y1Trans, w1Trans, x2Trans, y2Trans, w2Trans;
	double dist2x1AndInvHx2, dist2x2AndHx1, dist2Trans;

	CvMat* invH = cvCreateMat(3, 3, CV_32FC1);
	cvInvert(H, invH);

	// use d^2_transfer as distance measure
	for( i = 0 ; i < (int)corspMap.size() ; i++ )
	{
		x1 = corspMap[i].x2;
		y1 = corspMap[i].y2;
		x2 = corspMap[i].x1;
		y2 = corspMap[i].y1;

		// calculate x_trans = H * x
		x2Trans = cvmGet(H, 0, 0) * x1 + cvmGet(H, 0, 1) * y1 + cvmGet(H, 0, 2);
		y2Trans = cvmGet(H, 1, 0) * x1 + cvmGet(H, 1, 1) * y1 + cvmGet(H, 1, 2);
		w2Trans = cvmGet(H, 2, 0) * x1 + cvmGet(H, 2, 1) * y1 + cvmGet(H, 2, 2);
		x2Trans = x2Trans / w2Trans;
		y2Trans = y2Trans / w2Trans;

		// calculate x'_trans = H^(-1) * x'
		x1Trans = cvmGet(invH, 0, 0) * x2 + cvmGet(invH, 0, 1) * y2 + cvmGet(invH, 0, 2);
		y1Trans = cvmGet(invH, 1, 0) * x2 + cvmGet(invH, 1, 1) * y2 + cvmGet(invH, 1, 2);
		w1Trans = cvmGet(invH, 2, 0) * x2 + cvmGet(invH, 2, 1) * y2 + cvmGet(invH, 2, 2);
		x1Trans = x1Trans / w1Trans;
		y1Trans = y1Trans / w1Trans;

		// calculate the square distance (symmetric transfer error)
		dist2x1AndInvHx2 = (x1 - x1Trans)*(x1 - x1Trans) + (y1 - y1Trans)*(y1 - y1Trans);
		dist2x2AndHx1 = (x2 - x2Trans)*(x2 - x2Trans) + (y2 - y2Trans)*(y2 - y2Trans);
		dist2Trans = dist2x1AndInvHx2 + dist2x2AndHx1;

		if( dist2Trans < T_SQUARE )
			UpdateCorrespMap(inlierMap, x1, y1, x2, y2);
	}

	// release matrices
	cvReleaseMat(&invH);
}

//
// function : IsGoodSample
// usage : r = IsGoodSample(points, num)
// -------------------------------------------------
// This function checks colinearity of all given points.
//
bool CHomographyEst::IsGoodSample(point2f* points, int num)
{
	bool ret = false;
	int i, j, k;

	CvMat* A = cvCreateMat(3, 1, CV_32FC1);
	CvMat* B = cvCreateMat(3, 1, CV_32FC1);
	CvMat* C = cvCreateMat(3, 1, CV_32FC1);

	i = 0;
	j = i + 1;
	k = j + 1;

	// check colinearity recursively
	while(true)
	{
		// set point vectors
		cvmSet(A, 0, 0, points[i].x);
		cvmSet(A, 1, 0, points[i].y);
		cvmSet(A, 2, 0, 1);
		cvmSet(B, 0, 0, points[j].x);
		cvmSet(B, 1, 0, points[j].y);
		cvmSet(B, 2, 0, 1);
		cvmSet(C, 0, 0, points[k].x);
		cvmSet(C, 1, 0, points[k].y);
		cvmSet(C, 2, 0, 1);

		// check linearity
		ret = IsColinear(A, B, C) || ret;

		// update point index
		if( k < num - 1 )
		{
			k += 1;
		}
		else
		{
			if( j < num - 2 )
			{
				j += 1;
				k = j + 1;
			}
			else
			{
				if( i < num - 3 )
				{
					i += 1;
					j = i + 1;
					k = j + 1;
				}
				else
				{
					break;
				}
			}
		}
	}

	cvReleaseMat(&A);
	cvReleaseMat(&B);
	cvReleaseMat(&C);

	return(!ret);
}

//
// function : IsColinear
// usage : r = IsColinear(A, B, C);
// --------------------------------------
// This function checks the colinearity of
// the given 3 points A, B, and C.
// If these are colinear, it returns false. (true 반환해야하는거 아냐?)
//
bool CHomographyEst::IsColinear(CvMat *A, CvMat *B, CvMat *C)
{
	bool ret = false;

	CvMat* lineAB = cvCreateMat(3, 1, CV_32FC1);
	cvCrossProduct(A, B, lineAB);

	double d = cvDotProduct(lineAB, C);

	if( (d < EPS) && (d > -EPS) )
		ret = true;

	// release matrices
	cvReleaseMat(&lineAB);

	return ret;
}

//-------------------------------------------------------------------------
// Compute extrinsic camera parameter matrix from planar homography
//-------------------------------------------------------------------------
void CHomographyEst::ComputeCameraMat(CvMat* H, CvMat* M)
{
	double h11, h12, h13, h21, h22, h23, h31, h32, h33;
	double r11, r12, r13, r21, r22, r23, r31, r32, r33;
	double t1, t2, t3;

	double fu, fv, lambda;

	//-------------------------------------------------------------------------
	// h11 ~ h33 setting
	//-------------------------------------------------------------------------

	h11 = H->data.fl[0]; h12 = H->data.fl[1]; h13 = H->data.fl[2];
	h21 = H->data.fl[3]; h22 = H->data.fl[4]; h23 = H->data.fl[5];
	h31 = H->data.fl[6]; h32 = H->data.fl[7]; h33 = H->data.fl[8];

	//-------------------------------------------------------------------------
	// fu, fv, lambda
	//-------------------------------------------------------------------------

	double up =  h11*h12*(h21*h21-h22*h22) - h21*h22*(h11*h11-h12*h12);
	double du = -h31*h32*(h21*h21-h22*h22) + h21*h22*(h31*h31-h32*h32);
	double dv = -h31*h32*(h11*h11-h12*h12) + h11*h12*(h31*h31-h32*h32);

	fu = sqrt(up/du);
	fv = sqrt(up/dv);
	lambda = 1 / sqrt(h11*h11/(fu*fu) + h21*h21/(fv*fv) + h31*h31);

	//-------------------------------------------------------------------------
	// [R|t]
	//-------------------------------------------------------------------------

	r11 = lambda*h11/fu;
	r12 = lambda*h12/fu;
	r21 = lambda*h21/fv;
	r22 = lambda*h22/fv;
	r31 = lambda*h31;
	r32 = lambda*h32;
	r13 = r21*r32 - r31*r22;
	r23 = r31*r12 - r11*r32;
	r33 = r11*r22 - r21*r12;
	t1  = lambda*h13/fu;
	t2  = lambda*h23/fv;
	t3  = lambda*h33;

	//-------------------------------------------------------------------------
	// [R|t] -> M
	//-------------------------------------------------------------------------

	cvmSet(M, 0, 0, r11);
	cvmSet(M, 0, 1, r12);
	cvmSet(M, 0, 2, r13);
	cvmSet(M, 0, 3, t1);
	cvmSet(M, 1, 0, r21);
	cvmSet(M, 1, 1, r22);
	cvmSet(M, 1, 2, r23);
	cvmSet(M, 1, 3, t2);
	cvmSet(M, 2, 0, r31);
	cvmSet(M, 2, 1, r32);
	cvmSet(M, 2, 2, r33);
	cvmSet(M, 2, 3, t3);
}


//-------------------------------------------------------------------------

void CHomographyEst::Array2CvMat(float *arr, CvMat *cvArr, int row, int col)
{
	int i, j;
	for( j = 0 ; j < row ; j++ )
		for( i = 0 ; i < col ; i++ )
			cvmSet(cvArr, j, i, arr[j*col + i]);
}

void CHomographyEst::CvMat2Array(CvMat *cvArr, float *arr, int row, int col)
{
	int i, j;
	for( j = 0 ; j < row ; j++ )
		for( i = 0 ; i < col ; i++ )
			arr[j*col + i] = (float)cvmGet(cvArr, j, i);
}

void CHomographyEst::UpdateCorrespMap(std::vector<corsp2i>& corspMap, int x1, int y1, int x2, int y2)
{
	int num = (int)corspMap.size();

	if( (int)corspMap.size() >= MAX_KEY_POINTS )
	{
		printf("UpdateCorrespMap called on a full corspMap\n");
		printf("Next positions of correspondences will be overwritten\n");
		printf("in the current correspondence \n");

		num = MAX_KEY_POINTS - 1;
	}

//	corspMap.push_back( corsp2i(x1, y1, x2, y2) );
	corspMap.push_back( corsp2i(x2, y2, x1, y1) );
}

