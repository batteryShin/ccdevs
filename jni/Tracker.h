#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
using namespace std;


class Tracker{

    // File-level variables
    IplImage * pHSVImg; // the input image converted to HSV color mode
    IplImage * pHueImg; // the Hue channel of the HSV image
    IplImage * pMask; // this image is used for masking pixels
    IplImage * pProbImg; // the face probability estimates for each pixel
    CvHistogram * pHist; // histogram of hue in the original face image

    CvRect prevFaceRect;  // location of face in previous frame
    CvBox2D faceBox;      // current face-location estimate

    int vmin;
    int vmax;
    int smin;

    void updateHueImage(IplImage* pImg);
public:
    Tracker(IplImage * pImg, CvRect pFaceRect);
    ~Tracker();
    CvBox2D track(IplImage* pImg);

    float getPrevWidth();
    float getPrevHeight();
};
