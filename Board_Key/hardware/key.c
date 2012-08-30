#define LOG_TAG		"KeyStub"

#include <hardware/hardware.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include "key.h"


int fd;

static int key_device_close(struct hw_device_t *device)
{
	struct key_control_context_t *ctx = (struct key_control_context_t *)device;
	if (ctx) {
		free(ctx);
	}
	close(fd);

	return 0;
}

/* 将按键值从fd中读到key中，key会被jni层的key_value接收*/
static int read_key(struct key_control_device_t *dev, int32_t *key)
{
	read(fd, (void *)&key, sizeof(int32_t));
	return key;
}

static int key_device_open(const struct hw_module_t *module, const char *name,
		struct hw_device_t** device)
{
	struct key_control_device_t *dev;
	dev = malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	dev->common.tag = HARDWARE_DEVICE_TAG;
	dev->common.version = 0;
	dev->common.module = (struct hw_module_t *)module;
	dev->common.close = key_device_close;

	dev->read_key = read_key;
	*device = &dev->common;

	if ((fd = open("/dev/key", O_RDWR)) == -1)
	{
		LOGI("open error");
		return -1;	
	}
	else
		LOGI("open ok\n");

	return 0;
}

static struct hw_module_methods_t key_module_methods = {
open:key_device_open
};

const struct key_module_t HAL_MODULE_INFO_SYM  = {
common:{
tag:HARDWARE_MODULE_TAG,
	version_major: 1,
	version_minor: 0,
	id: KEY_HARDWARE_MODULE_ID,
	name: "key HAL module",
	author: "jellybean",
	methods: &key_module_methods,
	   },
};
