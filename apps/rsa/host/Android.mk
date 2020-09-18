LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)  
LOCAL_MODULE := libmtee      
LOCAL_SRC_FILES := $(CA_SDK)/libs/$(TARGET_ARCH_ABI)/libteec.so
LOCAL_EXPORT_C_INCLUDES := $(CA_SDK)/public/
include $(PREBUILT_SHARED_LIBRARY)
  
include $(CLEAR_VARS)  
LOCAL_MODULE    := ifs-rsa
LOCAL_SRC_FILES := ifs-rsa.c
  
LOCAL_SHARED_LIBRARIES  := libmtee   
LOCAL_LDLIBS += -ldl -llog
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
LOCAL_CFLAGS += -DANDROID_LOG_PRINT 
include $(BUILD_EXECUTABLE)  
