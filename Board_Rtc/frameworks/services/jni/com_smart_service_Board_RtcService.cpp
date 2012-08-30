#define LOG_TAG "RtcService"
#include "utils/Log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <jni.h>
#include "../../../hardware/rtc.h"

static rtc_control_device_t *sRtcDevice = 0;
static rtc_module_t* sRtcModule=0;
JNIEnv* env = NULL;
int args[6];
int sec;
int min;
int hour;
int date;
int mon;
int year;
static jint rtc_close(JNIEnv* env,jobject thiz){
	if(sRtcDevice){
		LOGI("rtc_close");
		sRtcDevice -> common.close((hw_device_t*)sRtcDevice);
	}
	else
	{
		LOGI("sRtcDevice is null");
	}
	return 0;
}
static jint rtc_read_timesec(JNIEnv* env, jobject thiz) {
	if (sRtcDevice) {
		LOGI("rtc_read");
		sec = sRtcDevice->read_sec(sRtcDevice);
	}else{
		LOGI("sRtcDevice is null");
	}
	return sec;
}

static jint rtc_read_timemin(JNIEnv* env, jobject thiz) {
	if (sRtcDevice) {
		LOGI("rtc_read");
		min = sRtcDevice->read_min(sRtcDevice);
	}else{
		LOGI("sRtcDevice is null");
	}
	return min;
}

static jint rtc_read_timehour(JNIEnv* env, jobject thiz) {
	if (sRtcDevice) {
		LOGI("rtc_read");
		hour = sRtcDevice->read_hour(sRtcDevice);
	}else{
		LOGI("sRtcDevice is null");
	}
	return hour;
}
static jint rtc_read_timedate(JNIEnv* env, jobject thiz) {
	if (sRtcDevice) {
		LOGI("rtc_read");
		date = sRtcDevice->read_date(sRtcDevice);
	}else{
		LOGI("sRtcDevice is null");
	}
	return date;
}
static jint rtc_read_timemon(JNIEnv* env, jobject thiz) {
	if (sRtcDevice) {
		LOGI("rtc_read");
		mon = sRtcDevice->read_mon(sRtcDevice);
	}else{
		LOGI("sRtcDevice is null");
	}
	return mon;
}
static jint rtc_read_timeyear(JNIEnv* env, jobject thiz) {
	if (sRtcDevice) {
		LOGI("rtc_read");
		year = sRtcDevice->read_year(sRtcDevice);
	}else{
		LOGI("sRtcDevice is null");
	}
	return year;
}

static jint rtc_write(JNIEnv* env, jobject thiz, jintArray args) {
	if (sRtcDevice) {
		LOGI("rtc_write");
		jint *body = env->GetIntArrayElements(args, NULL);
		LOGI("sRtcDevice year:mon:date=%02x:%02x:%02x hour:min:sec=%02x:%02x:%02x", body[0], body[1], body[2], body[3], body[4], body[5]);
		sRtcDevice->write_tm(sRtcDevice, (int *)body);
		LOGI("sRtcDevice year:mon:date=%02x:%02x:%02x hour:min:sec=%02x:%02x:%02x", body[0], body[1], body[2], body[3], body[4], body[5]);
		env->ReleaseIntArrayElements(args, body, 0);
	}else{
		LOGI("sRtcDevice is null");
	}
	return 0;
}

/** helper APIs */
static inline int rtc_control_open(const struct hw_module_t* module,
		struct rtc_control_device_t** device) {
	LOGI("rtc_control_open");//通过标准的Open接口，调用stub中的open函数
	return module->methods->open(module,
			RTC_HARDWARE_MODULE_ID, (struct hw_device_t**)device);
}

static jint rtc_init(JNIEnv *env, jclass clazz)
{
	rtc_module_t const * module;
	LOGI("rtc_init");   

	if (hw_get_module(RTC_HARDWARE_MODULE_ID, (const hw_module_t**)&module) == 0) {
		LOGI("get Module OK");     
		sRtcModule = (rtc_module_t *) module;
		if (rtc_control_open(&module->common, &sRtcDevice) != 0) {
			LOGI("rtc_init error");
			return -1;
		}
	}

	LOGI("rtc_init success");
	return 0;
}

/*
 *
 ** Array of methods.
 ** Each entry has three fields: the name of the method, the method
 ** signature, and a pointer to the native implementation.
 */ 
static const JNINativeMethod gMethods[] = {
	{"_init",     "()I",
		(void*)rtc_init},
	{"_close", "()I",
		(void*)rtc_close},
	{ "_read_rtc_sec",          "()I",
		(void*)rtc_read_timesec },
	{ "_read_rtc_min",          "()I",
		(void*)rtc_read_timemin },
	{ "_read_rtc_hour",          "()I",
		(void*)rtc_read_timehour },
	{ "_read_rtc_date",          "()I",
		(void*)rtc_read_timedate },
	{ "_read_rtc_mon",          "()I",
		(void*)rtc_read_timemon },
	{ "_read_rtc_year",          "()I",
		(void*)rtc_read_timeyear },
	{ "_write_rtc_time",          "([I)I",
		(void*)rtc_write },
};

static int registerMethods(JNIEnv* env) {
	LOGI("registerMethods enter");
	static const char* const kClassName =
		"com/smart/service/Board_RtcService";
	jclass clazz; 
	/* look up the class */
	clazz = env->FindClass(kClassName);
	if (clazz == NULL) {
		LOGE("Can't find class %s\n", kClassName);
		return -1;
	} 

	/* register all the methods */
	if (env->RegisterNatives(clazz, gMethods,
				sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
	{
		LOGE("Failed registering methods for %s\n", kClassName);
		return -1;
	}
	/* fill out the rest of the ID cache */
	return 0;
} 

/*
 *
 *   * This is called by the VM when the shared library is first loaded.
 */ 
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	jint result = -1;
	LOGI("JNI_OnLoad");

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("ERROR: GetEnv failed\n");
		goto fail;
	}

	assert(env != NULL);
	LOGE("assert success");
	if (registerMethods(env) != 0) {
		LOGE("ERROR: PlatformLibrary native registration failed\n");
		goto fail;
	}

	LOGI("registerMethods success");
	/* success -- return valid version number */	
	result = JNI_VERSION_1_4;

fail:
	return result;
} 


