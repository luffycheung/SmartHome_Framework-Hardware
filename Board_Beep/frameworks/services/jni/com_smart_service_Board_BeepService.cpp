#define LOG_TAG "BeepService"
#include "utils/Log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <jni.h>
#include "../../../hardware/beep.h"

static beep_control_device_t *sBeepDevice = 0;
static beep_module_t* sBeepModule=0;

static jint get_count(void)
{
	if(sBeepDevice)
		sBeepDevice->getcount_beep(sBeepDevice);
	else
		LOGI("sBeepDevice is null");
	return 0;
}

static jint beep_setOn(JNIEnv* env, jobject thiz) {
	if (sBeepDevice) {
		LOGI("beep_set_on");
		sBeepDevice->set_on(sBeepDevice); //调用HAL层方法
	}else{
		LOGI("sBeepDevice is null");
	}
	return 0;
}



static jint beep_setOff(JNIEnv* env, jobject thiz) {
	if (sBeepDevice) {
		LOGI("beep_set_off");
		sBeepDevice->set_off(sBeepDevice);
	}else{
		LOGI("sBeepDevice is null");
	}

	return 0;
}

/** helper APIs */
static inline int beep_control_open(const struct hw_module_t* module,
	struct beep_control_device_t** device) {
	LOGI("beep_control_open");//通过标准的Open接口，调用stub中的open函数
	return module->methods->open(module,
		BEEP_HARDWARE_MODULE_ID, (struct hw_device_t**)device);
}

static jint beep_init(JNIEnv *env, jclass clazz)
{
	beep_module_t const * module;
	LOGI("beep_init");   
/*
 *调用Android HAL 标准函数hw_get_module,通过BEEP_HARDWARE_MODULE_ID获取BEEP Stub的fd
 * fd保存在module变量中
 */
	if (hw_get_module(BEEP_HARDWARE_MODULE_ID, (const hw_module_t**)&module) == 0) {
		LOGI("get Module OK");     
		sBeepModule = (beep_module_t *) module;
		if (beep_control_open(&module->common, &sBeepDevice) != 0) {
			LOGI("beep_init error");
			return -1;
		}
	}

	LOGI("beep_init success");
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
		(void*)beep_init},
	{ "_set_on",          "()I",
		(void*)beep_setOn },
	{ "_set_off",          "()I",
		(void*)beep_setOff },
	{ "_get_count",          "()I",
		(void*)get_count },
};

static int registerMethods(JNIEnv* env) {
	static const char* const kClassName =
		"com/smart/service/Board_BeepService";
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
		LOGE("Faibeep registering methods for %s\n", kClassName);
		return -1;
	}
	/* fill out the rest of the ID cache */
	return 0;
} 

/*
 *
 *   * This is called by the VM when the shared library is first loaded.
 */ 
jint JNI_OnLoad(JavaVM* vm, void* reserved)  //实现函数registerMethods向当前Java环境中注册接口
{
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


