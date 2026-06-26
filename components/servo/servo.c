#include "servo.h"

void configure_mcpwm_servo(mcpwm_timer_config_t* timer_struct,
        mcpwm_timer_handle_t*      timer_handle,
        mcpwm_operator_config_t*   operator_struct,
        mcpwm_oper_handle_t*       operator_handle,
        mcpwm_comparator_config_t* comparator_struct,
        mcpwm_cmpr_handle_t*       comparator_handle,
        mcpwm_generator_config_t*  generator_struct,
        mcpwm_gen_handle_t*  generator_handle) {

    // Timer configuration
    (*timer_struct) = (mcpwm_timer_config_t){
        .group_id = 0,                                // Use MCPWM Group 0
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,       // Use the default system clock source
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,// Set clock to 1 tick per microsecond
        .period_ticks = SERVO_TIMEBASE_PERIOD,        // Count up to 20,000 ticks (50Hz)
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,      // Count upwards
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(timer_struct, timer_handle));

    // Operator configuration
    (*operator_struct) = (mcpwm_operator_config_t) {
        .group_id = 0, // operator must be in the same group to the timer
    };

    ESP_ERROR_CHECK(mcpwm_new_operator(operator_struct, operator_handle));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer((*operator_handle), (*timer_handle)));

    // Comparator configuration
    (*comparator_struct) = (mcpwm_comparator_config_t){
        .flags.update_cmp_on_tez = true,
    };

    ESP_ERROR_CHECK(mcpwm_new_comparator((*operator_handle), comparator_struct, comparator_handle));

    // Generator configuration
    (*generator_struct) = (mcpwm_generator_config_t){
        .gen_gpio_num = AZIMUTH_MOTOR,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator((*operator_handle), generator_struct, generator_handle));


    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value((*comparator_handle), example_angle_to_compare(0)));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event((*generator_handle),
                                                              MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event((*generator_handle),
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, (*comparator_handle), MCPWM_GEN_ACTION_LOW)));

    ESP_ERROR_CHECK(mcpwm_timer_enable((*timer_handle)));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop((*timer_handle), MCPWM_TIMER_START_NO_STOP));
}
