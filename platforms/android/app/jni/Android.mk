LOCAL_PATH := $(call my-dir)
BUILD_PATH=../../../../build-$(TARGET_ARCH_ABI)
SUBPROJECTS_PATH :=$(BUILD_PATH)/subprojects

# TODO: don't hardcode the versions of the dependencies 

include $(CLEAR_VARS)
LOCAL_MODULE := SDL2
LIB_PATH := $(SUBPROJECTS_PATH)/SDL2-2.30.3
LOCAL_C_INCLUDES := $(LIB_PATH)/include
LOCAL_SRC_FILES := $(LIB_PATH)/libsdl2.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := freetype
SO_PATH := $(SUBPROJECTS_PATH)/freetype-2.13.2
LOCAL_SRC_FILES :=  $(SO_PATH)/libfreetype.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libpng
SO_PATH := $(SUBPROJECTS_PATH)/libpng-1.6.43
LOCAL_SRC_FILES :=  $(SO_PATH)/libpng16.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := SDL2_ttf
LIB_PATH := $(SUBPROJECTS_PATH)/SDL2_ttf-2.20.1
LOCAL_SRC_FILES :=  $(LIB_PATH)/libsdl2_ttf.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := SDL2_mixer
LIB_PATH := $(SUBPROJECTS_PATH)/SDL2_mixer-2.6.2
LOCAL_SRC_FILES :=  $(LIB_PATH)/libsdl2mixer.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libvorbis
LIB_PATH := $(SUBPROJECTS_PATH)/libvorbis-1.3.7/lib
LOCAL_SRC_FILES :=  $(LIB_PATH)/libvorbis.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libvorbisfile
LIB_PATH := $(SUBPROJECTS_PATH)/libvorbis-1.3.7/lib
LOCAL_SRC_FILES :=  $(LIB_PATH)/libvorbisfile.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libogg
LIB_PATH := $(SUBPROJECTS_PATH)/libogg-1.3.5/src
LOCAL_SRC_FILES :=  $(LIB_PATH)/libogg.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libflac
LIB_PATH := $(SUBPROJECTS_PATH)/flac-1.4.3/src
LOCAL_SRC_FILES :=  $(LIB_PATH)/libFLAC/libFLAC.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := SDL2_image
LIB_PATH := $(SUBPROJECTS_PATH)/SDL2_image-2.6.3
LOCAL_SRC_FILES :=  $(LIB_PATH)/libsdl2image.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libfmt
LIB_PATH := $(SUBPROJECTS_PATH)/fmt-10.2.0
LOCAL_SRC_FILES :=  $(LIB_PATH)/libfmt.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := liboopetris_core
LIB_PATH := $(BUILD_PATH)/src/libs/core
LOCAL_SRC_FILES := $(LIB_PATH)/liboopetris_core.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := liboopetris_recordings
LIB_PATH := $(BUILD_PATH)/src/libs/recordings
LOCAL_SRC_FILES := $(LIB_PATH)/liboopetris_recordings.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := liboopetris_graphics
LIB_PATH := $(BUILD_PATH)/src
LOCAL_SRC_FILES := $(LIB_PATH)/liboopetris_graphics.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := liboopetris
LIB_PATH := $(BUILD_PATH)/src/executables
LOCAL_SRC_FILES := $(LIB_PATH)/liboopetris.so
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE := main
LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf freetype libpng SDL2_mixer libvorbis libvorbisfile libogg libflac SDL2_image libfmt liboopetris_core liboopetris_recordings liboopetris_graphics liboopetris
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid
LOCAL_LDFLAGS := -Wl,--no-undefined
include $(BUILD_SHARED_LIBRARY)
