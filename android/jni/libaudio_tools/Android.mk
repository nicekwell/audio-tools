LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libaudio_tools
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := libsamplerate
LOCAL_SRC_FILES := audio_tools.c resample.c queue.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/
include $(BUILD_STATIC_LIBRARY)

