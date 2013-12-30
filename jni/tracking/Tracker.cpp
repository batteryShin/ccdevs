#include "Tracker.h"

using namespace std;
static int nid = 0;

Tracker::Tracker(IplImage * pImg, CvRect pFaceRect){
    // File-level variables
    int   nHistBins = 30;                 // number of histogram bins
    float rangesArr[] = {0,180};          // histogram range
    vmin = 10;
    vmax = 256;
    smin = 55;
    float * pRanges = rangesArr;

    pHSVImg  = cvCreateImage( cvGetSize(pImg), 8, 3 );
    pHueImg  = cvCreateImage( cvGetSize(pImg), 8, 1 );
    pMask    = cvCreateImage( cvGetSize(pImg), 8, 1 );
    pProbImg = cvCreateImage( cvGetSize(pImg), 8, 1 );

    pHist = cvCreateHist( 1, &nHistBins, CV_HIST_ARRAY, &pRanges, 1 );

    float maxVal = 0.f;


    // Create a new hue image
    updateHueImage(pImg);


    // Create a histogram representation for the face
    cvSetImageROI( pHueImg, pFaceRect );
    cvSetImageROI( pMask,   pFaceRect );
    cvCalcHist( &pHueImg, pHist, 0, pMask );
    cvGetMinMaxHistValue( pHist, 0, &maxVal, 0, 0 );
    cvConvertScale( pHist->bins, pHist->bins, maxVal? 255.0/maxVal : 0, 0 );
    cvResetImageROI( pHueImg );
    cvResetImageROI( pMask );


    // Store the previous face location
    prevFaceRect = pFaceRect;
}


Tracker::~Tracker(){
    cvReleaseImage( &pHSVImg );
    cvReleaseImage( &pHueImg );
    cvReleaseImage( &pMask );
    cvReleaseImage( &pProbImg );

    cvReleaseHist( &pHist );
}

void Tracker::updateHueImage(IplImage * pImg)
{
    // Convert to HSV color model
    cvCvtColor( pImg, pHSVImg, CV_BGR2HSV );

    // Mask out-of-range values
    cvInRangeS( pHSVImg, cvScalar(0, smin, MIN(vmin,vmax), 0),
                cvScalar(180, 256, MAX(vmin,vmax) ,0), pMask );

    // Extract the hue channel
    cvSplit( pHSVImg, pHueImg, 0, 0, 0 );
}

CvBox2D Tracker::track(IplImage * pImg)
{
    CvConnectedComp components;

    updateHueImage(pImg);

    cvCalcBackProject( &pHueImg, pProbImg, pHist );
    cvAnd( pProbImg, pMask, pProbImg, 0 );

    cvCamShift( pProbImg, prevFaceRect,
                cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
                &components, &faceBox );


    prevFaceRect = components.rect;
    faceBox.angle = -faceBox.angle;

    drawOutput(pImg, faceBox);

    return faceBox;
}

void Tracker::drawOutput(IplImage *img, CvBox2D box) {
	CvScalar red = cvScalar(0,0,255);
	CvScalar green = cvScalar(0,255,0);
	CvScalar white = cvScalar(255,255,255);
	CvScalar yellow = cvScalar(0,255,255);
	CvScalar cyan = cvScalar(255,255,0);
	CvScalar *color;
    color = &red;

    int sz = (box.size.width+box.size.height)/2.f;
	cvCircle(img, cvPoint(box.center.x,box.center.y), sz, *color);
//	cvLine(img, box.center, cvPoint(box[2].x,box[2].y), *color);
}

float Tracker::getPrevWidth() {
    return prevFaceRect.width;
}

float Tracker::getPrevHeight() {
    return prevFaceRect.height;
}

