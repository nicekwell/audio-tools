LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := test_bin
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := libsamplerate libaudio_tools
LOCAL_SRC_FILES := main.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libaudio_tools/include/
include $(BUILD_EXECUTABLE)

