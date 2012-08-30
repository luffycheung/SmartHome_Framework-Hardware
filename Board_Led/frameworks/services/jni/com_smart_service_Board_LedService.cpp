#define LOG_TAG "LedService"
#include "utils/Log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <jni.h>
#include "../../../hardware/led.h"

static led_control_device_t *sLedDevice = 0;
static led_module_t* sLedModule=0;

static jint get_count(void)
{
	if(sLedDevice)
		sLedDevice->getcount_led(sLedDevice);
	else
		LOGI("sLedDevice is null");
	return 0;
}

static jint led_setOn(JNIEnv* env, jobject thiz, jint arg) {
	if (sLedDevice) {
		LOGI("led_set_on");//调用HAL层方法
		sLedDevice->set_on(sLedDevice, (int)arg);
	}else{
		LOGI("sLedDevice is null");
	}
	return 0;
}



static jint led_setOff(JNIEnv* env, jobject thiz, jint arg) {
	if (sLedDevice) {
		LOGI("led_set_off");
		sLedDevice->set_off(sLedDevice, (int)arg);
	}else{
		LOGI("sLedDevice is null");
	}

	return 0;
}

/** helper APIs */
static inline int led_control_open(const struct hw_module_t* module,
	struct led_control_device_t** device) {
	LOGI("led_control_open");
	return module->methods->open(module,
		LED_HARDWARE_MODULE_ID, (struct hw_device_t**)device);
}

static jint led_init(JNIEnv *env, jclass clazz)
{
	led_module_t const * module;
	LOGI("led_init");   

	if (hw_get_module(LED_HARDWARE_MODULE_ID, (const hw_module_t**)&module) == 0) {
		LOGI("get Module OK");     
		sLedModule = (led_module_t *) module;
		if (led_control_open(&module->common, &sLedDevice) != 0) {
			LOGI("led_init error");
			return -1;
		}
	}

	LOGI("led_init success");
	return 0;
}

/*
 *
 ** Array of methods.
 ** Each entry has three fields: the name of the method, the method
 ** signature, and a pointer to the native implementation.
 */ 
static const JNINativeMethod gMethods[] = {
	{"_init",     "()Z",
		(void*)led_init},
	{ "_set_on",          "(I)I",
		(void*)led_setOn },
	{ "_set_off",          "(I)I",
		(void*)led_setOff },
	{ "_get_count",          "()I",
		(void*)get_count },
};

static int registerMethods(JNIEnv* env) {
	static const char* const kClassName =
		"com/smart/service/Board_LedService";
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
	JNIEnv* env = NULL;
	jint result = -1;
	LOGI("JNI_OnLoad");

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("ERROR: GetEnv failed\n");
		goto fail;
	}

	assert(env != NULL);
	if (registerMethods(env) != 0) {
		LOGE("ERROR: PlatformLibrary native registration failed\n");
		goto fail;
	}
	/* success -- return valid version number */	
	result = JNI_VERSION_1_4;

fail:
	return result;
} 


