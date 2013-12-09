#include "FaceAligner.h"

FaceAligner::FaceAligner() {
	//-- 0. Initialize parameters
	//-- 1. Load the cascades
}

FaceAligner::~FaceAligner() {

}

/** @functions */
void FaceAligner::align_faces(IplImage *fimg, CvRect& face, const CvRect& refface) {
	int i, j;
	CVec2d srcpts[4];
	CVec2d dstpts[4];

	srcpts[0].x = face.x;
	srcpts[0].y = face.y;
	dstpts[0].x = refface.x;
	dstpts[0].y = refface.y;

	srcpts[1].x = face.x + face.width;
	srcpts[1].y = face.y;
	dstpts[1].x = refface.x + refface.width;
	dstpts[1].y = refface.y;

	srcpts[2].x = face.x + face.width;
	srcpts[2].y = face.y + face.height;
	dstpts[2].x = refface.x + refface.width;
	dstpts[2].y = refface.y + refface.height;

	srcpts[3].x = face.x;
	srcpts[3].y = face.y + face.height;
	dstpts[3].x = refface.x;
	dstpts[3].y = refface.y + refface.height;

	double** Hmat = new double*[3];
	double** Hinv = new double*[3];
	for(i=0; i<3; i++) {
		Hmat[i] = new double[3];
		Hinv[i] = new double[3];
	}

	CHomography homography;
	homography.compute(srcpts, dstpts, 4);

	for(j=0; j<3; j++) {
		for(i=0; i<3; i++) {
			Hmat[j][i] = homography.m_dData[j][i];
		}
	}
	homography.computeInverse(Hmat, Hinv);

	ARgbImage pfimg(fimg);
	IplImage *fcpy = cvCreateImage( cvSize(fimg->width,fimg->height), IPL_DEPTH_64F, 3 );
	RgbImageDbl pfcpy(fcpy);
	for(j=0; j<fimg->height; j++) {
		for(i=0; i<fimg->width; i++) {
			pfcpy[j][i].r = (double)pfimg[j][i].r;
			pfcpy[j][i].g = (double)pfimg[j][i].g;
			pfcpy[j][i].b = (double)pfimg[j][i].b;
		}
	}

	IplImage *dst = cvCreateImage( cvSize(fimg->width,fimg->height), IPL_DEPTH_64F, 3 );
	RgbImageDbl pdst(dst);

	warp_img(fcpy, dst, Hinv);
//	CvMat matA = cvMat(3,3,CV_64FC1,homography.m_dData);
//	CWarping::PerspectiveTransform2(fcpy, dst, &matA);
//	cvPerspectiveTransform(fcpy,dst,&matA);

	for(j=0; j<fimg->height; j++) {
		for(i=0; i<fimg->width; i++) {
			pfimg[j][i].r = (BYTE)pdst[j][i].r;
			pfimg[j][i].g = (BYTE)pdst[j][i].g;
			pfimg[j][i].b = (BYTE)pdst[j][i].b;
			pfimg[j][i].a = 255;
		}
	}

	for(i=0; i<3; i++)
	{
		delete [] Hmat[i];
		delete [] Hinv[i];
	}
	delete [] Hmat;
	delete [] Hinv;

	cvReleaseImage(&fcpy);
	cvReleaseImage(&dst);
}

void FaceAligner::warp_img(IplImage *src, IplImage *dst, double **Hinv)
{
	int w = dst->width;
	int h = dst->height;

	int x1, y1, x2, y2;
	double normpos;
	double rx, ry, p, q;
	double temp_r, temp_g, temp_b;

	int j, i;
	if( dst->nChannels==1 ) {
		BwImageDbl ptr1(src);
		BwImageDbl ptr2(dst);

		for(j=0; j<h; j++)
		{
			for(i=0; i<w; i++)
			{
				normpos = Hinv[2][0]*i + Hinv[2][1]*j + Hinv[2][2];

				rx = (Hinv[0][0]*i + Hinv[0][1]*j + Hinv[0][2]) / normpos;
				ry = (Hinv[1][0]*i + Hinv[1][1]*j + Hinv[1][2]) / normpos;

				x1 = (int)rx;
				y1 = (int)ry;

				x2 = x1 + 1; if( x2 == w ) x2 = w - 1;
				y2 = y1 + 1; if( y2 == h ) y2 = h - 1;

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
		RgbImageDbl ptr1(src);
		RgbImageDbl ptr2(dst);

		for(j=0; j<h; j++)
		{
			for(i=0; i<w; i++)
			{
				normpos = Hinv[2][0]*i + Hinv[2][1]*j + Hinv[2][2];
				rx = (Hinv[0][0]*i + Hinv[0][1]*j + Hinv[0][2]) / normpos;
				ry = (Hinv[1][0]*i + Hinv[1][1]*j + Hinv[1][2]) / normpos;

				x1 = (int)rx;
				y1 = (int)ry;

				x2 = x1 + 1; if( x2 == w ) x2 = w - 1;
				y2 = y1 + 1; if( y2 == h ) y2 = h - 1;

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

}
