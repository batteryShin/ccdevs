/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_lge_ccdevs_tracker_CameraPreview */

#ifndef _Included_com_lge_ccdevs_tracker_CameraPreview
#define _Included_com_lge_ccdevs_tracker_CameraPreview
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_lge_ccdevs_tracker_CameraPreview
 * Method:    native_cv_facex
 * Signature: (Landroid/graphics/Bitmap;)V
 */
JNIEXPORT void JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1facex
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_lge_ccdevs_tracker_CameraPreview
 * Method:    native_cv_init
 * Signature: (Landroid/graphics/Bitmap;Landroid/graphics/RectF;)V
 */
JNIEXPORT void JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1init
  (JNIEnv *, jobject, jobject, jobject);

/*
 * Class:     com_lge_ccdevs_tracker_CameraPreview
 * Method:    native_cv_track
 * Signature: (Landroid/graphics/Bitmap;)Landroid/graphics/RectF;
 */
JNIEXPORT jobject JNICALL Java_com_lge_ccdevs_tracker_CameraPreview_native_1cv_1track
  (JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
