/* DO NOT EDIT THIS FILE - it is machine generated */
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <sstream>

//#include <utils/threads.h>
//#include <utils/Log.h>
//#include <utils/Errors.h>  // for status_t
//#include <core/SkBitmap.h>
#include <android/bitmap.h>

#include <cvjni.h>
#include <highgui.h>

#include <nativehelper/JNIHelp.h>

#include "com_lge_ccdevs_tracker_CameraPreview.h"
#include <FaceDetector.h>
#include <FaceAligner.h>
#include <Tracker.h>
#include <Matcher.h>

#define TAG_DEBUG "CameraPreview-JNI"

/* Header for class com_lge_ccdevs_tracker_CameraActivity */

#ifdef __cplusplus
extern "C" {
#endif

struct fields_t {
	jclass bitmapClazz;
	jmethodID bitmapConstructor;

	jclass fileClazz;
	jmethodID fileConstructor;

	jclass rectfClazz;
    jmethodID rectfConstructor;
	jfieldID rectf_left_ID;
	jfieldID rectf_top_ID;
	jfieldID rectf_right_ID;
	jfieldID rectf_bottom_ID;
	jfieldID rectfField;
};

static fields_t fields;
static const char* const kClassPathName = "com/lge/ccdevs/tracker/CameraPreview";

static Tracker *tracker;
static Matcher *matcher;
static float *res_pts;
static int ncount_track = 0;
static int ncount_match = 0;

JNIEXPORT void JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1init__Landroid_graphics_Bitmap_2Landroid_graphics_RectF_2
(JNIEnv *env, jobject thiz, jobject srcimg, jobject rgn) {
	jclass clazz;

	clazz = env->FindClass(kClassPathName);
	if (clazz == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find com/lge/ccdevs/tracker/CameraActivity");
		return;
	}

	fields.bitmapClazz = env->FindClass("android/graphics/Bitmap");
	if (fields.bitmapClazz == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find android/graphics/Bitmap");
		return;
	}

	fields.fileClazz = env->FindClass("java/io/File");
	if (fields.fileClazz == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find java/io/File");
		return;
	}

	fields.rectfClazz = env->FindClass("android/graphics/RectF");
	if (fields.rectfClazz == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find android/graphics/RectF");
		return;
	}
    fields.rectfConstructor = env->GetMethodID(fields.rectfClazz, "<init>", "(FFFF)V");
	if (fields.rectfConstructor == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find RectF constructor");
		return;
	}
    fields.rectf_left_ID = env->GetFieldID(fields.rectfClazz, "left", "F");
    fields.rectf_top_ID = env->GetFieldID(fields.rectfClazz, "top", "F");
    fields.rectf_right_ID = env->GetFieldID(fields.rectfClazz, "right", "F");
    fields.rectf_bottom_ID = env->GetFieldID(fields.rectfClazz, "bottom", "F");



    // convert img
	AndroidBitmapInfo bInfo;
    char *bPixs;
	int bRet;
	if ((bRet = AndroidBitmap_getInfo(env, srcimg, &bInfo)) < 0) {
		LOGE("AndroidBitmap_getInfo failed(src)! error = %d", bRet);
		return;
	}
	if (bInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap(src) format is not RGBA_8888!");
		return;
	}

	if ((bRet = AndroidBitmap_lockPixels(env, srcimg, (void**)&bPixs)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed(src)! error = %d", bRet);
		return;
	}

	IplImage* bimg = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 4);
	memcpy(bimg->imageData, bPixs, bimg->imageSize);
	AndroidBitmap_unlockPixels(env, srcimg);


	IplImage* img = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 3);
    cvCvtColor(bimg, img, CV_RGBA2BGR);

    // test capture
    Converter::saveJPG("/sdcard/tracker_init.jpg", img);

    // convert RectF
    float left = env->GetFloatField(rgn, fields.rectf_left_ID);
    float top = env->GetFloatField(rgn, fields.rectf_top_ID);
    float right = env->GetFloatField(rgn, fields.rectf_right_ID);
    float bottom = env->GetFloatField(rgn, fields.rectf_bottom_ID);

    LOGE("#### assign initial box = ( %f, %f, %f, %f )",left,top,right,bottom);
	tracker = new Tracker(img, cvRect(left,top,right-left,bottom-top));

    cvReleaseImage( &bimg );
    cvReleaseImage( &img );
}


