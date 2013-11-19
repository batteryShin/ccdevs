#include "FaceDetector.h"

FaceDetector::FaceDetector() {
	//-- 0. Initialize parameters
	storage = 0;
	face_cascade = eyes_cascade = 0;
	mfaces = meyes = 0;

	//-- 1. Load the cascades
	face_cascade = (CvHaarClassifierCascade*)cvLoad( FACE_CASCADE_NAME, 0, 0, 0 );
	if( !face_cascade ) {
		printf("--(!)Error loading\n");
	}

#ifdef USE_EYE_CASCADE
	eyes_cascade = (CvHaarClassifierCascade*)cvLoad( EYES_CASCADE_NAME, 0, 0, 0 );
	if( !eyes_cascade ) {
		printf("--(!)Error loading\n");
	}
#endif

	storage = cvCreateMemStorage(0);
}

FaceDetector::~FaceDetector() {
	cvReleaseMemStorage(&storage);
}

/** @function detect & display */
void FaceDetector::detect_faces(IplImage* img) {
	IplImage *gray, *small_img;
    CvMat small_img_roi;

	gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
	small_img = cvCreateImage( cvSize( cvRound (img->width/DETECTION_SCALE),
						 cvRound (img->height/DETECTION_SCALE)), 8, 1 );

	cvCvtColor( img, gray, CV_RGB2GRAY );
	cvResize( gray, small_img, CV_INTER_LINEAR );
	cvEqualizeHist( small_img, small_img );
	cvClearMemStorage( storage );

	if( face_cascade )
	{
		double t = (double)cvGetTickCount();
		mfaces = cvHaarDetectObjects( small_img, face_cascade, storage,
									  1.1, 2, 0
									  //|CV_HAAR_FIND_BIGGEST_OBJECT
									  //|CV_HAAR_DO_ROUGH_SEARCH
									  |CV_HAAR_DO_CANNY_PRUNING
									  //|CV_HAAR_SCALE_IMAGE
									  ,
									  cvSize(30, 30) );
		t = (double)cvGetTickCount() - t;

#ifdef USE_EYE_CASCADE
		for(int i=0; i<(mfaces?mfaces->total:0); i++)
		{
			if( !eyes_cascade )
				continue;
			CvRect* r = (CvRect*)cvGetSeqElem( mfaces, i );
			cvGetSubRect( small_img, &small_img_roi, *r );
			meyes = cvHaarDetectObjects( &small_img_roi, eyes_cascade, storage,
										 1.1, 2, 0
										 //|CV_HAAR_FIND_BIGGEST_OBJECT
										 //|CV_HAAR_DO_ROUGH_SEARCH
										 //|CV_HAAR_DO_CANNY_PRUNING
										 //|CV_HAAR_SCALE_IMAGE
										 ,
										 cvSize(0, 0) );
		}
#endif
	}
}

void FaceDetector::display_faces(IplImage *img) {
	for(int i=0; i<(mfaces?mfaces->total:0); i++) {
		CvRect* r = (CvRect*)cvGetSeqElem( mfaces, i );
		CvPoint center;
		CvScalar color = colors[i%8];
		int radius;
		center.x = cvRound((r->x + r->width*0.5)*DETECTION_SCALE);
		center.y = cvRound((r->y + r->height*0.5)*DETECTION_SCALE);
		radius = cvRound((r->width + r->height)*0.25*DETECTION_SCALE);
		cvCircle( img, center, radius, color, 3, 8, 0 );

#ifdef USE_EYE_CASCADE
		//-- In each face, draw eyes
		for(int j=0; j<(meyes?meyes->total:0); j++) {
			CvRect* nr = (CvRect*)cvGetSeqElem( meyes, j );
			center.x = cvRound((r->x + nr->x + nr->width*0.5)*DETECTION_SCALE);
			center.y = cvRound((r->y + nr->y + nr->height*0.5)*DETECTION_SCALE);
			radius = cvRound((nr->width + nr->height)*0.25*DETECTION_SCALE);
			cvCircle( img, center, radius, color, 3, 8, 0 );
		}
#endif
	}
}
