#include <stdio.h>

#include <cv.h>
#include <cxcore.h>

#include "Homography.h"
#include "Warping.h"

class FaceAligner {

public:
	/** Constructor, Destructor */
	FaceAligner();
	~FaceAligner();

	/** Function Headers */
	void align_faces(IplImage *fimg, CvRect& face, const CvRect& refface);

	void warp_img(IplImage *src, IplImage *dst, double **Hinv);

	/** Class Variables */
};
