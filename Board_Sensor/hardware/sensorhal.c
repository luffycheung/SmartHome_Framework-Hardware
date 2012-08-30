/*
 * Copyright (C) 2010 Analog Devices Inc.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "sensors"
#define SENSORS_SERVICE_NAME "sensors"

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/input.h>

#include <cutils/sockets.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/native_handle.h>
#include <errno.h>
#include <hardware/sensors.h>

#define ID_BASE SENSORS_HANDLE_BASE
#define ID_ACCELERATION (ID_BASE + 0)
#define  ID_MAGNETIC_FIELD (ID_BASE+1)
#define  ID_ORIENTATION    (ID_BASE+2)
#define  ID_TEMPERATURE    (ID_BASE+3)//分配各种传感器ID

#define MAX_NUM_SENSORS	4            //系统中传感器最大个数
/*
 * This driver assumes the ADXL345/6 set in 13-bit full resolution mode +/-16g.
 */
/* 3.9 mg resolution */
/*
#define LSG                     (256.0f)
#define CONVERT                 (GRAVITY_EARTH / LSG)
#define CONVERT_X               CONVERT
#define CONVERT_Y               CONVERT
#define CONVERT_Z               CONVERT
*/
#define SENSORS_ACCELERATION    (1 << ID_ACCELERATION)
#define ID_A  			(0)
#define INPUT_DIR               "/dev/input"
#define SUPPORTED_SENSORS       (SENSORS_ACCELERATION)
#define EVENT_MASK_ACCEL_ALL    ((1 << ABS_X) | (1 << ABS_Y) | (1 << ABS_Z))
#define DEFAULT_THRESHOLD 100 /*
#define ACCELERATION_X		(1 << ABS_X)
#define ACCELERATION_Y		(1 << ABS_Y)
#define ACCELERATION_Z		(1 << ABS_Z)
#define SENSORS_ACCELERATION_ALL (ACCELERATION_X | ACCELERATION_Y | \
ACCELERATION_Z)
#define SEC_TO_NSEC 		1000000000LL
#define USEC_TO_NSEC		1000
#define MSEC_TO_USEC		1000
*/
/* 200 Hz */
/*
#define ADXL_MAX_SAMPLE_RATE_VAL	11
*/

//globle variables used for testing
float x=1.0, y=1.0, z=0.0;    //方向传感器三个轴的值
int count = 0;

struct sensors_control_context_t {
	struct sensors_control_device_t device;  //传感器控制结构体，提供控制接口
	int sensor_fd[2];
	uint32_t active_sensors;
};


struct sensors_data_context_t {
	struct sensors_data_device_t device;  //传感器数据结构体，提供数据读取接口
	int event_fd[2];
	sensors_data_t sensors[MAX_NUM_SENSORS];
};

/*
 * 传感器列表
 */
static const struct sensor_t device_sensor_list[] = { 
/*	{   
		.name		= "Analog Devices ADXL345/6 3-axis Accelerometer",
		.vendor		= "ADI",
		.version	= 1,
		.handle		= ID_ACCELERATION,
		.type		= SENSOR_TYPE_ACCELEROMETER,
		.maxRange	= (GRAVITY_EARTH * 16.0f),
		.resolution = (GRAVITY_EARTH * 16.0f) / 4096.0f,
		.power		= 0.145f,
		.reserved	= {}, 
	},  

	{ 
		.name       = "Farsight 3-axis Magnetic field sensor",
		.vendor     = "Farsight",
		.version    = 1,
		.handle     = ID_MAGNETIC_FIELD,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = 2000.0f,
		.resolution = 1.0f,
		.power      = 6.7f,
		.reserved   = {}
	},  
	*/
	{ 
		.name       = "Farsight Orientation sensor",
		.vendor     = "Farsight",
		.version    = 1,
		.handle     = ID_ORIENTATION,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = 360.0f,
		.resolution = 1.0f,
		.power      = 9.7f,
		.reserved   = {}
	},  
	{ 
		.name       = "Farsight ADC sensor",
		.vendor     = "Farsight",
		.version    = 1,
		.handle     = ID_MAGNETIC_FIELD,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = 80.0f,
		.resolution = 1.0f,
		.power      = 10.0f,
		.reserved   = {}
	},

	{ 
		.name       = "Farsight Temperature sensor",
		.vendor     = "Farsight",
		.version    = 1,
		.handle     = ID_TEMPERATURE,
		.type       = SENSOR_TYPE_TEMPERATURE,
		.maxRange   = 100.0f,
		.resolution = 1.0f,
		.power      = 2.0f,
		.reserved   = {}
	},  
};


