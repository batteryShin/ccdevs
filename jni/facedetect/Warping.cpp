#include "Warping.h"

void CWarping::PerspectiveTransform(double** src, double** dst, CvSize elem_size, CvMat* matH)
{
	CvMat* matHinv = cvCreateMat( 3, 3, CV_64FC1 );
	cvInvert(matH, matHinv, CV_SVD);

	int w = elem_size.width;
	int h = elem_size.height;

	int x1, y1, x2, y2;
	double normpos;
	double rx, ry, p, q;
	double temp_v;

	int j, i;
	for(j=-h/2; j<h/2; j++)
	{
		for(i=-w/2; i<w/2; i++)
		{
			normpos = cvmGet(matHinv,2,0)*i + cvmGet(matHinv,2,1)*j + cvmGet(matHinv,2,2);
			rx = (cvmGet(matHinv,0,0)*i + cvmGet(matHinv,0,1)*j + cvmGet(matHinv,0,2)) / normpos;
			ry = (cvmGet(matHinv,1,0)*i + cvmGet(matHinv,1,1)*j + cvmGet(matHinv,1,2)) / normpos;

			if( rx>0 )
				x1 = (int)rx;
			else
				x1 = (int)rx-1;

			if( ry>0 )
				y1 = (int)ry;
			else
				y1 = (int)ry-1;

			x2 = x1 + 1;
			y2 = y1 + 1;

			p = rx - x1;
			q = ry - y1;

			if( x1<-w/2 || x1>=w/2 || x2<-w/2 || x2>=w/2 || y1<-h/2 || y1>=h/2 || y2<-h/2 || y2>=h/2 )
			{
				temp_v = 0;
			}
			else
			{
				x1 += w/2;	x2 += w/2;
				y1 += h/2;	y2 += h/2;

				temp_v = (1.0-p)*(1.0-q)*src[y1][x1] + p*(1.0-q)*src[y1][x2]
				+ (1.0-p)*q*src[y2][x1] + p*q*src[y2][x2];
			}

			dst[j+h/2][i+w/2] = temp_v;
		}
	}

	cvReleaseMat(&matHinv);
}

