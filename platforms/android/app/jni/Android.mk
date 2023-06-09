LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := SDL2
LIB_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/SDL2-2.26.5
LOCAL_C_INCLUDES := $(LIB_PATH)/include
LOCAL_SRC_FILES := $(LIB_PATH)/libsdl2.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := freetype
SO_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/freetype-2.12.1
LOCAL_SRC_FILES :=  $(SO_PATH)/libfreetype.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libpng
SO_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/libpng-1.6.39
LOCAL_SRC_FILES :=  $(SO_PATH)/libpng16.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := SDL2_ttf
LIB_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/SDL2_ttf-2.20.1
LOCAL_SRC_FILES :=  $(LIB_PATH)/libsdl2_ttf.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := SDL2_mixer
LIB_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/SDL2_mixer-2.6.2
LOCAL_SRC_FILES :=  $(LIB_PATH)/libsdl2mixer.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libvorbis
LIB_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/libvorbis-1.3.7/lib
LOCAL_SRC_FILES :=  $(LIB_PATH)/libvorbis.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libvorbisfile
LIB_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/libvorbis-1.3.7/lib
LOCAL_SRC_FILES :=  $(LIB_PATH)/libvorbisfile.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libogg
LIB_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/libogg-1.3.5/src
LOCAL_SRC_FILES :=  $(LIB_PATH)/libogg.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libflac
LIB_PATH := ../../../../build-$(TARGET_ARCH_ABI)/subprojects/flac-1.4.2/src
LOCAL_SRC_FILES :=  $(LIB_PATH)/libFLAC/libFLAC.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := liboopetris
LOCAL_SRC_FILES := ../../../../build-$(TARGET_ARCH_ABI)/liboopetris.so
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE := main
LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf freetype libpng SDL2_mixer libvorbis libvorbisfile libogg libflac liboopetris
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid
LOCAL_LDFLAGS := -Wl,--no-undefined
TARGET_ARCH_ABI= arm64-v8a
include $(BUILD_SHARED_LIBRARY)
