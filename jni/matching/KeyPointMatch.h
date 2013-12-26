#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include "kpmtypes.h"

#include "AnnMatch.h"
#include "Homography.h"
#include "HomographyEst.h"

#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define	OBJECTNUM	2

using namespace std;

class CKeyPointMatch
{
public:
	CKeyPointMatch(void);
	~CKeyPointMatch(void);

public:
	IplImage* m_img1[OBJECTNUM];
	IplImage* m_img2;
	double** m_fltimg;				// normalized image..


	// Keypoint extraction results from SURF..
	vector<CvSURFPoint> m_surfp1[OBJECTNUM];
	vector<CvSURFPoint> m_surfp2;
	// For SURF fast matching..(laplacian parameters implementation)
	vector<CvSURFPoint> m_surfp1_minus[OBJECTNUM];
	vector<CvSURFPoint> m_surfp2_minus;


	CAnnMatch m_Ann[OBJECTNUM];

	std::vector<corsp2i> m_CorspMap[OBJECTNUM];
	std::vector<corsp2i> m_InlierMap[OBJECTNUM];
	// temporal use..
	std::vector<corsp2i> m_prevInlierMap[OBJECTNUM];

//	CvMat* m_Homography[OBJECTNUM];
	CHomography m_Homography[OBJECTNUM];

	point2i m_box[OBJECTNUM][4];
	point2i m_prevbox[OBJECTNUM][4];

	int m_prevnum[OBJECTNUM];


public:
	void Init();
	bool IsValid() { return (m_Ann[0].m_kdTree != NULL); }

    inline bool PtInRect(CvPoint2D32f pt, CvRect rect);

 	void SetReferImageSURF(CvPoint* pts, IplImage* img, int nObj);
	void SetQueryImageSURF(IplImage* img, int nObj);

	// SURF
	void SaveFVsFile(const char* filename, vector<SURFfV>& features);
	void LoadFVsFile(const char* filename, vector<SURFfV>& features);
	void SaveKPsFile(const char* filename, vector<CvSURFPoint>& kps, point2i* bdpts);
	void LoadKPsFile(const char* filename, vector<CvSURFPoint>& kps, point2i* bdpts);

	void DrawOutput(IplImage* img, point2i* box, int numObj);

	bool RemoveOutlier(CvMat* preH, int numObj);
};

inline bool CKeyPointMatch::PtInRect(CvPoint2D32f pt, CvRect rect) {
    return (pt.x>=rect.x && pt.x<=rect.x+rect.width &&
     pt.y>=rect.y && pt.y<=rect.y+rect.height) ? true : false;
}


