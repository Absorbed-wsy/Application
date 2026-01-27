// src/algo/pid.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include "pid.h"

/**
 * @brief Initialize PID controller parameters
 * @param pid PID controller structure pointer
 * @param Kp Proportional coefficient
 * @param Ki Integral coefficient
 * @param Kd Derivative coefficient
 * @param max_out Maximum output limit
 * @param max_int Maximum integral limit
 */
void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float max_out, float max_int) 
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->target = 0;
    pid->prev_error = 0;
    pid->prev_error2 = 0;
    pid->prev_actual = 0;
    pid->integral = 0;
    pid->max_output = max_out;
    pid->max_integral = max_int;
    pid->last_time = 0;
    pid->mode = 0;
}

/**
 * @brief Set PID controller mode
 * @param pid PID controller structure pointer
 * @param mode Control mode (0: Positional PID, 1: Incremental PID)
 */
void PID_SetMode(PID_Controller *pid, int mode) 
{
    pid->mode = mode;
    // Reset historical state when switching modes
    pid->prev_error = 0;
    pid->prev_error2 = 0;
    pid->integral = 0;
}

/**
 * @brief Set PID controller target value
 * @param pid PID controller structure pointer
 * @param new_target New target setpoint
 */
void PID_SetTarget(PID_Controller *pid, float new_target) 
{
    pid->target = new_target;
}

/**
 * @brief Execute PID control calculation
 * @param pid PID controller structure pointer
 * @param actual Actual measured value
 * @param current_time_us Current timestamp (microseconds)
 * @return PID controller output value
 */
float PID_Calculate(PID_Controller *pid, float actual, unsigned long long current_time_us)
{
    // Calculate time increment
    float dt = (pid->last_time > 0) ? 
              (current_time_us - pid->last_time) / 1000000.0f : 0.001f;
    dt = (dt > 0.001f) ? dt : 0.001f;
    pid->last_time = current_time_us;
    
    float error = pid->target - actual;
    float output = 0;
    
    if (pid->mode == 0) { // Positional PID
        // Integral term processing
        pid->integral += error * dt;
        if (pid->integral > pid->max_integral) pid->integral = pid->max_integral;
        if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
        
        // Derivative term (actual value derivative)
        float derivative = (actual - pid->prev_actual) / dt;
        
        output = (pid->Kp * error) 
               + (pid->Ki * pid->integral)
               - (pid->Kd * derivative); 
        
        pid->prev_actual = actual;
    } 
    else { // Incremental PID
        // Incremental calculation: Δu = Kp*(e(k)-e(k-1)) + Ki*e(k) + Kd*(e(k)-2e(k-1)+e(k-2))
        float delta = pid->Kp * (error - pid->prev_error)
                   + pid->Ki * error * dt
                   + pid->Kd * (error - 2 * pid->prev_error + pid->prev_error2) / dt;
        
        // Output increment limiting
        if (delta > pid->max_output) delta = pid->max_output;
        if (delta < -pid->max_output) delta = -pid->max_output;
        
        output = delta;
        
        // Update historical errors
        pid->prev_error2 = pid->prev_error;
    }
    
    // Update common state
    pid->prev_error = error;
    
    return output;
}
