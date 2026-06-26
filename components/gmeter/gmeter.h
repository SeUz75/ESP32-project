#ifndef GMETER_H
#define GMETER_H

#include "driver/i2c_master.h"
#include "soc/clk_tree_defs.h"
#include "utils.h"

// Accelerometer
#define I2C_GPIO_CLOCK 22
#define I2C_GPIO_DATA  21
#define SLAVE_ADD      0x53
#define POWER_CTL      0x2D


void configure_i2c_accelerometer(i2c_master_bus_config_t* master_conf,
        i2c_master_bus_handle_t* master_handle,
        i2c_device_config_t*     slave_conf,
        i2c_master_dev_handle_t* master_slave_handle);

void find_slave_address(i2c_master_bus_handle_t* master_handle_p);

#endif // GMETER_H