JNIEXPORT void JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1init__Landroid_graphics_Bitmap_2_3F
(JNIEnv *env, jobject thiz, jobject srcimg, jfloatArray pts) {
	jclass clazz;

	clazz = env->FindClass(kClassPathName);
	if (clazz == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find com/lge/ccdevs/tracker/CameraActivity");
		return;
	}

	fields.bitmapClazz = env->FindClass("android/graphics/Bitmap");
	if (fields.bitmapClazz == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find android/graphics/Bitmap");
		return;
	}

	fields.fileClazz = env->FindClass("java/io/File");
	if (fields.fileClazz == NULL) {
		jniThrowException(env, "java/lang/RuntimeException", "Can't find java/io/File");
		return;
	}

    // convert img
	AndroidBitmapInfo bInfo;
    char *bPixs;
	int bRet;
	if ((bRet = AndroidBitmap_getInfo(env, srcimg, &bInfo)) < 0) {
		LOGE("AndroidBitmap_getInfo failed(src)! error = %d", bRet);
		return;
	}
	if (bInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap(src) format is not RGBA_8888!");
		return;
	}

	if ((bRet = AndroidBitmap_lockPixels(env, srcimg, (void**)&bPixs)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed(src)! error = %d", bRet);
		return;
	}

	IplImage* bimg = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 4);
	memcpy(bimg->imageData, bPixs, bimg->imageSize);
	AndroidBitmap_unlockPixels(env, srcimg);


	IplImage* img = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 3);
    cvCvtColor(bimg, img, CV_RGBA2BGR);

    // test capture
    Converter::saveJPG("/sdcard/matcher_init.jpg", img);

    // convert to float[8]
    jfloat* fpts = env->GetFloatArrayElements(pts,0);
    float points[8];
    for(int i=0; i<8; i++) {
        points[i] = fpts[i];
    }
    LOGE("#### assign initial pts = (%f,%f), (%f,%f), (%f,%f), (%f,%f) )",
            points[0], points[1], points[2], points[3],
            points[4], points[5], points[6], points[7] );

	matcher = new Matcher(img, points);
    res_pts = new float[8];

    cvReleaseImage( &bimg );
    cvReleaseImage( &img );
}


JNIEXPORT void JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1facex
(JNIEnv *env, jobject thiz, jobject srcimg) {
	AndroidBitmapInfo bInfo;
//    uint32_t* bPixs;
    char *bPixs;
	int bRet;
	LOGE("**IN JNI bitmap converter IN!");
	//1. retrieve information about the bitmap
	if ((bRet = AndroidBitmap_getInfo(env, srcimg, &bInfo)) < 0) {
		LOGE("AndroidBitmap_getInfo failed! error = %d", bRet);
		return;
	}
	if (bInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888!");
		return;
	}

	//2. lock the pixel buffer and retrieve a pointer to it
	if ((bRet = AndroidBitmap_lockPixels(env, srcimg, (void**)&bPixs)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed! error = %d", bRet);
		return;
	}

	//3. convert bitmap to IplImage
	LOGE("#### Start JNI bitmap processing");
	IplImage* img = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 4);
	memcpy(img->imageData, bPixs, img->imageSize);
	AndroidBitmap_unlockPixels(env, srcimg);

	//4. apply processing
	IplImage* dst = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 4);
	cvCopy(img, dst);

	FaceDetector *detector = new FaceDetector();
	detector->detect_faces(dst);
//	detector->display_faces(dst);

	if(detector->mfaces) {
		FaceAligner *aligner = new FaceAligner();
		CvRect rt_ref = cvRect(0,0,bInfo.width,bInfo.height);
		aligner->align_faces(dst, *(CvRect*)cvGetSeqElem(detector->mfaces,0), rt_ref);
	}

/*
	SkBitmap* bitmap = new SkBitmap;
	bitmap->setConfig(SkBitmap::kARGB_8888_Config, bInfo.width, bInfo.height);
	if( !bitmap->allocPixels() ) {
		android_printLog(ANDROID_LOG_DEBUG, "CheckPoint", "Fail: allocPixels");
		delete bitmap;
		return NULL;
	}

	char *p = (char *)bitmap->getPixels();
	memcpy( p, dst->imageData, dst->imageSize );
*/
	memcpy( bPixs, dst->imageData, dst->imageSize );
	AndroidBitmap_unlockPixels(env, srcimg);

	cvReleaseImage(&img);
	cvReleaseImage(&dst);
	LOGE("#### End processing");

