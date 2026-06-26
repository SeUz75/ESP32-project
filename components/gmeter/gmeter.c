#include "gmeter.h"


void configure_i2c_accelerometer(i2c_master_bus_config_t* master_conf,
        i2c_master_bus_handle_t* master_handle,
        i2c_device_config_t* slave_conf,
        i2c_master_dev_handle_t* master_slave_handle) {

    // Configuring master conf
    (*master_conf) = (i2c_master_bus_config_t){
        .i2c_port   = I2C_NUM_0,                // Use the first hardware I2C port
        .clk_source = I2C_CLK_SRC_DEFAULT,      // Let the system choose the best clock source
        .scl_io_num = I2C_GPIO_CLOCK,           // Your SCL Pin
        .sda_io_num = I2C_GPIO_DATA,            // Your SDA Pin
        .flags.enable_internal_pullup = 1,      // Enable weak internal pull-ups
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(master_conf, master_handle));

    // Configuring slave
    (*slave_conf) = (i2c_device_config_t) {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = SLAVE_ADD,
        .scl_speed_hz    = 10000,
    };

    // Adding slave to Master bus so it can communicat with master

    ESP_ERROR_CHECK(i2c_master_bus_add_device((*master_handle),
                slave_conf, master_slave_handle));
}


void find_slave_address(i2c_master_bus_handle_t* master_handle_p) {
    esp_err_t error_status = ESP_OK;

    printf("Master probe...\n");

    for (int i = 0; i < 127; i++) {
        error_status = i2c_master_probe((*master_handle_p), i, 200);

        if (error_status != ESP_OK) {
            printf("ADDRESS does not exist... 0x%x \n",error_status);
        } else {
            printf("Address has been found -> 0x%x at %d \n", i, i);
        }
    }
}
