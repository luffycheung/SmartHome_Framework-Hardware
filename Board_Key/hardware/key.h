#include <hardware/hardware.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#define KEY_HARDWARE_MODULE_ID	"key"

struct key_module_t
{
	struct hw_module_t common;
};

struct key_control_device_t
{
	struct hw_device_t common;
	int (*read_key)(struct key_control_device_t *dev, int32_t *key);
};

struct key_control_context_t
{
	struct key_control_device_t device;
};