//	return env->NewObject(fields.bitmapClazz, fields.bitmapConstructor, (int)bitmap, p, true, NULL, -1);
	return;
}

JNIEXPORT jobject JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1track
(JNIEnv *env, jobject thiz, jobject srcimg) {
	AndroidBitmapInfo bInfo;
    char *bPixs;
	int bRet;

    // convert img
	if ((bRet = AndroidBitmap_getInfo(env, srcimg, &bInfo)) < 0) {
		LOGE("AndroidBitmap_getInfo failed(src)! error = %d", bRet);
		return 0;
	}
	if (bInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap(src) format is not RGBA_8888!");
		return 0;
	}

	if ((bRet = AndroidBitmap_lockPixels(env, srcimg, (void**)&bPixs)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed(src)! error = %d", bRet);
		return 0;
	}

	IplImage* bimg = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 4);
	memcpy(bimg->imageData, bPixs, bimg->imageSize);
	AndroidBitmap_unlockPixels(env, srcimg);
	IplImage* img = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 3);
    cvCvtColor(bimg, img, CV_RGBA2BGR);

    CvBox2D res_box = tracker->track(img);

    // test capture
    stringstream ss;
    ss << "/sdcard/tracker_track" << ncount_track++ << ".jpg";
    Converter::saveJPG(ss.str().c_str(), img);

    float tw = res_box.size.width;
    float th = res_box.size.height;
    float tcx = res_box.center.x;
    float tcy = res_box.center.y;

    float left, top, right, bottom;
    
    if( tw>0 ) {
        left = tcx-tw/2;
        right = tcx+tw/2;
    } else {
        left = tcx - tracker->getPrevWidth()/2;
        right = tcx + tracker->getPrevWidth()/2;
    }
    if( th>0 ) {
        top = tcy-th/2;
        bottom = tcy+th/2;
    } else {
        top = tcy - tracker->getPrevHeight()/2;
        bottom = tcy + tracker->getPrevHeight()/2;
    }

    LOGE("#### tracked box = ( %f, %f, %f, %f )",left,top,right,bottom);
    cvReleaseImage( &bimg );
    cvReleaseImage( &img );

	fields.rectfClazz = env->FindClass("android/graphics/RectF");
    fields.rectfConstructor = env->GetMethodID(fields.rectfClazz, "<init>", "(FFFF)V");
    return env->NewObject(fields.rectfClazz, fields.rectfConstructor, left, top, right, bottom);
}

JNIEXPORT jfloatArray JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1match
(JNIEnv *env, jobject thiz, jobject srcimg) {
	AndroidBitmapInfo bInfo;
    char *bPixs;
	int bRet;

    // convert img
	if ((bRet = AndroidBitmap_getInfo(env, srcimg, &bInfo)) < 0) {
		LOGE("AndroidBitmap_getInfo failed(src)! error = %d", bRet);
		return 0;
	}
	if (bInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap(src) format is not RGBA_8888!");
		return 0;
	}

	if ((bRet = AndroidBitmap_lockPixels(env, srcimg, (void**)&bPixs)) < 0) {
		LOGE("AndroidBitmap_lockPixels() failed(src)! error = %d", bRet);
		return 0;
	}

	IplImage* bimg = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 4);
	memcpy(bimg->imageData, bPixs, bimg->imageSize);
	AndroidBitmap_unlockPixels(env, srcimg);
	IplImage* img = cvCreateImage(cvSize(bInfo.width,bInfo.height), IPL_DEPTH_8U, 3);
    cvCvtColor(bimg, img, CV_RGBA2BGR);

    
    res_pts = matcher->match(img);

    // test capture
    stringstream ss;
    ss << "/sdcard/tracker_match" << ncount_match++ << ".jpg";
    Converter::saveJPG(ss.str().c_str(), img);

    LOGE("#### matched pts = (%f,%f), (%f,%f), (%f,%f), (%f,%f) )",
            res_pts[0], res_pts[1], res_pts[2], res_pts[3],
            res_pts[4], res_pts[5], res_pts[6], res_pts[7] );
    cvReleaseImage( &bimg );
    cvReleaseImage( &img );

    jfloatArray result;
    result = env->NewFloatArray(8);
    env->SetFloatArrayRegion(result,0,8,res_pts);

    return result;
}

#ifdef __cplusplus
}
#endif