/*获得传感器列表*/
static int sensors_get_list(struct sensors_module_t *module,
		struct sensor_t const **list)
{
	*list = device_sensor_list;
	return sizeof(device_sensor_list) / sizeof(device_sensor_list[0]);
}

/**关闭传感器设备，填充结构体hw_dvice_t结构体*/
static int sensors__common_close(struct hw_device_t *dev)
{
	unsigned int i;
	struct sensors_data_context_t *device_data =
		(struct sensors_data_context_t *)dev;

	if (device_data) {
		for(i = 0; i < sizeof(device_data->event_fd)/sizeof(device_data->event_fd[0]); i++)
		if (device_data->event_fd[i] > 0)
			close(device_data->event_fd[i]);

		free(device_data);
	}
	return 0;
}


/*打开传感器设备*/
static native_handle_t* control_open_data_source(struct sensors_control_device_t *dev)
{
	native_handle_t* handle;
	int fd[2];
	printf("open device:adc\n");
	if(	(fd[0] = open("/dev/mysensor", O_RDONLY)) < 0)
	{
		LOGE("adc open error",errno);
	}                                                  //获得ADC设备描述符
	if(	(fd[1] = open("/sys/bus/i2c/devices/0-0048/temp1_input", O_RDONLY)) < 0)
	{
		LOGE("temp open error",errno);
	}                                                  //获得lm75设备描述符

	handle = native_handle_create(2, 0);
	handle->data[0] = fd[0];
	handle->data[1] = fd[1];

	return handle;
}

/* here we temporarily do nothing but return 0 */
static int control_activate(struct sensors_control_context_t *dev,
		int handle, int enabled)
{
	return 0;
}

/* here we temporarily do nothing but return 0 */
static int control_set_delay(struct sensors_control_context_t *dev, int32_t ms)
{
	return 0;
}

/* here we temporarily do nothing but return 0 */
static int control_wake(struct sensors_control_context_t *dev)
{
	return 0;
}
/*关闭设备，填充控制结构体中hw_device_t结构体*/
static int control_close(struct hw_device_t *dev)
{
	struct sensors_control_context_t *device_control = (void *)dev;
	unsigned int i;
	if (device_control) {
		for(i = 0; i < sizeof(device_control->sensor_fd) / sizeof(device_control->sensor_fd[0]); i++)
			if (device_control->sensor_fd[i] > 0)
				close(device_control->sensor_fd[i]);
		free(device_control);
	}

	return 0;
}

static int sensors__data_open(struct sensors_data_context_t *dev, native_handle_t* handle)
{
	unsigned int i;
	memset(&dev->sensors, 0, sizeof(dev->sensors));

	for (i = 0; i < MAX_NUM_SENSORS; i++) {
		dev->sensors[i].vector.status = SENSOR_STATUS_ACCURACY_HIGH;
	}
	for(i = 0; i < 2; i++)
	{
		dev->event_fd[i] = dup(handle->data[i]);
	}
	/* native_handle_close(handle); */
	native_handle_delete(handle);
	return 0;

}
/*关闭设备，填充数据结构体*/
static int sensors__data_close(struct sensors_data_device_t *dev)
{
	unsigned int i;
	struct sensors_data_context_t *data_device =
		(struct sensors_data_context_t *)dev;

	printf("close data devices\n");
	for(i = 0; i < sizeof(data_device->event_fd) / sizeof(data_device->event_fd[0]); i++)
		if (data_device->event_fd[i] > 0)
			close(data_device->event_fd[i]);
	return 0;
}

/*轮询读取数据*/

