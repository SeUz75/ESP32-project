#ifndef SERVO_H
#define SERVO_H

#include "driver/mcpwm_prelude.h"
#include "driver/mcpwm_timer.h"
#include "utils.h"

// Servo
#define AZIMUTH_MOTOR                18
#define ALTITUDE_MOTOR               19
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD        20000    // 20000 ticks = 20ms (50Hz)

#define SERVO_MIN_PULSEWIDTH_US      500  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US      2500  // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE             -90   // Minimum angle
#define SERVO_MAX_DEGREE             90    // Maximum angle

void configure_mcpwm_servo(mcpwm_timer_config_t* timer_struct,
        mcpwm_timer_handle_t*      timer_handle,
        mcpwm_operator_config_t*   operator_struct,
        mcpwm_oper_handle_t*       operator_handle,
        mcpwm_comparator_config_t* comparator_struct,
        mcpwm_cmpr_handle_t*       comparator_handle,
        mcpwm_generator_config_t*  generator_struct,
        mcpwm_gen_handle_t*  generator_handle);

// Servo motors
    // Helper function to convert a desired servo angle in degrees
    // into a raw compare value (pulse width in microseconds).
uint32_t example_angle_to_compare(int angle) {
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

#endif // SERVO_H
