// src/algo/pid.h
#ifndef PID_H
#define PID_H


typedef struct {
    float Kp, Ki, Kd;       // 比例、积分、微分系数
    float target;           // 当前目标值
    float prev_error;       // 上一次的误差
    float prev_actual;      //上一次调节值
    float integral;         // 积分项累加值
    float max_output;       // 最大输出限制
    float max_integral;     // 最大积分限制（抗饱和）
    unsigned long last_time;    // 上一次计算时间（用于dt计算）
} PID_Controller;


void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float max_out, float max_int);
float PID_Calculate(PID_Controller *pid, float actual, unsigned long current_time_us);
void PID_SetTarget(PID_Controller *pid, float new_target);



#endif // PID_H
