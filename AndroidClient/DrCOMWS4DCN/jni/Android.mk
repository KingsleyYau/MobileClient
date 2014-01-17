MAIN_LOCAL_PATH := $(call my-dir)/../../../DrClientLib
include $(CLEAR_VARS)
LOCAL_PATH := $(MAIN_LOCAL_PATH)
LOCAL_MODULE := DrClientLib

LOCAL_C_INCLUDES := $(MAIN_LOCAL_PATH)/polarssl_inc
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/DrClientSrc
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(call my-dir)

# prebuild marco for .c
#LOCAL_CFLAGS := -std=gnu99
# prebuild marco for .cpp
LOCAL_CPPFLAGS += -std=gnu++0x -D_ANDROID
#LOCAL_CPPFLAGS += -_ANDROID
#LOCAL_LDLIBS := -lssl -lcrypto

LOCAL_SRC_FILES :=\
	library/aes.c library/arc4.c \
	library/asn1parse.c library/asn1write.c \
	library/base64.c library/bignum.c \
	library/blowfish.c library/camellia.c \
	library/certs.c library/cipher_wrap.c \
	library/cipher.c library/ctr_drbg.c \
	library/debug.c library/des.c \
	library/dhm.c library/entropy_poll.c \
	library/entropy.c library/error.c \
	library/gcm.c library/havege.c \
	library/md_wrap.c library/md.c \
	library/md2.c library/md4.c \
	library/md5.c library/net.c \
	library/padlock.c library/pbkdf2.c \
	library/pem.c library/pkcs11.c \
	library/rsa.c library/sha1.c \
	library/sha2.c library/sha4.c \
	library/ssl_cache.c library/ssl_cli.c \
	library/ssl_srv.c library/ssl_tls.c \
	library/timing.c library/version.c \
	library/x509parse.c library/x509write.c \
	library/xtea.c \
	DrClientSrc/Arithmetic.cpp  DrClientSrc/DrCOMAuth.cpp \
	DrClientSrc/DrCOMSocket.cpp DrClientSrc/IDrCOMAuth.cpp \
	../AndroidClient/DrCOMWS/jni/com_drcom_Android_DrCOMWS_Jni.cpp

include $(BUILD_SHARED_LIBRARY) 