void CWarping::PerspectiveTransform(IplImage* src, IplImage* dst, CvMat* matH)
{
	CvMat* matHinv = cvCreateMat( 3, 3, matH->type );
	cvInvert(matH, matHinv, CV_SVD);

	int w = dst->width;
	int h = dst->height;

	int x1, y1, x2, y2;
	double normpos;
	double rx, ry, p, q;
	double temp_r, temp_g, temp_b;

	int j, i;
	if( dst->nChannels==1 )
	{
		BwImage ptr1(src);
		BwImage ptr2(dst);

		for(j=-h/2; j<h/2; j++)
		{
			for(i=-w/2; i<w/2; i++)
			{
				normpos = cvmGet(matHinv,2,0)*i + cvmGet(matHinv,2,1)*j + cvmGet(matHinv,2,2);
				rx = (cvmGet(matHinv,0,0)*i + cvmGet(matHinv,0,1)*j + cvmGet(matHinv,0,2)) / normpos;
				ry = (cvmGet(matHinv,1,0)*i + cvmGet(matHinv,1,1)*j + cvmGet(matHinv,1,2)) / normpos;

				if( rx>0 )
					x1 = (int)rx;
				else
					x1 = (int)rx-1;

				if( ry>0 )
					y1 = (int)ry;
				else
					y1 = (int)ry-1;

				x2 = x1 + 1;
				y2 = y1 + 1;

				p = rx - x1;
				q = ry - y1;

				if( x1<-w/2 || x1>=w/2 || x2<-w/2 || x2>=w/2 || y1<-h/2 || y1>=h/2 || y2<-h/2 || y2>=h/2 )
				{
					temp_r = 0;
				}
				else
				{
					x1 += w/2;	x2 += w/2;
					y1 += h/2;	y2 += h/2;

					temp_r = (1.0-p)*(1.0-q)*ptr1[y1][x1] + p*(1.0-q)*ptr1[y1][x2]
					+ (1.0-p)*q*ptr1[y2][x1] + p*q*ptr1[y2][x2];
				}

				ptr2[j+h/2][i+w/2] = (BYTE)temp_r;
			}
		}
	}
	else if( dst->nChannels==3 )
	{
		RgbImage ptr1(src);
		RgbImage ptr2(dst);

		for(j=-h/2; j<h/2; j++)
		{
			for(i=-w/2; i<w/2; i++)
			{
				normpos = cvmGet(matHinv,2,0)*i + cvmGet(matHinv,2,1)*j + cvmGet(matHinv,2,2);
				rx = (cvmGet(matHinv,0,0)*i + cvmGet(matHinv,0,1)*j + cvmGet(matHinv,0,2)) / normpos;
				ry = (cvmGet(matHinv,1,0)*i + cvmGet(matHinv,1,1)*j + cvmGet(matHinv,1,2)) / normpos;

				if( rx>0 )
					x1 = (int)rx;
				else
					x1 = (int)rx-1;

				if( ry>0 )
					y1 = (int)ry;
				else
					y1 = (int)ry-1;

				x2 = x1 + 1;
				y2 = y1 + 1;

				p = rx - x1;
				q = ry - y1;

				if( x1<-w/2 || x1>=w/2 || x2<-w/2 || x2>=w/2 || y1<-h/2 || y1>=h/2 || y2<-h/2 || y2>=h/2 )
				{
					temp_r = temp_g = temp_b = 0;
				}
				else
				{
					x1 += w/2;	x2 += w/2;
					y1 += h/2;	y2 += h/2;

					temp_r = (1.0-p)*(1.0-q)*ptr1[y1][x1].r + p*(1.0-q)*ptr1[y1][x2].r
						+ (1.0-p)*q*ptr1[y2][x1].r + p*q*ptr1[y2][x2].r;
					temp_g = (1.0-p)*(1.0-q)*ptr1[y1][x1].g + p*(1.0-q)*ptr1[y1][x2].g
						+ (1.0-p)*q*ptr1[y2][x1].g + p*q*ptr1[y2][x2].g;
					temp_b = (1.0-p)*(1.0-q)*ptr1[y1][x1].b + p*(1.0-q)*ptr1[y1][x2].b
						+ (1.0-p)*q*ptr1[y2][x1].b + p*q*ptr1[y2][x2].b;
				}

				ptr2[j+h/2][i+w/2].r = (BYTE)temp_r;
				ptr2[j+h/2][i+w/2].g = (BYTE)temp_g;
				ptr2[j+h/2][i+w/2].b = (BYTE)temp_b;
			}
		}
	}

	cvReleaseMat(&matHinv);
}

void CWarping::PerspectiveTransform2(IplImage* src, IplImage* dst, CvMat* matH)
{
	CvMat* matHinv = cvCreateMat( 3, 3, matH->type );
	cvInvert(matH, matHinv, CV_SVD);

	int w = dst->width;
	int h = dst->height;

	int x1, y1, x2, y2;
	double normpos;
	double rx, ry, p, q;
	double temp_r, temp_g, temp_b;

	int j, i;
	if( dst->nChannels==1 )
	{
		BwImage ptr1(src);
		BwImage ptr2(dst);

		for(j=0; j<h; j++)
		{
			for(i=0; i<w; i++)
			{
				normpos = cvmGet(matHinv,2,0)*i + cvmGet(matHinv,2,1)*j + cvmGet(matHinv,2,2);
				rx = (cvmGet(matHinv,0,0)*i + cvmGet(matHinv,0,1)*j + cvmGet(matHinv,0,2)) / normpos;
				ry = (cvmGet(matHinv,1,0)*i + cvmGet(matHinv,1,1)*j + cvmGet(matHinv,1,2)) / normpos;

				x1 = (int)rx;
				y1 = (int)ry;

				x2 = x1 + 1;
				y2 = y1 + 1;

				p = rx - x1;
				q = ry - y1;

				if( x1<0 || x1>=w || x2<0 || x2>=w || y1<0 || y1>=h || y2<0 || y2>=h )
				{
					temp_r = 0;
				}
				else
				{
					temp_r = (1.0-p)*(1.0-q)*ptr1[y1][x1] + p*(1.0-q)*ptr1[y1][x2]
														+ (1.0-p)*q*ptr1[y2][x1] + p*q*ptr1[y2][x2];
				}

				ptr2[j][i] = (BYTE)temp_r;
			}
		}
	}
	else if( dst->nChannels==3 )
	{
		RgbImage ptr1(src);
		RgbImage ptr2(dst);

		for(j=0; j<h; j++)
		{
			for(i=0; i<w; i++)
			{
				normpos = cvmGet(matHinv,2,0)*i + cvmGet(matHinv,2,1)*j + cvmGet(matHinv,2,2);
				rx = (cvmGet(matHinv,0,0)*i + cvmGet(matHinv,0,1)*j + cvmGet(matHinv,0,2)) / normpos;
				ry = (cvmGet(matHinv,1,0)*i + cvmGet(matHinv,1,1)*j + cvmGet(matHinv,1,2)) / normpos;

				x1 = (int)rx;
				y1 = (int)ry;

				x2 = x1 + 1;
				y2 = y1 + 1;

				p = rx - x1;
				q = ry - y1;

				if( x1<0 || x1>=w || x2<0 || x2>=w || y1<0 || y1>=h || y2<0 || y2>=h )
				{
					temp_r = temp_g = temp_b = 0;
				}
				else
				{
					temp_r = (1.0-p)*(1.0-q)*ptr1[y1][x1].r + p*(1.0-q)*ptr1[y1][x2].r
						+ (1.0-p)*q*ptr1[y2][x1].r + p*q*ptr1[y2][x2].r;
					temp_g = (1.0-p)*(1.0-q)*ptr1[y1][x1].g + p*(1.0-q)*ptr1[y1][x2].g
						+ (1.0-p)*q*ptr1[y2][x1].g + p*q*ptr1[y2][x2].g;
					temp_b = (1.0-p)*(1.0-q)*ptr1[y1][x1].b + p*(1.0-q)*ptr1[y1][x2].b
						+ (1.0-p)*q*ptr1[y2][x1].b + p*q*ptr1[y2][x2].b;
				}

				ptr2[j][i].r = (BYTE)temp_r;
				ptr2[j][i].g = (BYTE)temp_g;
				ptr2[j][i].b = (BYTE)temp_b;
			}
		}
	}
	cvReleaseMat(&matHinv);
}

