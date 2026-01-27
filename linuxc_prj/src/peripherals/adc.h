// src/peripherals/adc.h
#ifndef ADC_H
#define ADC_H

#include <stdio.h>

#define MAX_CHANNELS 8
#define MAX_PATH_LEN 256

typedef struct {
    FILE* raw_file;
    char channel_name[16];
} ADCChannel;

typedef struct {
    ADCChannel channels[MAX_CHANNELS];
    int channel_count;
    float scale;
    char adc_dir[MAX_PATH_LEN];
} ADCController;

int adc_init(ADCController* adc, const char* adc_dir, int num_channels, ...);
int adc_get_scale(ADCController* adc);
int adc_get_voltage(ADCController* adc, int channel_index, float* voltage);
void adc_cleanup(ADCController* adc);

float get_adc_value(int chn);

#endif // ADC_H
