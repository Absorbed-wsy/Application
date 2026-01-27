// src/algo/pid.h
#ifndef PID_H
#define PID_H

typedef struct {
    float Kp, Ki, Kd;       // Proportional, integral, derivative coefficients
    float target;           // Current target value
    float prev_error;       // Previous error
    float prev_error2;      // Error from two steps ago (used for incremental)
    float prev_actual;      // Previous actual value
    float integral;         // Integral term accumulation (used for positional)
    float max_output;       // Maximum output limit
    float max_integral;     // Maximum integral limit
    unsigned long last_time;// Last calculation time
    int mode;               // PID mode: 0=positional, 1=incremental
} PID_Controller;

// Initialize PID controller
void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float max_out, float max_int);

// Set PID mode (0=positional, 1=incremental)
void PID_SetMode(PID_Controller *pid, int mode);

// Set target value
void PID_SetTarget(PID_Controller *pid, float new_target);

// PID calculation function
float PID_Calculate(PID_Controller *pid, float actual, unsigned long long current_time_us);

#endif // PID_H
