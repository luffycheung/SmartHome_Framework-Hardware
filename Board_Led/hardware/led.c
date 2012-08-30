#define LOG_TAG "LedStub"
#include <hardware/hardware.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include "led.h"
#define LED_ON 0x4800
#define LED_OFF 0x4801
int fd;

static int led_device_close(struct hw_device_t* device)
{   //创建结构体变量ctx，用其接收强转后的参数device
	struct led_control_context_t* ctx = (struct led_control_context_t*)device;
	if (ctx) {
		free(ctx);
	}
	close(fd);
	return 0; }

static int led_getcount(struct led_control_device_t *dev)
{
	LOGI("led_getcount");
	return 4;
}


static int led_set_on(struct led_control_device_t *dev, int arg)
{    
	//FIXME: do system call to control gpio led
	LOGI("led_set_on");
	ioctl(fd,LED_ON,arg);  //GPF0 0
	return 0;
} 

static int led_set_off(struct led_control_device_t *dev, int arg)
{
	//FIXME: do system call to control gpio led
	LOGI("led_set_off");
	ioctl(fd,LED_OFF,arg); //GPF0 1
	return 0;
}

static int led_device_open(const struct hw_module_t* module, const char* name,
	struct hw_device_t** device)
{   //建立继承hw_device_t的结构体变量context,并初始化
	struct led_control_context_t *context;
	LOGD("led_device_open");
	context = (struct led_control_context_t *)malloc(sizeof(*context));
	memset(context, 0, sizeof(*context)); 

	//HAL must init property
	context->device.common.tag= HARDWARE_DEVICE_TAG;
	context->device.common.version = 0;
	context->device.common.module= module;
	context->device.common.close = led_device_close; 

	// 初始化控制 API  ，关联各种操作函数
	context->device.set_on= led_set_on;
	context->device.set_off= led_set_off;
	context->device.getcount_led = led_getcount;
	*device= (struct hw_device_t *)&(context->device);
    //将fd初始化为设备文件
	if((fd=open("/dev/led",O_RDWR))==-1)
	{
		LOGI("open error");
	//	exit(1);
	}else
	LOGI("open ok\n");

	return 0;
}

/*定义一个hw_module_module_methods_t结构体变量，关联入口函数*/
static struct hw_module_methods_t led_module_methods = {
open: led_device_open   //入口函数
};
/*定义stub入口*/
const struct led_module_t HAL_MODULE_INFO_SYM = {
common: {
tag: HARDWARE_MODULE_TAG,
	 version_major: 1,
	 version_minor: 0,
	 id: LED_HARDWARE_MODULE_ID, //上层通过这个id应用这个stub
	 name: "led HAL module",
	 author: "jellybean",
	 methods: &led_module_methods, //入口函数管理结构体
		}, 
		/* supporting APIs go here */
};


 
