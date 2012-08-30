#include <hardware/hardware.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#define RTC_HARDWARE_MODULE_ID "rtc"

struct rtc_module_t {
	struct hw_module_t common;
	/* support API for RTCServices constructor */
};

struct rtc_control_device_t {
	struct hw_device_t common;
	/* supporting control APIs go here */
	//int *(*read_tm)(struct rtc_control_device_t *dev);
	int (*read_sec)(struct rtc_control_device_t *dev);
	int (*read_min)(struct rtc_control_device_t *dev);
	int (*read_hour)(struct rtc_control_device_t *dev);
	int (*read_date)(struct rtc_control_device_t *dev);
	int (*read_mon)(struct rtc_control_device_t *dev);
	int (*read_year)(struct rtc_control_device_t *dev);
	int (*write_tm)(struct rtc_control_device_t *dev, int time[]);
};

struct rtc_control_context_t {
	struct rtc_control_device_t device;
}; 
