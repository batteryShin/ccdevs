#include <cv.h>
#include <cxcore.h>
#include <core/SkBitmap.h>

#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
//ANDROID_LOG_UNKNOWN, ANDROID_LOG_DEFAULT, ANDROID_LOG_VERBOSE, ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR, ANDROID_LOG_FATAL, ANDROID_LOG_SILENT
//LOGV(ANDROID_LOG_DEBUG, "JNI", "");

#define BitmapColorGetA(color) (((color) >> 24) & 0xFF) 
#define BitmapColorGetR(color) (((color) >> 16) & 0xFF) 
#define BitmapColorGetG(color) (((color) >> 8) & 0xFF) 
#define BitmapColorGetB(color) (((color) >> 0) & 0xFF) 

using namespace std;

class Converter{
public:
    static void saveCVIMG(const char* path, const IplImage* cvimg);
    static void saveSkBitmapToBMPFile(const SkBitmap& skBitmap, const char* path);
};
