// src/algo/pid.h
#ifndef PID_H
#define PID_H

typedef struct {
    float Kp, Ki, Kd;       // 比例、积分、微分系数
    float target;           // 当前目标值
    float prev_error;       // 上一次的误差
    float prev_error2;      // 上上次的误差（用于增量式）
    float prev_actual;      // 上一次调节值
    float integral;         // 积分项累加值（位置式使用）
    float max_output;       // 最大输出限制
    float max_integral;     // 最大积分限制
    unsigned long last_time;// 上一次计算时间
    int mode;               // PID模式：0=位置式，1=增量式
} PID_Controller;

// 初始化PID控制器
void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float max_out, float max_int);

// 设置PID模式 (0=位置式, 1=增量式)
void PID_SetMode(PID_Controller *pid, int mode);

// 设置目标值
void PID_SetTarget(PID_Controller *pid, float new_target);

// PID计算函数
float PID_Calculate(PID_Controller *pid, float actual, unsigned long long current_time_us);

#endif // PID_H
