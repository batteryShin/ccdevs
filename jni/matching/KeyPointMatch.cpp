#include "KeyPointMatch.h"

#define	THRESH_CORNER_HIGH	400
#define	THRESH_CORNER_LOW	50
#define THRESH_CORNER_RATE	0.9
#define THRESH_CORNER_RELOCATION	2.0
#define THRESH_BOX			30000

#define REGIST_FILENAME				"reg_model_SURF"
#define REGIST_FILENAME2			"reg_model"

#define TRACKINGKALMAN

#define	IND_CORNERNUM		100

int corner_type = 10;
int corner_barrier = 40;

CvMemStorage* storage;

using namespace std;

CKeyPointMatch::CKeyPointMatch(void)
{
/*
 	for(int i=0; i<OBJECTNUM; i++)
 		m_Homography[i] = NULL;
*/
	storage = cvCreateMemStorage(0);

	Init();
}

CKeyPointMatch::~CKeyPointMatch(void)
{
//	for(int i=0; i<OBJECTNUM; i++)
//		if( m_Homography[i] )	cvReleaseMat(&m_Homography[i]);

	if( m_fltimg )
	{
		for(int i=0; i<m_img1[0]->height; i++)
			delete [] m_fltimg[i];
		delete [] m_fltimg;
	}
}

void CKeyPointMatch::Init()
{
	register int n, i;

    if( m_img2 ) {
    	cvReleaseImage(&m_img2);
    }

	m_fltimg = NULL;

	for(n=0; n<OBJECTNUM; n++)
	{
        if( m_img1[n] ) {
            cvReleaseImage(&m_img1[n]);
        }

		m_CorspMap[n].clear();
		m_InlierMap[n].clear();

		m_Ann[n].Clear();

		for(i=0; i<4; i++)
		{
			m_box[n][i].x = -1;
			m_box[n][i].y = -1;
			m_prevbox[n][i].x = -1;
			m_prevbox[n][i].y = -1;
		}
/*
		if( m_Homography[n] )
			cvReleaseMat(&m_Homography[n]);
		m_Homography[n] = cvCreateMat(3,3,CV_32FC1);
*/
		m_prevnum[n] = 0;
	}
}

// SURF matching 
void CKeyPointMatch::SetReferImageSURF(CvPoint* pts, IplImage* img, int nObj)
{
	IplImage* tmpimg = cvCreateImage(cvGetSize(img), 8, 1);

    if( img->nChannels == tmpimg->nChannels ) {
        cvCopy(img,tmpimg);
    } else {
        cvCvtColor(img,tmpimg,CV_BGR2GRAY);
    }

	CvSeq *kps_src, *desc_src;
	CvSURFParams surfparams = cvSURFParams(2800,0);
	cvExtractSURF( tmpimg, 0, &kps_src, &desc_src, storage, surfparams );


	// Local descriptor extraction
	CvSeqReader reader, kreader;
	cvStartReadSeq( kps_src, &kreader );
	cvStartReadSeq( desc_src, &reader );

	vector<SURFfV> features1;
//	vector<SURFfV> features2;
	SURFfV fv;

    CvMat* contour = cvCreateMat(4,2,CV_8UC1);
    cvmSet(contour,0,0,pts[0].x);
    cvmSet(contour,0,1,pts[0].y);
    cvmSet(contour,1,0,pts[1].x);
    cvmSet(contour,1,1,pts[1].y);
    cvmSet(contour,2,0,pts[2].x);
    cvmSet(contour,2,1,pts[2].y);
    cvmSet(contour,3,0,pts[3].x);
    cvmSet(contour,3,1,pts[3].y);

	// 이전 matching결과 clear..
	m_surfp1[nObj].clear();

	register int i, j;
	for( i = 0; i < desc_src->total; i++ )
	{
		CvSURFPoint* kp = (CvSURFPoint*)kreader.ptr;
		float* descriptor = (float*)reader.ptr;
		CV_NEXT_SEQ_ELEM( kreader.seq->elem_size, kreader );
		CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );

//        if( rgn.PtInRegion(CPoint((int)kp->pt.x,(int)kp->pt.y)) )
		if( cvPointPolygonTest(contour, cvPoint2D32f(kp->pt.x,kp->pt.y),true)>0 )
		{
			for(j=0; j<SURF_FEATURE_DIM; j++)
				fv.v[j] = (double)(descriptor[j]);

	//		if( kp[i].laplacian > 0 )
	//		{
				features1.push_back(fv);
				m_surfp1[nObj].push_back(*kp);
	//		}
	//		else
	//		{
	//			features2.push_back(fv);
	//			m_surfp1_minus.push_back(*kp);
	//		}
		}
		else
			continue;
	}

	point2i bdpts[4];
	for(int i=0; i<4; i++)
	{
		bdpts[i].x = pts[i].x;
		bdpts[i].y = pts[i].y;
	}

	SaveKPsFile(REGIST_FILENAME, m_surfp1[nObj], bdpts);
	SaveFVsFile(REGIST_FILENAME, features1);

	//-------------------------------------------------------------------------
	// ANN Construction
	//-------------------------------------------------------------------------
	m_Ann[nObj].Construct(features1);

	// Release memory..
	cvReleaseImage(&tmpimg);
	cvReleaseMat(&contour);
}

