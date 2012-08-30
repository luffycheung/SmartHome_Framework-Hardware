#define LOG_TAG "BeepService"
#include <hardware/hardware.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include "beep.h"
#define BEEP_ON   _IO('c',1)
#define BEEP_OFF  _IO('c',2)

int fd;

static int beep_device_close(struct hw_device_t* device)
{
	struct beep_control_context_t* ctx = (struct beep_control_context_t*)device;
	if (ctx) {
		free(ctx);
	}
	close(fd);
	return 0;
}

static int beep_getcount(struct beep_control_device_t *dev)
{
	LOGI("beep_getcount");
	return 4;
}


static int beep_set_on(struct beep_control_device_t *dev)
{    
	//FIXME: do system call to control gpio beep
	LOGI("beep_set_on");
	ioctl(fd,BEEP_ON);  //GPF0 0
	return 0;
} 

static int beep_set_off(struct beep_control_device_t *dev)
{
	//FIXME: do system call to control gpio beep
	LOGI("beep_set_off");
	ioctl(fd,BEEP_OFF); //GPF0 1
	return 0;

}

static int beep_device_open(const struct hw_module_t* module, const char* name,
	struct hw_device_t** device)
{
	struct beep_control_context_t *context;
	LOGD("beep_device_open");
	context = (struct beep_control_context_t *)malloc(sizeof(*context));
	memset(context, 0, sizeof(*context)); 

	//HAL must init property
	context->device.common.tag= HARDWARE_DEVICE_TAG;
	context->device.common.version = 0;
	context->device.common.module= module;
	context->device.common.close = beep_device_close; 

	// 初始化控制 API 
	context->device.set_on= beep_set_on;
	context->device.set_off= beep_set_off;
	context->device.getcount_beep = beep_getcount;
	*device= (struct hw_device_t *)&(context->device);

	if((fd=open("/dev/beep",O_RDWR))==-1)
	{
		LOGI("open error");
	//	exit(1);
	}else
	LOGI("open ok\n");

	return 0;
}


static struct hw_module_methods_t beep_module_methods = {
open: beep_device_open  
};

const struct beep_module_t HAL_MODULE_INFO_SYM = {
common: {
tag: HARDWARE_MODULE_TAG,
	 version_major: 1,
	 version_minor: 0,
	 id: BEEP_HARDWARE_MODULE_ID,
	 name: "beep HAL module",
	 author: "jellybean",
	 methods: &beep_module_methods,
		}, 
		/* supporting APIs go here */
};



