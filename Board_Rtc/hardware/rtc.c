#define LOG_TAG "RtcStub"
#include <hardware/hardware.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include "rtc.h"

#define READ_TIME _IO('Z', 0)
#define SET_TIME _IO('Z', 1)

struct rtc_tm 
{
	int sec;
	int min;
	int hour;
	int date;
	int mon;
	int year;
} tm;

int timeArray[5];

int fd;

static int rtc_device_close(struct hw_device_t* device)
{
	struct rtc_control_context_t* ctx = (struct rtc_control_context_t*)device;
	if (ctx) {
		free(ctx);
	}
	close(fd);
	return 0; 
}
/*
static int *rtc_read_tm(struct rtc_control_device_t *dev)
{    
	//FIXME: do system call to control the getting of time
	LOGI("Get time");
	ioctl(fd, READ_TIME, &tm);
	timeArray[0] = tm.sec;
	timeArray[1] = tm.min;
	timeArray[2] = tm.hour;
	timeArray[3] = tm.date;
	timeArray[4] = tm.mon;
	timeArray[5] = tm.year;
	return timeArray;
} 
*/

static int rtc_read_sec(struct rtc_control_device_t *dev)
{    
	//FIXME: do system call to control the getting of time
	LOGI("Get time sec");
	ioctl(fd, READ_TIME, &tm);
	return tm.sec;
}

static int rtc_read_min(struct rtc_control_device_t *dev)
{    
	//FIXME: do system call to control the getting of time
	LOGI("Get time min");
	ioctl(fd, READ_TIME, &tm);
	return tm.min;
}
static int rtc_read_hour(struct rtc_control_device_t *dev)
{    
	//FIXME: do system call to control the getting of time
	LOGI("Get time hour");
	ioctl(fd, READ_TIME, &tm);
	return tm.hour;
}
static int rtc_read_date(struct rtc_control_device_t *dev)
{    
	//FIXME: do system call to control the getting of time
	LOGI("Get time hour");
	ioctl(fd, READ_TIME, &tm);
	return tm.date;
}
static int rtc_read_mon(struct rtc_control_device_t *dev)
{    
	//FIXME: do system call to control the getting of time
	LOGI("Get time hour");
	ioctl(fd, READ_TIME, &tm);
	return tm.mon;
}
static int rtc_read_year(struct rtc_control_device_t *dev)
{    
	//FIXME: do system call to control the getting of time
	LOGI("Get time hour");
	ioctl(fd, READ_TIME, &tm);
	return tm.year;
}
static int rtc_write_tm(struct rtc_control_device_t *dev, int timeArray[])
{
	//FIXME: do system call to control the setting of time
	LOGI("set time");
	tm.year = timeArray[0];
	tm.mon = timeArray[1];
	tm.date = timeArray[2];
	tm.hour = timeArray[3];
	tm.min = timeArray[4];
	tm.sec = timeArray[5];
	LOGI("rtc_stub %02x:%02x:%02x %02x:%02x:%02x", tm.year, tm.mon, tm.date, tm.hour, tm.min, tm.sec);
	ioctl(fd, SET_TIME, &tm);
	return 0;
}

static int rtc_device_open(const struct hw_module_t* module, const char* name,
	struct hw_device_t** device)
{
	struct rtc_control_context_t *context;
	LOGD("rtc_device_open");
	context = (struct rtc_control_context_t *)malloc(sizeof(*context));
	memset(context, 0, sizeof(*context)); 

	//HAL must init property
	context->device.common.tag = HARDWARE_DEVICE_TAG;
	context->device.common.version = 0;
	context->device.common.module = module;
	context->device.common.close = rtc_device_close; 

	// 初始化控制 API 
	//context->device.read_tm= rtc_read_tm;
	context->device.read_sec= rtc_read_sec;
	context->device.read_min= rtc_read_min;
	context->device.read_hour= rtc_read_hour;
	context->device.read_date= rtc_read_date;
	context->device.read_mon= rtc_read_mon;
	context->device.read_year= rtc_read_year;
	context->device.write_tm= rtc_write_tm;
	*device= (struct hw_device_t *)&(context->device);

	if((fd=open("/dev/myrtc",O_RDWR))==-1)
	{
		LOGI("open error");
	//	exit(1);
	}else
	LOGI("open ok\n");

	return 0;
}


static struct hw_module_methods_t rtc_module_methods = {
open: rtc_device_open  
};

const struct rtc_module_t HAL_MODULE_INFO_SYM = {
common: {
tag: HARDWARE_MODULE_TAG,
	 version_major: 1,
	 version_minor: 0,
	 id: RTC_HARDWARE_MODULE_ID,
	 name: "rtc HAL module",
	 author: "jellybean",
	 methods: &rtc_module_methods,
		}, 
		/* supporting APIs go here */
};