static int sensors__data_poll(struct sensors_data_context_t *dev, sensors_data_t * values)
{
    int n;
	int mag;
	float temp;
	char buf[10];

	while (1) {
		if(count % 3 == 2)      // 读取ＡＤＣ值
		{
			if( read(dev->event_fd[0], &mag, sizeof(mag)) < 0)
			{
				LOGE("read adc error");
			}else{ 
                 
                
                                                                 
				dev->sensors[ID_MAGNETIC_FIELD].magnetic.v[0] =(float)mag;	
				LOGE("read adc %f\n",(float)mag);
				*values = dev->sensors[ID_MAGNETIC_FIELD];
				values->sensor = ID_MAGNETIC_FIELD;
				count++;
			}
			usleep(500000);
			return ID_MAGNETIC_FIELD;
		}
		else if(count%3 == 1)  //读取温度传感器值
		{

			memset(buf, 0 ,sizeof(buf));
			if((n =  read(dev->event_fd[1], buf, 10)) < 0)
			{
				LOGE("read temp error");
			}else{
				buf[n - 1] = '\0';
				temp =(float) (atoi(buf) / 1000);
				dev->sensors[ID_TEMPERATURE].temperature = temp;
				LOGE("read temp %f\n",temp);
				*values = dev->sensors[ID_TEMPERATURE];
				values->sensor = ID_TEMPERATURE;
				count++;
			}
			close(dev->event_fd[1]);
			dev->event_fd[1]= open("/sys/bus/i2c/devices/0-0048/temp1_input", O_RDONLY);
			usleep(500000);
			return ID_TEMPERATURE;
		}
		else if(count%3 == 0)  //读取方向传感器模拟值
		{
			LOGI("read orientation\n");
			/* fill up data of orientation */
			dev->sensors[ID_ORIENTATION].orientation.azimuth = x + 5;
			dev->sensors[ID_ORIENTATION].orientation.pitch   = y + 5;
			dev->sensors[ID_ORIENTATION].orientation.roll    = z + 5;
			*values = dev->sensors[ID_ORIENTATION];
			values->sensor = ID_ORIENTATION;
			count++;
			x += 0.0001;	y += 0.0001;	z += 0.0001;
			usleep (500000);
			return ID_ORIENTATION;
		}
	}
}

/** Open a new instance of a sensor device using name */
static int open_sensors(const struct hw_module_t *module, const char *name,
		struct hw_device_t **device)
{
	printf("open sensors");
	int status = -EINVAL;
	if (!strcmp(name, SENSORS_HARDWARE_CONTROL)) {
		LOGD("receive call from SensorService.cpp");

		struct sensors_control_context_t *dev;
		dev = malloc(sizeof(*dev));
		memset(dev, 0, sizeof(*dev));

		dev->sensor_fd[0]            = -1;
		dev->sensor_fd[1]            = -1;
		dev->device.common.tag		 = HARDWARE_DEVICE_TAG;
		dev->device.common.version	 = 0;
		dev->device.common.module	 = (struct hw_module_t *)module;
		dev->device.common.close	 = control_close;
		dev->device.open_data_source = control_open_data_source;
		dev->device.activate		 = control_activate;
		dev->device.set_delay		 = control_set_delay;
		dev->device.wake			 = control_wake;

		*device = &dev->device.common;

		status = 0;
	} else if (!strcmp(name, SENSORS_HARDWARE_DATA)) {
		LOGD("receive call from SenserManager.cpp");

		struct sensors_data_context_t *dev;
		dev = malloc(sizeof(*dev));
		memset(dev, 0, sizeof(*dev));

		dev->event_fd[0]           = -1;
		dev->event_fd[1]           = -1;
		dev->device.common.tag     = HARDWARE_DEVICE_TAG;
		dev->device.common.version = 0;
		dev->device.common.module  = (struct hw_module_t *)module;
		dev->device.common.close   = sensors__common_close;
		dev->device.data_open	   = sensors__data_open;
		dev->device.data_close	   = sensors__data_close;
		dev->device.poll		   = sensors__data_poll;

		*device = &dev->device.common;

		status = 0;
	}
	return status;
}

static struct hw_module_methods_t sensors_module_methods = {
	.open = open_sensors,
};

/*
 * The Sensors Hardware Module
 */
const struct sensors_module_t HAL_MODULE_INFO_SYM = {
	.common = {
		.tag = HARDWARE_MODULE_TAG,
		.version_major = 1,
		.version_minor = 0,
		.id = SENSORS_HARDWARE_MODULE_ID,
		.name = "Analog Devices ADXL345/6 sensor",
		.author = "OPS MH",
		.methods = &sensors_module_methods,
	},
	.get_sensors_list = sensors_get_list,
};