// 요건 bicubic interpolation으로 한거..
/*
void CWarping::PerspectiveTransform(IplImage* src, CvMat* matH, IplImage* dst)
{
	CvMat* matHinv = cvCreateMat( 3, 3, CV_64FC1 );
	cvInvert(matH, matHinv, CV_SVD);

	int w = dst->width;
	int h = dst->height;

	double normpos;
	int x1, x2, x3, x4, y1, y2, y3, y4;
	double v1, v2, v3, v4;
	double rx, ry, p, q;
	double temp_r, temp_g, temp_b;

	int j, i;
	if( dst->nChannels==1 )
	{
		BwImage ptr1(src);
		BwImage ptr2(dst);

		for(j=-h/2; j<h/2; j++)
		{
			for(i=-w/2; i<w/2; i++)
			{
				normpos = cvmGet(matHinv,2,0)*i + cvmGet(matHinv,2,1)*j + cvmGet(matHinv,2,2);
				rx = (cvmGet(matHinv,0,0)*i + cvmGet(matHinv,0,1)*j + cvmGet(matHinv,0,2)) / normpos;
				ry = (cvmGet(matHinv,1,0)*i + cvmGet(matHinv,1,1)*j + cvmGet(matHinv,1,2)) / normpos;

				if( rx>0 )
					x2 = (int)rx;
				else
					x2 = (int)rx-1;
				x1 = x2 - 1;
				x3 = x2 + 1;
				x4 = x2 + 2;
				p  = rx - x2;

				if( ry>0 )
					y2 = (int)ry;
				else
					y2 = (int)ry-1;
				y1 = y2 - 1;
				y3 = y2 + 1;
				y4 = y2 + 2;
				q  = ry - y2;

				if( x1<-w/2 || x1>=w/2 || x2<-w/2 || x2>=w/2 || x3<-w/2 || x3>=w/2 || x4<-w/2 || x4>=w/2 ||
					y1<-h/2 || y1>=h/2 || y2<-h/2 || y2>=h/2 || y3<-h/2 || y3>=h/2 || y4<-h/2 || y4>=h/2 )
				{
					temp_r = 0;
				}
				else
				{
					x1 += w/2;	x2 += w/2;	x3 += w/2;	x4 += w/2;
					y1 += h/2;	y2 += h/2;	y3 += h/2;	y4 += h/2;

					v1 = cubic_interpolation(ptr1[y1][x1], ptr1[y1][x2], ptr1[y1][x3], ptr1[y1][x4], p);
					v2 = cubic_interpolation(ptr1[y2][x1], ptr1[y2][x2], ptr1[y2][x3], ptr1[y2][x4], p);
					v3 = cubic_interpolation(ptr1[y3][x1], ptr1[y3][x2], ptr1[y3][x3], ptr1[y3][x4], p);
					v4 = cubic_interpolation(ptr1[y4][x1], ptr1[y4][x2], ptr1[y4][x3], ptr1[y4][x4], p);

					temp_r  = cubic_interpolation(v1, v2, v3, v4, q);
				}

				ptr2[j+h/2][i+w/2] = (BYTE)temp_r;
			}
		}
	}
	else if( dst->nChannels==3 )
	{
		RgbImage ptr1(src);
		RgbImage ptr2(dst);

		for(j=-h/2; j<h/2; j++)
		{
			for(i=-w/2; i<w/2; i++)
			{
				normpos = cvmGet(matHinv,2,0)*i + cvmGet(matHinv,2,1)*j + cvmGet(matHinv,2,2);
				rx = (cvmGet(matHinv,0,0)*i + cvmGet(matHinv,0,1)*j + cvmGet(matHinv,0,2)) / normpos;
				ry = (cvmGet(matHinv,1,0)*i + cvmGet(matHinv,1,1)*j + cvmGet(matHinv,1,2)) / normpos;

				if( rx>0 )
					x2 = (int)rx;
				else
					x2 = (int)rx-1;
				x1 = x2 - 1;
				x3 = x2 + 1;
				x4 = x2 + 2;
				p  = rx - x2;

				if( ry>0 )
					y2 = (int)ry;
				else
					y2 = (int)ry-1;
				y1 = y2 - 1;
				y3 = y2 + 1;
				y4 = y2 + 2;
				q  = ry - y2;

				if( x1<-w/2 || x1>=w/2 || x2<-w/2 || x2>=w/2 || x3<-w/2 || x3>=w/2 || x4<-w/2 || x4>=w/2 ||
					y1<-h/2 || y1>=h/2 || y2<-h/2 || y2>=h/2 || y3<-h/2 || y3>=h/2 || y4<-h/2 || y4>=h/2 )
				{
					temp_r = temp_g = temp_b = 0;
				}
				else
				{
					x1 += w/2;	x2 += w/2;	x3 += w/2;	x4 += w/2;
					y1 += h/2;	y2 += h/2;	y3 += h/2;	y4 += h/2;

					v1 = cubic_interpolation(ptr1[y1][x1].r, ptr1[y1][x2].r, ptr1[y1][x3].r, ptr1[y1][x4].r, p);
					v2 = cubic_interpolation(ptr1[y2][x1].r, ptr1[y2][x2].r, ptr1[y2][x3].r, ptr1[y2][x4].r, p);
					v3 = cubic_interpolation(ptr1[y3][x1].r, ptr1[y3][x2].r, ptr1[y3][x3].r, ptr1[y3][x4].r, p);
					v4 = cubic_interpolation(ptr1[y4][x1].r, ptr1[y4][x2].r, ptr1[y4][x3].r, ptr1[y4][x4].r, p);
					temp_r  = cubic_interpolation(v1, v2, v3, v4, q);

					v1 = cubic_interpolation(ptr1[y1][x1].g, ptr1[y1][x2].g, ptr1[y1][x3].g, ptr1[y1][x4].g, p);
					v2 = cubic_interpolation(ptr1[y2][x1].g, ptr1[y2][x2].g, ptr1[y2][x3].g, ptr1[y2][x4].g, p);
					v3 = cubic_interpolation(ptr1[y3][x1].g, ptr1[y3][x2].g, ptr1[y3][x3].g, ptr1[y3][x4].g, p);
					v4 = cubic_interpolation(ptr1[y4][x1].g, ptr1[y4][x2].g, ptr1[y4][x3].g, ptr1[y4][x4].g, p);
					temp_g  = cubic_interpolation(v1, v2, v3, v4, q);

					v1 = cubic_interpolation(ptr1[y1][x1].b, ptr1[y1][x2].b, ptr1[y1][x3].b, ptr1[y1][x4].b, p);
					v2 = cubic_interpolation(ptr1[y2][x1].b, ptr1[y2][x2].b, ptr1[y2][x3].b, ptr1[y2][x4].b, p);
					v3 = cubic_interpolation(ptr1[y3][x1].b, ptr1[y3][x2].b, ptr1[y3][x3].b, ptr1[y3][x4].b, p);
					v4 = cubic_interpolation(ptr1[y4][x1].b, ptr1[y4][x2].b, ptr1[y4][x3].b, ptr1[y4][x4].b, p);
					temp_b  = cubic_interpolation(v1, v2, v3, v4, q);
				}

				ptr2[j+h/2][i+w/2].r = (BYTE)temp_r;
				ptr2[j+h/2][i+w/2].g = (BYTE)temp_g;
				ptr2[j+h/2][i+w/2].b = (BYTE)temp_b;
			}
		}
	}

	cvReleaseMat(&matHinv);
}
*/

