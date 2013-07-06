#include "i2c-max961x.h"

#include <rtems.h>
#include <rtems/libi2c.h>
#include <rtems/libio.h>

static int max961x_read_register_byte(rtems_device_minor_number minor, unsigned char reg)
{
    unsigned char data = 0;
    rtems_libi2c_send_start(minor);
    rtems_libi2c_send_addr(minor, 0);
    rtems_libi2c_write_bytes(minor, &reg, 1);
    rtems_libi2c_send_start(minor);
    rtems_libi2c_send_addr(minor, 1);
    rtems_libi2c_read_bytes(minor, &data, 1);
    rtems_libi2c_send_stop(minor);
    return data;
}

static int max961x_write_control_register(rtems_device_minor_number minor, unsigned char reg, unsigned short data)
{
    rtems_libi2c_send_start(minor);
    rtems_libi2c_send_addr(minor, 0);
    rtems_libi2c_write_bytes(minor, &reg, 1);
    rtems_libi2c_write_bytes(minor, (char*)&data, 2);
    return rtems_libi2c_send_stop(minor);
}

static int max961x_read_register(rtems_device_minor_number minor, unsigned char reg_hi,
                                 unsigned char reg_lo)
{
    return (((max961x_read_register_byte(minor, reg_hi) << 8) |
            (max961x_read_register_byte(minor, reg_lo))) >> 4)
           & 0xFFF;
}

static rtems_status_code max961x_ioctl(rtems_device_major_number major,
                                       rtems_device_minor_number minor,
                                       void *arg)
{
    rtems_libio_ioctl_args_t *ioargs = arg;
    switch (ioargs->command)
    {
    case IOCTL_MAX961X_CSA_DATA:
        ioargs->ioctl_return = max961x_read_register(minor, MAX961X_CSA_DATA_MSB,
                                                     MAX961X_CSA_DATA_LSB);
        break;
    case IOCTL_MAX961X_RSP_DATA:
        ioargs->ioctl_return = max961x_read_register(minor, MAX961X_RSP_DATA_MSB,
                                                     MAX961X_RSP_DATA_LSB);
        break;
    case IOCTL_MAX961X_TEMP_DATA:
        ioargs->ioctl_return = (max961x_read_register(minor, MAX961X_TEMP_DATA_MSB,
                                                      MAX961X_TEMP_DATA_LSB) >> 3)
                               & 0x1FF;
        break;
    case IOCTL_MAX961X_CONTROL:
        ioargs->ioctl_return = max961x_write_control_register(minor, MAX961_CONTROL_REG, (unsigned short)ioargs->buffer);
        break;
    }
    return RTEMS_SUCCESSFUL;
}

static rtems_status_code max961x_read(rtems_device_major_number major,
                                      rtems_device_minor_number minor,
                                      void *arg)
{
    rtems_libio_rw_args_t *rwarg = arg;
    return rtems_libi2c_start_read_bytes(minor, (unsigned char)rwarg->buffer, rwarg->count);
}

static rtems_status_code max961x_write(rtems_device_major_number major,
                                       rtems_device_minor_number minor,
                                       void *arg)
{
    rtems_libio_rw_args_t *rwarg = arg;
    return rtems_libi2c_start_write_bytes(minor, (unsigned char)rwarg->buffer, rwarg->count);
}

static rtems_driver_address_table max961x_ops = {
    read_entry: max961x_read,
    write_entry: max961x_write,
    control_entry: max961x_ioctl
};

static rtems_libi2c_drv_t max961x_drv_table = {
    ops: &max961x_ops,
    size: sizeof(max961x_drv_table)
};

rtems_libi2c_drv_t *i2c_max961x_driver_descriptor = &max961x_drv_table;
