#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "adc.h"

/**
 * @brief Initialize ADC controller
 * @param adc ADC controller structure pointer
 * @param adc_dir ADC device path
 * @param num_channels Number of channels
 * @param ... Channel number list
 * @return Returns 0 on success, -1 on failure
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
            // Close already opened files
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
 * @brief Get ADC scale factor
 * @param adc ADC controller structure pointer
 * @return Returns 0 on success, -1 on failure
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
 * @brief Get voltage value of specified channel
 * @param adc ADC controller structure pointer
 * @param channel_index Channel index
 * @param voltage Voltage output pointer
 * @return Returns 0 on success, -1 on failure
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
 * @brief Clean up ADC controller resources
 * @param adc ADC controller structure pointer
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
 * @brief Quickly get ADC voltage value of single channel
 * @param chn Channel number
 * @return Channel voltage value
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
