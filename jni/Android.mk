LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#include Opencv.mk
ANDROID_PATH := $(LOCAL_PATH)/../libs/android

OPENCV_PATH := $(LOCAL_PATH)/../libs/opencv
REFLIB_PATH := $(LOCAL_PATH)/../libs/prebuilt

LOCAL_C_INCLUDES += 					\
	$(OPENCV_PATH)						\
	$(OPENCV_PATH)/cv/include			\
	$(OPENCV_PATH)/cxcore/include		\
	$(OPENCV_PATH)/cvaux/include		\
	$(OPENCV_PATH)/ml/include			\
	$(OPENCV_PATH)/otherlibs/highgui

ifdef HISTORICAL_NDK_VERSIONS_ROOT # In the platform build system
LOCAL_STATIC_LIBRARIES += libcxcore libcv libcvaux libcvml libcvhighgui libopencv
else # In the NDK build system
LOCAL_LDLIBS += -L$(REFLIB_PATH) -lcxcore -lcv -lcvaux -lcvml -lcvhighgui -lopencv
endif

#LOCAL_H_INCLUDE:= kpmtypes.h
LOCAL_SRC_FILES:= \
	com_lge_ccdevs_tracker_CameraPreview.cpp	\
	FaceDetector.cpp	\
	FaceAligner.cpp		\
	Tracker.cpp			\
	Vec2d.cpp			\
	Vec3d.cpp			\
	Homography.cpp		\
	Warping.cpp			\
	Converter.cpp

ifdef HISTORICAL_NDK_VERSIONS_ROOT # In the platform build system
LOCAL_SHARED_LIBRARIES += \
	libandroid_runtime \
    libnativehelper \
	libdl \
	libjpeg \
	libskia
else # In the NDK build system
LOCAL_LDLIBS += -L$(REFLIB_PATH) -landroid_runtime -lnativehelper -ldl -lskia -ljpeg
endif

ifdef HISTORICAL_NDK_VERSIONS_ROOT # In the platform build system
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    libnativehelper/include \
    external/jpeg \
    external/skia/include \
    $(call include-path-for, corecg graphics)
else # In the NDK build system
LOCAL_C_INCLUDES += \
	$(ANDROID_PATH)/frameworks/native/include \
	$(ANDROID_PATH)/frameworks/native/opengl/include \
    $(ANDROID_PATH)/frameworks/base/include \
    $(ANDROID_PATH)/libnativehelper/include	\
    $(ANDROID_PATH)/system/core/include \
	$(REFLIB_PATH)/../jpeg \
	$(REFLIB_PATH)/../skia/include
endif

LOCAL_CFLAGS := -DANDROID_NDK -g -O0
LOCAL_LDFLAGS += -fuse-ld=bfd -llog -ljnigraphics
#LOCAL_LDLIBS :=

#LOCAL_PRELINK_MODULE := false

ifdef HISTORICAL_NDK_VERSIONS_ROOT # In the platform build system
LOCAL_SHARED_LIBRARIES += libstlport
else # In the NDK build system
LOCAL_STATIC_LIBRARIES += libstlport_static
endif

ifeq ($(FLAG_DO_PROFILE), true)
$(warning Making profiling version of native library)
LOCAL_SHARED_LIBRARIES += libcutils libutils
else # FLAG_DO_PROFILE
ifeq ($(FLAG_DBG), true)
$(warning Making debug version of native library)
LOCAL_SHARED_LIBRARIES += libcutils libutils
endif # FLAG_DBG
endif # FLAG_DO_PROFILE

LOCAL_MODULE_PATH :=
LOCAL_MODULE := libTracker_jni
LOCAL_MODULE_TAGS := optional

ifdef HISTORICAL_NDK_VERSIONS_ROOT # In the platform build system
include $(ANDROID_PATH)/external/stlport/libstlport.mk
endif

include $(BUILD_SHARED_LIBRARY)
