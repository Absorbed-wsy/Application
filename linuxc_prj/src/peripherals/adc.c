#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "adc.h"

/**
 * @brief 初始化ADC控制器
 * @param adc ADC控制器结构体指针
 * @param adc_dir ADC设备路径
 * @param num_channels 通道数量
 * @param ... 通道编号列表
 * @return 成功返回0，失败返回-1
 */
int adc_init(ADCController* adc, const char* adc_dir, int num_channels, ...)
{
    strncpy(adc->adc_dir, adc_dir, MAX_PATH_LEN-1);
    adc->channel_count = num_channels;

    va_list args;
    va_start(args, num_channels);
    
    for (int i = 0; i < num_channels; i++) {
        int ch_num = va_arg(args, int);
        ADCChannel* ch = &adc->channels[i];
        
        snprintf(ch->channel_name, sizeof(ch->channel_name), "in_voltage%d", ch_num);
        char raw_path[MAX_PATH_LEN];
        snprintf(raw_path, sizeof(raw_path), "%s/%s_raw", adc_dir, ch->channel_name);
        
        ch->raw_file = fopen(raw_path, "r");
        if (!ch->raw_file) {
            perror("Failed to open channel file");
            // 关闭已打开的文件
            for (int j = 0; j < i; j++) {
                fclose(adc->channels[j].raw_file);
            }
            va_end(args);
            return -1;
        }
    }
    va_end(args);
    return 0;
}

/**
 * @brief 获取ADC比例因子
 * @param adc ADC控制器结构体指针
 * @return 成功返回0，失败返回-1
 */
int adc_get_scale(ADCController* adc)
{
    char scale_path[300];
    snprintf(scale_path, sizeof(scale_path), "%s/in_voltage_scale", adc->adc_dir);
    
    FILE* scale_file = fopen(scale_path, "r");
    if (!scale_file) return -1;
    
    char buffer[32];
    if (!fgets(buffer, sizeof(buffer), scale_file)) {
        fclose(scale_file);
        return -1;
    }
    
    adc->scale = strtof(buffer, NULL);
    fclose(scale_file);
    return 0;
}

/**
 * @brief 获取指定通道电压值
 * @param adc ADC控制器结构体指针
 * @param channel_index 通道索引
 * @param voltage 电压输出指针
 * @return 成功返回0，失败返回-1
 */
int adc_get_voltage(ADCController* adc, int channel_index, float* voltage)
{
    if (channel_index < 0 || channel_index >= adc->channel_count) {
        fprintf(stderr, "Invalid channel index\n");
        return -1;
    }

    ADCChannel* ch = &adc->channels[channel_index];
    rewind(ch->raw_file);
    fflush(ch->raw_file);
    
    char buffer[32];
    if (!fgets(buffer, sizeof(buffer), ch->raw_file)) {
        return -1;
    }

    int raw_value = atoi(buffer);
    *voltage = (raw_value * adc->scale) / 1000.0f;
    return 0;
}

/**
 * @brief 清理ADC控制器资源
 * @param adc ADC控制器结构体指针
 */
void adc_cleanup(ADCController* adc)
{
    for (int i = 0; i < adc->channel_count; i++) {
        if (adc->channels[i].raw_file) {
            fclose(adc->channels[i].raw_file);
        }
    }
    adc->channel_count = 0;
}

/**
 * @brief 快速获取单个通道的ADC电压值
 * @param chn 通道编号
 * @return 通道电压值
 */
float get_adc_value(int chn)
{
    ADCController ADC;
    float voltage;
    adc_init(&ADC, "/sys/bus/iio/devices/iio:device0", 1, chn);
    adc_get_scale(&ADC);
    adc_get_voltage(&ADC, 0, &voltage);
    adc_cleanup(&ADC);

    return voltage;
}
