// TODO !! DEALLOCATE THE OBJECTS IN THE SAME TRANSLATION UNIT THEY HAVE BEEN ALLOCATED !!!

#include "utils.h"
#include "gmeter.h"
#include "servo.h"


void app_main(void) {
    // Gmeter configuration and connection master bus to device
    i2c_master_bus_config_t master_conf = {0};
    i2c_master_bus_handle_t master_handle = NULL;
    i2c_device_config_t slave_conf = {0};
    i2c_master_dev_handle_t master_slave_handle = NULL;
    configure_i2c_accelerometer(&master_conf, &master_handle, &slave_conf, &master_slave_handle);


    // Servo motor configuration
    mcpwm_timer_config_t timer_struct = {0};
    mcpwm_timer_handle_t timer_handle = NULL;

    mcpwm_operator_config_t operator_struct = {0};
    mcpwm_oper_handle_t operator_handle = NULL;

    mcpwm_comparator_config_t comparator_struct = {0};
    mcpwm_cmpr_handle_t comparator_handle = NULL;

    mcpwm_generator_config_t generator_struct = {0};
    mcpwm_gen_handle_t generator_handle = NULL;

    configure_mcpwm_servo(&timer_struct, &timer_handle,
                          &operator_struct, &operator_handle,
                          &comparator_struct, &comparator_handle,
                          &generator_struct, &generator_handle);


    // Master specifying in slave which memory to read ! Check your data sheets !

    uint8_t receive_buffer[6] = {0}; // Receiving 6 bytes of X, Y and Z data
    uint16_t size_of_receive_buffer = 6 * sizeof(uint8_t);
    const uint8_t slave_XYZ_axis = 0x32;

    // Waking up device to start I2C transmission
    const uint8_t write_buff[2] = {POWER_CTL, 0x08};
    i2c_master_transmit(master_slave_handle, write_buff, sizeof(write_buff), -1);
    int16_t x_axis, y_axis, z_axis;
    float x_float, y_float, z_float, angle_rad;
    int target_angle;

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator_handle, example_angle_to_compare(0)));

    while(1) {
        esp_err_t err_status =  i2c_master_transmit_receive(master_slave_handle,
            &slave_XYZ_axis, sizeof(slave_XYZ_axis), receive_buffer, size_of_receive_buffer, -1);

            if (err_status == ESP_OK) {
                x_axis = (int16_t)((receive_buffer[1] << 8) | receive_buffer[0]);
                y_axis = (int16_t)((receive_buffer[3] << 8) | receive_buffer[2]);
                z_axis = (int16_t)((receive_buffer[5] << 8) | receive_buffer[4]);

                x_float = (float)(x_axis);
                y_float = (float)(y_axis);
                z_float = (float)(z_axis);
                angle_rad = atan2f(x_float, sqrtf((y_float * y_float) + (z_float * z_float)));
                target_angle = (int)(angle_rad * 180.0f / M_PI); // M_PI is defined in math.h

                if (target_angle < SERVO_MIN_DEGREE) target_angle = SERVO_MIN_DEGREE;
                if (target_angle > SERVO_MAX_DEGREE) target_angle = SERVO_MAX_DEGREE;

                ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator_handle, example_angle_to_compare(target_angle)));
                printf("Raw X: %d | Calculated Angle: %d°\n", x_axis, target_angle);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            else {
                ESP_LOGI(TAG, "Could not receive data from Gmeter %s \n",esp_err_to_name(err_status));
            }
    }

    // Releasing resources
    // recycle_resources(master_handle, master_slave_handle);
}