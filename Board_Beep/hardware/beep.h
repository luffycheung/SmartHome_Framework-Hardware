#include <hardware/hardware.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#define BEEP_HARDWARE_MODULE_ID "beep"

struct beep_module_t {
	struct hw_module_t common;
	/* support API for BeepServices constructor */
};

struct beep_control_device_t {
	struct hw_device_t common;
	/* supporting control APIs go here */
	int (*getcount_beep)(struct beep_control_device_t *dev);
	int (*set_on)(struct beep_control_device_t *dev);
	int (*set_off)(struct beep_control_device_t *dev);
};

struct beep_control_context_t {
	struct beep_control_device_t device;
}; 