void CKeyPointMatch::SetQueryImageSURF(IplImage* img, int nObj)
{
	IplImage* tmpimg;
	tmpimg = cvCreateImage(cvGetSize(img), 8, 1);

    if( img->nChannels == tmpimg->nChannels ) {
        cvCopy(img,tmpimg);
    } else {
        cvCvtColor(img,tmpimg,CV_BGR2GRAY);
    }

	CvSeq *kps_dst, *desc_dst;
	CvSURFParams surfparams = cvSURFParams(2800,0);
	cvExtractSURF( tmpimg, 0, &kps_dst, &desc_dst, storage, surfparams );

	// Local descriptor extraction
	CvSeqReader reader, kreader;
	cvStartReadSeq( kps_dst, &kreader );
	cvStartReadSeq( desc_dst, &reader );

	vector<SURFfV> features1;
//	vector<SURFfV> features2;
	SURFfV fv;

	// 이전 matching결과 clear..
	m_surfp2.clear();

	register int i, j, k;
	for( i = 0; i < desc_dst->total; i++ )
	{
		const CvSURFPoint* kp = (const CvSURFPoint*)kreader.ptr;
		const float* descriptor = (const float*)reader.ptr;
		CV_NEXT_SEQ_ELEM( kreader.seq->elem_size, kreader );
		CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );

		for(j=0; j<SURF_FEATURE_DIM; j++)
			fv.v[j] = (double)(descriptor[j]);

//		if( kp[i].laplacian > 0 )
//		{
			features1.push_back(fv);
			m_surfp2.push_back(*kp);
//		}
//		else
//		{
//			features2.push_back(fv);
//			m_surfp2_minus.push_back(*kp);
//		}
	}

	m_Ann[nObj].Match(features1);

	vector<int> match = m_Ann[nObj].GetMatch();
	vector<int>::iterator iter;

	m_CorspMap[nObj].clear();
	for( k=0,iter=match.begin(); iter!=match.end(); k++,++iter )
	{
		if( *iter == -1 ) continue;

		m_CorspMap[nObj].push_back( corsp2i((int)(m_surfp1[nObj].at(*iter).pt.x+0.5),
											(int)(m_surfp1[nObj].at(*iter).pt.y+0.5),
											(int)(m_surfp2.at(k).pt.x+0.5),
											(int)(m_surfp2.at(k).pt.y+0.5)				)	);
	}

	// Release memory..
	cvReleaseImage(&tmpimg);
	cvClearMemStorage(storage);
}

void CKeyPointMatch::SaveKPsFile(const char* filename, vector<CvSURFPoint>& kps, point2i* bdpts)
{
    stringstream ss;
    ss << filename << ".rkp";
	FILE* file = fopen(ss.str().c_str(), "w");

	int nkps = (int)kps.size();

	// Registration <the number of points> & <dimension of feature vector> & <boundary>..
	fprintf(file, "%d", nkps);

	vector<CvSURFPoint>::iterator iter;
	for(iter=kps.begin(); iter!=kps.end(); ++iter)
		fprintf(file, "\n%f\t%f", iter->pt.x, iter->pt.y);

	fprintf(file, "\n%d\t%d", bdpts[0].x, bdpts[0].y); 
	for(int i=1; i<4; i++)
		fprintf(file, "\t%d\t%d", bdpts[i].x, bdpts[i].y);	

	fclose(file);
}

void CKeyPointMatch::SaveFVsFile(const char* filename, vector<SURFfV>& features)
{
    stringstream ss;
    ss << filename << ".rfv";
	FILE* file = fopen(ss.str().c_str(), "w");
	int npt = (int)features.size();

	// Registration <the number of points> & <dimension of feature vector>..
	fprintf(file, "%d\t%d", npt, SURF_FEATURE_DIM);

	int i;
	vector<SURFfV>::iterator iter;
	for(iter=features.begin(); iter!=features.end(); ++iter)
	{
		fprintf(file, "\n%lf", iter->v[0]);
		for(i=1; i<SURF_FEATURE_DIM; i++)
			fprintf(file, "\t%lf", iter->v[i]);
	}

	fclose(file);
}

void CKeyPointMatch::LoadFVsFile(const char* filename, vector<SURFfV>& features)
{
    stringstream ss;
    ss << filename << ".rfv";
	FILE* file = fopen(ss.str().c_str(), "r");

	// Load <the number of points> & <dimension of feature vector>..
	int npt = 0;
	int dimfv = 0;

	fscanf(file, "%d\t%d", &npt, &dimfv);

	vector<SURFfV>::iterator iter;
	SURFfV fv;
	int j, i;
	for(j=0; j<npt; j++)
	{
		fscanf(file, "\n%lf", &(fv.v[0]));
		for(i=1; i<dimfv; i++)
			fscanf(file, "\t%lf", &(fv.v[i]));

		features.push_back(fv);
	}

	fclose(file);
}