void CWarping::MakeTransformMatrix(double* theta, double* scale, double* tr, CvMat* dstH)
{
	double R1[4][4] = {0,};
	double R2[4][4] = {0,};
	double R3[4][4] = {0,};
	double S[4][4] = {0,};
	double P[3][4] = {0,};

	R1[3][3] = R2[3][3] = R3[3][3] = S[3][3] = 1.0;

	// Make a rotation matrix (performed by x-axis)
	R1[0][0] = 1.0;
	R1[1][1] = cos( theta[0] );
	R1[1][2] = sin( theta[0] );
	R1[2][1] = -sin( theta[0] );
	R1[2][2] = cos( theta[0] );
	CvMat matR1 = cvMat( 4, 4, CV_64FC1, R1 );

	// Make a rotation matrix (performed by y-axis)
	R2[0][0] = cos( theta[1] );
	R2[0][2] = -sin( theta[1] );
	R2[1][1] = 1.0;
	R2[2][0] = sin( theta[1] );
	R2[2][2] = cos( theta[1] );
	CvMat matR2 = cvMat( 4, 4, CV_64FC1, R2 );

	// Make a rotation matrix (performed by z-axis)
	R3[0][0] = cos( theta[2] );
	R3[0][1] = sin( theta[2] );
	R3[1][0] = -sin( theta[2] );
	R3[1][1] = cos( theta[2] );
	R3[2][2] = 1.0;
	CvMat matR3 = cvMat( 4, 4, CV_64FC1, R3 );

	// Make a scaling matrix
	S[0][0] = scale[0];
	S[1][1] = scale[1];
	S[2][2] = scale[2];
	S[0][3] = tr[0];
	S[1][3] = tr[1];
	S[2][3] = tr[2];
	CvMat matS = cvMat( 4, 4, CV_64FC1, S );

	// Make a projection matrix
	P[0][0] = P[1][1] = P[2][3] = 1.0;
	CvMat matP = cvMat( 3, 4, CV_64FC1, P );

	// Matrix multiplication
	CvMat* matTmp1 = cvCreateMat( 4, 4, CV_64FC1 );
	CvMat* matTmp2 = cvCreateMat( 4, 4, CV_64FC1 );
	CvMat* matTmp3 = cvCreateMat( 4, 4, CV_64FC1 );
	CvMat* matTmp4 = cvCreateMat( 3, 4, CV_64FC1 );

	cvMatMul(&matR3, &matR2, matTmp1);
	cvMatMul(matTmp1, &matR1, matTmp2);
	cvMatMul(&matS, matTmp2, matTmp3);
	cvMatMul(&matP, matTmp3, matTmp4);

	// Assign matrix elements to the destination matrix..
	double mat_elem = 0;
	int j, i, m;
	for(j=0; j<3; j++)
	{
		for(i=0; i<3; i++)
		{
			if( i==2 )	m=i+1;
			else		m=i;

			mat_elem = cvmGet(matTmp4, j, m);
			cvmSet(dstH, j, i, mat_elem);
		}
	}

	cvReleaseMat(&matTmp1);
	cvReleaseMat(&matTmp2);
	cvReleaseMat(&matTmp3);
	cvReleaseMat(&matTmp4);
}

int CWarping::FindArea(IplImage* img)
{
	int res = 0;

	int w = img->width;
	int h = img->height;

	BwImage ptr(img);

	int j, i;
	for(j=0; j<h; j++)
	for(i=0; i<w; i++)
	{
		if( ptr[j][i]!=0 )
			res++;
	}

	return res;
}
