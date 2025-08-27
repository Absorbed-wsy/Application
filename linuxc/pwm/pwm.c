#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *PWM_PATH = NULL;

// 导出 PWM 通道
int pwm_export(int channel) {
    char export_path[256];
    snprintf(export_path, sizeof(export_path), "%s/export", PWM_PATH);
    FILE *pwm_export_file = fopen(export_path, "w");
    if (!pwm_export_file) {
        perror("Failed to open PWM export");
        return 1;
    }
    fprintf(pwm_export_file, "%d", channel);
    fclose(pwm_export_file);
    return 0;
}

// 取消导出 PWM 通道
int pwm_unexport(int channel) {
    char unexport_path[256];
    snprintf(unexport_path, sizeof(unexport_path), "%s/unexport", PWM_PATH);
    FILE *pwm_unexport_file = fopen(unexport_path, "w");
    if (!pwm_unexport_file) {
        perror("Failed to open PWM unexport");
        return 1;
    }
    fprintf(pwm_unexport_file, "%d", channel);
    fclose(pwm_unexport_file);
    return 0;
}

// 设置 PWM 周期
int pwm_set_period(int channel, int period_ns) {
    char period_path[256];
    snprintf(period_path, sizeof(period_path), "%s/pwm%d/period", PWM_PATH, channel);
    
    FILE *period_file = fopen(period_path, "w");
    if (!period_file) {
        perror("Failed to open PWM period");
        return 1;
    }
    fprintf(period_file, "%d", period_ns);
    fclose(period_file);
    return 0;
}

// 设置占空比
int pwm_set_duty_cycle(int channel, int duty_cycle_ns) {
    char duty_cycle_path[256];
    snprintf(duty_cycle_path, sizeof(duty_cycle_path), "%s/pwm%d/duty_cycle", PWM_PATH, channel);

    FILE *duty_cycle_file = fopen(duty_cycle_path, "w");
    if (!duty_cycle_file) {
        perror("Failed to open PWM duty cycle");
        return 1;
    }
    fprintf(duty_cycle_file, "%d", duty_cycle_ns);
    fclose(duty_cycle_file);
    return 0;
}

// 启用/禁用 PWM
int pwm_enable(int channel, int enable) {
    char enable_path[256];
    snprintf(enable_path, sizeof(enable_path), "%s/pwm%d/enable", PWM_PATH, channel);

    FILE *enable_file = fopen(enable_path, "w");
    if (!enable_file) {
        perror("Failed to open PWM enable");
        return 1;
    }
    fprintf(enable_file, "%d", enable);
    fclose(enable_file);
    return 0;
}

int main(int argc, char **argv) {
    int channel = 0;  // 使用通道 0

    if (argc != 4) {
        printf("Usage: %s <pwm path> <period (hz)> <duty (%%)> \n", argv[0]);
        return -1;
    }

    PWM_PATH = argv[1];
    int period = 1000000000 / atoi(argv[2]);
    int duty = (period * atoi(argv[3])) / 100;

    // 导出 PWM
    if (pwm_export(channel)) {
        return -1;
    }

    // 设置周期
    if (pwm_set_period(channel, period)) {
        return -1;
    }

    // 设置占空比
    if (pwm_set_duty_cycle(channel, duty)) {
        return -1;
    }

    // 启用 PWM
    if (pwm_enable(channel, 1)) {
        return -1;
    }

    printf("PWM configured successfully:\n");
    printf("  Path: %s\n", PWM_PATH);
    printf("  Channel: %d\n", channel);
    printf("  Period: %d ns\n", period);
    printf("  Duty cycle: %d ns\n", duty);

    // 程序结束后可以取消注释以下代码来禁用和取消导出PWM
    /*
    sleep(5); // 运行5秒
    
    // 禁用 PWM
    if (pwm_enable(channel, 0)) {
        return -1;
    }

    // 取消导出 PWM
    if (pwm_unexport(channel)) {
        return -1;
    }
    */

    return 0;
}