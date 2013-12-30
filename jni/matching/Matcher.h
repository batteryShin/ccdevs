#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include "KeyPointMatch.h"
#include "Converter.h"

using namespace std;


class Matcher{
	CKeyPointMatch* m_surf;

	CvPoint m_pts[OBJECTNUM][4];
	float m_fpts[OBJECTNUM][8];

    CvMat* m_preH;
	CvRect m_rect;

	int m_curObj;			// current object
	int m_numObj;			// number of objects
	int m_ptnum;


//	double m_PlanarPose[16];

    //// Operations ////
	// From four corner points, find the modelview matrix of OpenGL
//	void FindGLModelView(point2i* srcPts, CHomography* srcH, point2i* box, CHomography* dstH);

//	void OpenGLInit(int w, int h);

//	void DrawFeaturePoints();
//	void DrawEpipolarLines();
//	void LineEqDraw(CDC* pDc, std::vector<CVec3d>& lineEqs);



public:
    Matcher(IplImage *pImg, float* rgn);
    ~Matcher();
    float* match(IplImage* pImg);
    void FindModelView(point2i* srcPts, CHomography* srcH, point2i* box, CHomography* dstH);
    float* getSrcPts();
};

