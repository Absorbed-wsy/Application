// src/algo/pid.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include "pid.h"


// 初始化PID控制器
void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float max_out, float max_int) 
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->target = 0;
    pid->prev_error = 0;
    pid->integral = 0;
    pid->max_output = max_out;
    pid->max_integral = max_int;
}

// 更新目标值
void PID_SetTarget(PID_Controller *pid, float new_target) 
{
    pid->target = new_target;
}

// PID计算函数（需传入当前实际值和当前时间戳）
float PID_Calculate(PID_Controller *pid, float actual, unsigned long current_time_us)
{
    // 计算时间增量(dt)
    float dt = (current_time_us - pid->last_time) / 1000000.0f;
    dt = (dt > 0.001f) ? dt : 0.001f; // 防止除零错误
    pid->last_time = current_time_us;
    
    // 计算当前误差
    float error = pid->target - actual;
    
    // 积分项处理（带限幅和抗饱和）
    pid->integral += error * dt;
    // 积分限幅
    if (pid->integral > pid->max_integral) pid->integral = pid->max_integral;
    if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
    
    // 微分项（主要使用实际值微分，减少设定值突变带来的冲击）
    float derivative = (actual - pid->prev_actual) / dt;
    
    // 保存当前状态
    pid->prev_error = error;
    pid->prev_actual = actual;
    
    // PID输出计算
    float output = (pid->Kp * error) 
                 + (pid->Ki * pid->integral)
                 - (pid->Kd * derivative);  // 注意微分项符号
    
    // 输出限幅
    if (output > pid->max_output) output = pid->max_output;
    if (output < -pid->max_output) output = -pid->max_output;
    
    return output;
}
