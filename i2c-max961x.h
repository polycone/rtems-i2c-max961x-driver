#ifndef _I2C_MAX961X_H_
#define _I2C_MAX961X_H_

#include <rtems/libi2c.h>

#define IOCTL_MAX961X_CSA_DATA        1
#define MAX961X_CSA_DATA_MSB          0x00
#define MAX961X_CSA_DATA_LSB          0x01

#define IOCTL_MAX961X_RSP_DATA        2
#define MAX961X_RSP_DATA_MSB          0x02
#define MAX961X_RSP_DATA_LSB          0x03

#define IOCTL_MAX961X_TEMP_DATA       3
#define MAX961X_TEMP_DATA_MSB         0x08
#define MAX961X_TEMP_DATA_LSB         0x09

#define IOCTL_MAX961X_CONTROL         4
#define MAX961_CONTROL_REG            0x0A

#define MAX961X_ADDRESS               0x70

#define MAX961X_LSB_STEP_1X_GAIN      0.0001075

#define MAX961X_TEMP_ACCURACY         0.48

#define MAX961X_VOLTAGE_LSB_STEP      0.014

extern rtems_libi2c_drv_t *i2c_max961x_driver_descriptor;

#endif /* _I2C_MAX961X_H_ */
