#include <stdlib.h>
#include <inttypes.h> // Required for PRIu8
#include "driver/i2c_master.h"
#include "soc/clk_tree_defs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm_prelude.h"
#include "driver/mcpwm_timer.h"
#include "esp_log.h"


static const char *TAG = "main";

// Accelerometer
#define I2C_GPIO_CLOCK 22
#define I2C_GPIO_DATA  21
#define SLAVE_ADD      0x53
#define POWER_CTL      0x2D

// Servo
#define AZIMUTH_MOTOR                18
#define ALTITUDE_MOTOR               19
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD        20000    // 20000 ticks = 20ms (50Hz)

#define SERVO_MIN_PULSEWIDTH_US      500  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US      2500  // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE             -90   // Minimum angle
#define SERVO_MAX_DEGREE             90    // Maximum angle

void configure_i2c_accelerometer(i2c_master_bus_config_t* master_conf,
        i2c_master_bus_handle_t* master_handle,
        i2c_device_config_t* slave_conf,
        i2c_master_dev_handle_t* master_slave_handle);

void find_slave_address(i2c_master_bus_handle_t* master_handle_p);

void recycle_resources(i2c_master_bus_handle_t master_bus_handle,
        i2c_master_dev_handle_t master_dev_handle);

void configure_servo();

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


    // Master specifying in slave which memory to read ! Check your data sheets !

    uint8_t receive_buffer[6] = {0}; // Receiving 6 bytes of X, Y and Z data
    uint16_t size_of_receive_buffer = 6 * sizeof(uint8_t);
    const uint8_t slave_XYZ_axis = 0x32;

    // Waking up device to start I2C transmission
    const uint8_t write_buff[2] = {POWER_CTL, 0x08};
    i2c_master_transmit(master_slave_handle, write_buff, sizeof(write_buff), -1);
    int16_t x_axis, y_axis, z_axis;

    while(1) {
        esp_err_t err_status =  i2c_master_transmit_receive(master_slave_handle,
            &slave_XYZ_axis, sizeof(slave_XYZ_axis), receive_buffer, size_of_receive_buffer, -1);

            if (err_status == ESP_OK) {
                x_axis = (int16_t)((receive_buffer[1] << 8) | receive_buffer[0]);
                y_axis = (int16_t)((receive_buffer[3] << 8) | receive_buffer[2]);
                z_axis = (int16_t)((receive_buffer[5] << 8) | receive_buffer[4]);
                printf("x -> %d\ny-> %d\nz-> %d\n", x_axis,y_axis,z_axis);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else {
                ESP_LOGI(TAG, "Could not receive data from Gmeter %s \n",esp_err_to_name(err_status));
            }
    }

    // Releasing resources
    recycle_resources(master_handle, master_slave_handle);
}



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

void recycle_resources(i2c_master_bus_handle_t master_bus_handle,
        i2c_master_dev_handle_t master_dev_handle) {
    i2c_master_bus_rm_device(master_dev_handle);
    i2c_del_master_bus(master_bus_handle);
}



// Servo motors

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void configure_servo() {
    mcpwm_timer_handle_t servo_handle = NULL;

    const mcpwm_timer_config_t timer_cfg = {
        .group_id = 0,                                // Use MCPWM Group 0
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,       // Use the default system clock source
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,// Set clock to 1 tick per microsecond
        .period_ticks = SERVO_TIMEBASE_PERIOD,        // Count up to 20,000 ticks (50Hz)
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,      // Count upwards
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_cfg, &servo_handle));


    // Operator
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t conf = {
        .group_id = 0, // operator must be in the same group to the timer
    };

    ESP_ERROR_CHECK(mcpwm_new_operator(&conf, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, servo_handle));

    // Comparator
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    mcpwm_cmpr_handle_t comparator;

    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));


    // Generator

    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = AZIMUTH_MOTOR,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));


    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
                                                              MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_timer_enable(servo_handle));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(servo_handle, MCPWM_TIMER_START_NO_STOP));


    int angle = 0;
    int step = 2;
    while (1) {
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(angle)));
        //Add delay, since it takes time for servo to rotate, usually 200ms/60degree rotation under 5V power supply
        vTaskDelay(pdMS_TO_TICKS(500));
        if ((angle + step) > 60 || (angle + step) < -60) {
            step *= -1;
        }
        angle += step;
    }
}