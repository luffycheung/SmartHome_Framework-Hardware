#define LOG_TAG		"KeyService"
#include "utils/Log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <jni.h>
#include "../../../hardware/key.h"

static key_control_device_t *sKeyDevice = 0;
static key_module_t *sKeyModule = 0;

static jint read_key(JNIEnv *env, jobject thiz)
{
	int key_value = 0;
	int temp = 0;
	if (sKeyDevice)
	{
		temp = sKeyDevice->read_key(sKeyDevice, &key_value);
	}
	else
	{
		LOGE("sKeyDevice is Null");
	}
	return temp;
}

static inline int key_control_open(const struct hw_module_t *module,
		struct key_control_device_t** device)
{
	return module->methods->open(module, KEY_HARDWARE_MODULE_ID, 
			(struct hw_device_t **)device);
}


static jint key_init(JNIEnv *env, jclass clazz)
{
	key_module_t const *module;

	if (hw_get_module(KEY_HARDWARE_MODULE_ID, 
				(const hw_module_t **)&module) == 0)
	{
		sKeyModule = (key_module_t *)module;
		if (key_control_open(&module->common, &sKeyDevice) != 0)
		{
			LOGI("key_init error");
		}
	}
	return 0;
}


static const JNINativeMethod gMethods[] = {
	{"_init",		"()I",		(void *)key_init},
	{"_read_key",		"()I",		(void *)read_key},
};

static jint registerMethods(JNIEnv* env)
{
	static const char* const kClassName =
		"com/smart/service/Board_KeyService";
	jclass clazz;
	clazz = env->FindClass(kClassName);
	if (clazz == NULL)
	{
		LOGE("can't find class %s\n", kClassName);
		return -1;
	}

	if (env->RegisterNatives(clazz, gMethods, 
				sizeof(gMethods)/sizeof(gMethods[0])) != JNI_OK)
	{
		LOGE("Failed registering methods for %s\n", kClassName);
		return -1;
	}	

	return 0;
}


jint JNI_OnLoad(JavaVM* vm, void *reserved)
{
	JNIEnv *env = NULL;
	jint result = -1;
	LOGI("JNI_OnLoad");

	if (vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK)
	{
		LOGE("ERROR: GetEnv failed\n");
		goto fail;
	}

	assert(env != NULL);
	if (registerMethods(env) != 0)
	{
		LOGE("ERROR: PlatformLibrary native registration failed\n");
		goto fail;
	}

	result = JNI_VERSION_1_4;

fail:
	return result;
}
