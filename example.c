#include <rtems.h>

#define CONFIGURE_INIT
#include <bsp.h>
rtems_task Init( rtems_task_argument argument);

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_NULL_DRIVER
#define CONFIGURE_MAXIMUM_TASKS                          8
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS                      (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS         16
#define CONFIGURE_INIT_TASK_PRIORITY                     100
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#include <rtems/confdefs.h>


#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_I2CMST

#include <drvmgr/drvmgr_confdefs.h>

#include <rtems/libi2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <i2cmst.h>

#include "config.c"
#include "i2c-max961x.h"

#define CIRCUIT_BRIDGE_RESISTANCE    0.02
#define MAX961X_DEVICE_PATH          "/dev/i2c1.max961x"

rtems_task measure_current(rtems_task_argument argument);

rtems_id task_id;
rtems_name task_name;

rtems_task Init(rtems_task_argument arg)
{
    int status = 0;
    system_init();
    printf("******** Starting MAX9611 Current Measurement ********\n");

    status = rtems_libi2c_register_drv("max961x", i2c_max961x_driver_descriptor, 0, MAX961X_ADDRESS);
    if (status < 0) {
        printf("ERROR: Could not register MAX961x driver\n");
        exit(0);
    }

    printf("driver registered successfully\n");

    task_name = rtems_build_name('C', 'U', 'R', 'M');

    rtems_task_create(task_name, 1, RTEMS_MINIMUM_STACK_SIZE * 2,
                      RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &task_id);
    rtems_task_start(task_id, measure_current, 1);
    rtems_task_delete(RTEMS_SELF);
}

int verbose_open(const char *path, int flags)
{
    int fd = open(path, flags);
    if (fd < 0)
        printf("Could not open %s: %s\n", path, strerror(errno));
    return fd;
}



float get_circuit_current(int fd)
{
    int csa_level = ioctl(fd, IOCTL_MAX961X_CSA_DATA);
    return csa_level * MAX961X_LSB_STEP_1X_GAIN / CIRCUIT_BRIDGE_RESISTANCE;
}

float get_circuit_voltage(int fd)
{
    int rsp_level = ioctl(fd, IOCTL_MAX961X_RSP_DATA);
    return rsp_level * MAX961X_VOLTAGE_LSB_STEP;
}

float get_circuit_temp(int fd)
{
    int temp_level = ioctl(fd, IOCTL_MAX961X_TEMP_DATA);
    float temp = (temp_level & 0xFF) * MAX961X_TEMP_ACCURACY;
    if (temp_level & 0x100)
        temp = -temp;
    return temp;
}

rtems_task measure_current(rtems_task_argument unused)
{
    int max961x_device;
    float current, voltage, temp;

    printf("[measure_current] Started\n");

    max961x_device = verbose_open(MAX961X_DEVICE_PATH, O_RDWR);
    if (max961x_device < 0)
        exit(1);

    ioctl(max961x_device, IOCTL_MAX961X_CONTROL, 0x0700);

    printf("\tCurrent\t\tVoltage\t\tTemp\n");

    while (true)
    {
        current = get_circuit_current(max961x_device);
        voltage = get_circuit_voltage(max961x_device);
        temp = get_circuit_temp(max961x_device);
        usleep(1000000);
        printf("\t%.3f A\t\t%.3f V\t\t%.3f %cC\n", current, voltage, temp, 248);
    }
}
