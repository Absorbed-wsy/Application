// src/algo/pid.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include "pid.h"

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

void PID_SetMode(PID_Controller *pid, int mode) 
{
    pid->mode = mode;
    // 切换模式时重置历史状态
    pid->prev_error = 0;
    pid->prev_error2 = 0;
    pid->integral = 0;
}

void PID_SetTarget(PID_Controller *pid, float new_target) 
{
    pid->target = new_target;
}

float PID_Calculate(PID_Controller *pid, float actual, unsigned long long current_time_us)
{
    // 计算时间增量
    float dt = (pid->last_time > 0) ? 
              (current_time_us - pid->last_time) / 1000000.0f : 0.001f;
    dt = (dt > 0.001f) ? dt : 0.001f;
    pid->last_time = current_time_us;
    
    float error = pid->target - actual;
    float output = 0;
    
    if (pid->mode == 0) { // 位置式PID
        // 积分项处理
        pid->integral += error * dt;
        if (pid->integral > pid->max_integral) pid->integral = pid->max_integral;
        if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
        
        // 微分项（实际值微分）
        float derivative = (actual - pid->prev_actual) / dt;
        
        output = (pid->Kp * error) 
               + (pid->Ki * pid->integral)
               - (pid->Kd * derivative); 
        
        pid->prev_actual = actual;
    } 
    else { // 增量式PID
        // 增量式计算：Δu = Kp*(e(k)-e(k-1)) + Ki*e(k) + Kd*(e(k)-2e(k-1)+e(k-2))
        float delta = pid->Kp * (error - pid->prev_error)
                   + pid->Ki * error * dt
                   + pid->Kd * (error - 2 * pid->prev_error + pid->prev_error2) / dt;
        
        // 输出增量限幅
        if (delta > pid->max_output) delta = pid->max_output;
        if (delta < -pid->max_output) delta = -pid->max_output;
        
        output = delta;
        
        // 更新历史误差
        pid->prev_error2 = pid->prev_error;
    }
    
    // 更新公共状态
    pid->prev_error = error;
    
    return output;
}

