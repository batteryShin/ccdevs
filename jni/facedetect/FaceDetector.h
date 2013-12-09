#include <stdio.h>

#include <cv.h>
#include <cxcore.h>

static CvScalar colors[] =
{
    {{0,0,255}},
    {{0,128,255}},
    {{0,255,255}},
    {{0,255,0}},
    {{255,128,0}},
    {{255,255,0}},
    {{255,0,0}},
    {{255,0,255}}
};

class FaceDetector {

#define USE_EYE_CASCADE
#define FACE_CASCADE_NAME	"/data/data/com.lge.facealign/databases/haarcascade_frontalface_alt.xml"
#define EYES_CASCADE_NAME	"/data/data/com.lge.facealign/databases/haarcascade_eye_tree_eyeglasses.xml"
#define DETECTION_SCALE		1

public:
	// Constructor, Destructor..
	FaceDetector();
	~FaceDetector();

	/** Function Headers */
	void detect_faces(IplImage *img);
	void display_faces(IplImage *img);

	/** Global variables */
	CvMemStorage* storage;
	CvHaarClassifierCascade* face_cascade;
	CvHaarClassifierCascade* eyes_cascade;

	CvSeq* mfaces;
	CvSeq* meyes;
};
