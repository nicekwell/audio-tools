#libsamplerate

LIB_VERSION:=libsamplerate

LOCAL_PATH:= $(call my-dir)
LIB_ROOT_REL:= ../../$(LIB_VERSION)
LIB_ROOT_ABS:= $(LOCAL_PATH)/../../$(LIB_VERSION)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DHAVE_CONFIG_H -Werror -g -I$(LIB_ROOT_ABS)/libsamplerate

LOCAL_SRC_FILES := \
 $(LIB_ROOT_REL)/samplerate.c \
 $(LIB_ROOT_REL)/src_linear.c \
 $(LIB_ROOT_REL)/src_sinc.c \
 $(LIB_ROOT_REL)/src_zoh.c


LOCAL_C_INCLUDES := \
 $(LIB_ROOT_ABS)/ \
 $(LIB_ROOT_ABS)/include \

  
LOCAL_LDLIBS := -llog -lm

LOCAL_SHARED_LIBRARIES := 

LOCAL_MODULE := libsamplerate

include $(BUILD_STATIC_LIBRARY)


