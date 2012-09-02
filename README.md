SmartHome_Framework-Hardware
============================

JNI层和HAL层代码分析
1、HAL层：
HAL位于Linux Kernel与Libraries和Android Runtime之间。也就是说，HAL是底层硬件设备驱动程序提供给Application Framework
（也就是在应用层开发应用所使用的Andriod API)的一个接口层，它将直接和底层的设备驱动程序挂接。因此，当我们需要将Android
移植到其他硬件上时，或者给Android系统添加新的硬件支持时，都需要对Android的HAL层进行移植或者实现。
Android HAL 实现位于Android源码中的如下路径：
hardware/libhardware_legacy/
hardware/libhardware/
hardware/ril/Radio Interface Layer

1.1 Android HAL 的实现（stub）
HAL其实就是一个硬件抽象层的框架，其硬件设备的具体操作由对应的stub进行间接地回调。位于如下两个文件中：
hardware/libhardware/include/hardware/hardware.h
hardware/libhardware/hardware.c
hardware.h中定义了以下三个非常重要的结构体：
struct hw_module_t;
struct hw_module_methods_t;
struct hw_device_t;

结构体hw_device_t表示硬件设备，存储了各种硬件设备的公共属性和方法
—————————————————————————————————————————————————————————————
/**
 * Every device data structure must begin with hw_device_t
 * followed by module specific public methods and attributes.
 */
typedef struct hw_device_t {
    /** tag must be initialized to HARDWARE_DEVICE_TAG */
    uint32_t tag;

    /** version number for hw_device_t */
    uint32_t version;

    /** reference to the module this device belongs to */
    struct hw_module_t* module;

    /** padding reserved for future use */
    uint32_t reserved[12];

    /** Close this device */
    int (*close)(struct hw_device_t* device);

} hw_device_t;
————————————————————————————————————————————————————
如果需要移植或者添加新硬件，那么都需要使用该结构体进行注册，其中的tag必须初始化。
结构体hw_module_t在进行加载的时候用于判断属于哪一个module，其代码定义如下：
————————————————————————————————————————————————————————————
/**
 * Every hardware module must have a data structure named HAL_MODULE_INFO_SYM
 * and the fields of this data structure must begin with hw_module_t
 * followed by module specific information.
 */
typedef struct hw_module_t {
    /** tag must be initialized to HARDWARE_MODULE_TAG */
    uint32_t tag;

    /** major version number for the module */
    uint16_t version_major;

    /** minor version number of the module */
    uint16_t version_minor;

    /** Identifier of module */
    const char *id;

    /** Name of this module */
    const char *name;

    /** Author/owner/implementor of the module */
    const char *author;

    /** Modules methods */
    struct hw_module_methods_t* methods;

    /** module's dso */
    void* dso;

    /** padding to 128 bytes, reserved for future use */
    uint32_t reserved[32-7];

} hw_module_t;
——————————————————————————————————————————————————————————————
结构体hw_module_methods_t用于定义操作设备的方法operations,这里只定义了一定打开设备的方法open，其代码定义如下：
typedef struct hw_module_methods_t {
    /** Open a specific device */
    int (*open)(const struct hw_module_t* module, const char* id,
            struct hw_device_t** device);

} hw_module_methods_t;
————————————————————————————————————————————————————————————
如果要执行打开设备等操作可以使用“module->methods->open(module,
  	LED_HARDWARE_MODULE_ID, (struct hw_device_t**)device);”
该方法在framework的JNI层代码中：
/** helper APIs */
static inline int led_control_open(const struct hw_module_t* module,
  struct led_control_device_t** device) {
	LOGI("led_control_open");
	return module->methods->open(module,
		LED_HARDWARE_MODULE_ID, (struct hw_device_t**)device);
	//这个过程非常重要JNI通过该ID找到对应的Stub
}
————————————————————————————————————————————————————————
下面继续分析如何获得HAL stub
当加载module时，可以调用hardware.c中的hw_get_module函数获得HAL，其实现代码如下：
int hw_get_module(const char *id, const struct hw_module_t **module) 
{
    int status;
    int i;
    const struct hw_module_t *hmi = NULL;
    char prop[PATH_MAX];
    char path[PATH_MAX];

    /*
     * Here we rely on the fact that calling dlopen multiple times on
     * the same .so will simply increment a refcount (and not load
     * a new copy of the library).
     * We also assume that dlopen() is thread-safe.
     */

    /* Loop through the configuration variants looking for a module */
    for (i=0 ; i<HAL_VARIANT_KEYS_COUNT+1 ; i++) {
        if (i < HAL_VARIANT_KEYS_COUNT) {
            if (property_get(variant_keys[i], prop, NULL) == 0) {
                continue;
            }
            snprintf(path, sizeof(path), "%s/%s.%s.so",
                    HAL_LIBRARY_PATH, id, prop);
        } else {
            snprintf(path, sizeof(path), "%s/%s.default.so",
                    HAL_LIBRARY_PATH, id);
        }
        if (access(path, R_OK)) {
            continue;
        }
        /* we found a library matching this id/variant */
        break;
    }

    status = -ENOENT;
    if (i < HAL_VARIANT_KEYS_COUNT+1) {
        /* load the module, if this fails, we're doomed, and we should not try
         * to load a different variant. */
        status = load(id, path, module);
    }

    return status;
}
————————————————————————————————————————————————————
在hw_get_module函数中，Android系统首先在系统属性中查找硬件定义，然后通过改函数的参数id和查找到的模块的路径（path）加载
相应硬件HAL的特定模块so库文件。如果在系统属性中未定义硬件属性，则使用默认硬件HAL对于模块的so库文件。其中property_get
函数将根据定义的硬件属性配置查找对应的模块及其路径，然后调用load函数加载。
。。。
加载了so库文件之后，就可以操作具体的硬件设备。对于不同的硬件设备，Android提供了一些实现接口，它们位于
“hardware/libhardware/include/hardware"中，比如gps等。如果需要自定义HAL，那么也需要遵守这些已提供的接口。