void CKeyPointMatch::LoadKPsFile(const char* filename, vector<CvSURFPoint>& kps, point2i* pts)
{
    stringstream ss;
    ss << filename << ".rkp";
	FILE* file = fopen(ss.str().c_str(), "r");

	// Load <the number of points> x 4scales ..
	int npt;
	fscanf(file, "%d", &npt);

	int j;
	vector<CvSURFPoint>::iterator iter;
	CvSURFPoint surfpt;
	for(j=0; j<npt; j++)
	{
		fscanf(file, "\n%f\t%f", &(surfpt.pt.x), &(surfpt.pt.y));

		kps.push_back(surfpt);
	}

	fscanf(file, "\n%d\t%d", &(pts[0].x), &(pts[0].y) );
	for(j=1; j<4; j++)
		fscanf(file, "\t%d\t%d", &(pts[j].x), &(pts[j].y) );

	fclose(file);
}

bool CKeyPointMatch::RemoveOutlier(CvMat* preH, int numObj)
{
	CvMat *srcMat, *dstMat;

	if( m_CorspMap[numObj].size() > 4 )
	{
		CvMat* Hmat = cvCreateMat(3, 3, CV_32FC1);

		m_prevInlierMap[numObj] = m_InlierMap[numObj];

		m_InlierMap[numObj].clear();
		RansacHomograhyEstimation(m_CorspMap[numObj], m_InlierMap[numObj], Hmat);

		if( m_InlierMap[numObj].size() >= 4 )
		{
			// opencv를 이용하는 Homography 계산으로 수정 by 동철
			srcMat = cvCreateMat(3, (int)m_InlierMap[numObj].size(), CV_32FC1);
			dstMat = cvCreateMat(3, (int)m_InlierMap[numObj].size(), CV_32FC1);

			int i;	
			vector<corsp2i>::iterator iter;
			for(iter=m_InlierMap[numObj].begin(),i=0; iter!=m_InlierMap[numObj].end(); iter++,i++)
			{
				cvmSet(srcMat, 0, i, (float)(*iter).x2 );
				cvmSet(srcMat, 1, i, (float)(*iter).y2 );
				cvmSet(srcMat, 2, i, 1.f );
				cvmSet(dstMat, 0, i, (float)(*iter).x1 );
				cvmSet(dstMat, 1, i, (float)(*iter).y1 );
				cvmSet(dstMat, 2, i, 1.f );
			}

			cvFindHomography( srcMat, dstMat, Hmat, CV_RANSAC, 5.99 );

			cvReleaseMat(&srcMat);
			cvReleaseMat(&dstMat);
		}
		else
			return false;

		// rectify model과 결합
		cvMatMul(Hmat, preH, Hmat);

		for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			m_Homography[numObj].m_dData[i][j] = cvmGet(Hmat, i, j);
		cvReleaseMat(&Hmat);

		return true;
	}

	return false;
}

void CKeyPointMatch::DrawOutput(IplImage* img, point2i* box, int numObj)
{
	CvScalar red = cvScalar(255,0,0);
	CvScalar green = cvScalar(0,255,0);
	CvScalar white = cvScalar(255,255,255);
	CvScalar yellow = cvScalar(255,255,0);
	CvScalar cyan = cvScalar(0,255,255);
	CvScalar *color;
	if( numObj%5==0 )
		color = &red;
	else if( numObj%5==1 )
		color = &green;
	else if( numObj%5==2 )
		color = &white;
	else if( numObj%5==3 )
		color = &yellow;
	else
		color = &cyan;

	int w = img->width;
	int h = img->height;
	int mx = w/2;
	int my = h/2;

// 	ippDrawLine(img, box[0].x+mx, box[0].y+my, box[1].x+mx, box[1].y+my, *color);
// 	ippDrawLine(img, box[1].x+mx, box[1].y+my, box[2].x+mx, box[2].y+my, *color);
// 	ippDrawLine(img, box[2].x+mx, box[2].y+my, box[3].x+mx, box[3].y+my, *color);
// 	ippDrawLine(img, box[3].x+mx, box[3].y+my, box[0].x+mx, box[0].y+my, *color);
	cvLine(img, cvPoint(box[0].x,box[0].y), cvPoint(box[1].x,box[1].y), *color);
	cvLine(img, cvPoint(box[1].x,box[1].y), cvPoint(box[2].x,box[2].y), *color);
	cvLine(img, cvPoint(box[2].x,box[2].y), cvPoint(box[3].x,box[3].y), *color);
	cvLine(img, cvPoint(box[3].x,box[3].y), cvPoint(box[0].x,box[0].y), *color);
}

