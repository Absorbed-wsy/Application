#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <linux/limits.h>

#include "lcd.h"

#define BUFFER_SIZE 256

#if 0
void LCD_ShowString(int x, int y, int size, const char* buffer, int mode) {
    // 这是一个示例函数，具体实现取决于您的显示屏驱动库
    printf("LCD_ShowString at (%d, %d): %s\n", x, y, buffer);
}
#endif

void get_system_version(char *buffer) {
    FILE *fp = popen("lsb_release -d", "r");
    if (fp == NULL) {
        perror("popen");
        strcpy(buffer, "System Version: Unknown");
        return;
    }
	
	char s[256];
    if (fgets(s, BUFFER_SIZE, fp) != NULL) {
        // Remove newline character if it exists
        s[strcspn(s, "\n")] = '\0';
        snprintf(buffer, BUFFER_SIZE, "System Version: %s", s + 13); // Skip "Description:\t"
    }
    pclose(fp);
}

void get_kernel_version(char *buffer) {
    struct utsname uname_data;
    if (uname(&uname_data) == 0) {
        snprintf(buffer, BUFFER_SIZE, "Kernel Version: %s", uname_data.release);
    }
}

void get_system_load(char *buffer) {
    double load[3];
    if (getloadavg(load, 3) != -1) {
        snprintf(buffer, BUFFER_SIZE, "System Load: 1 min: %.2f, 5 min: %.2f, 15 min: %.2f", load[0], load[1], load[2]);
    }
}

void get_uptime(char *buffer) {
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        long uptime_minutes = sys_info.uptime / 60;
        snprintf(buffer, BUFFER_SIZE, "Uptime: %ld minutes", uptime_minutes);
    }
}

void get_ddr_usage(char *buffer) {
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == 0) {
        long total_ram = sys_info.totalram / 1024 / 1024;
        long free_ram = sys_info.freeram / 1024 / 1024;
        long used_ram = total_ram - free_ram;
        double used_percent = (double)used_ram / total_ram * 100;
        snprintf(buffer, BUFFER_SIZE, "DDR Usage: Total: %ld MB, Used: %ld MB (%.2f%%)", total_ram, used_ram, used_percent);
    }
}

void get_emmc_usage(char *buffer) {
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        long total_space = (stat.f_blocks * stat.f_frsize) / 1024 / 1024;
        long free_space = (stat.f_bfree * stat.f_frsize) / 1024 / 1024;
        long used_space = total_space - free_space;
        double used_percent = (double)used_space / total_space * 100;
        snprintf(buffer, BUFFER_SIZE, "eMMC Usage: Total: %ld MB, Used: %ld MB (%.2f%%)", total_space, used_space, used_percent);
    }
}

void get_network_interfaces(char *buffer) {
    struct ifaddrs *ifaddr, *ifa;
    char addr[INET_ADDRSTRLEN];
    buffer[0] = '\0';

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        snprintf(buffer, BUFFER_SIZE, "Network Interfaces: Error");
        return;
    }

    strcat(buffer, "Network IPs: ");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        if (strcmp(ifa->ifa_name, "lo") == 0)
            continue;
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            void *addr_ptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(family, addr_ptr, addr, sizeof(addr));
            char temp_buffer[BUFFER_SIZE];
            snprintf(temp_buffer, sizeof(temp_buffer), "%s; ", addr);
            strcat(buffer, temp_buffer);
        }
    }
    freeifaddrs(ifaddr);
    // 移除最后一个分号和空格
    if (buffer[strlen(buffer) - 2] == ';') {
        buffer[strlen(buffer) - 2] = '\0';
    }
}

void get_cpu_temperature(char *buffer) {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp == NULL) {
        perror("fopen");
        strcpy(buffer, "CPU Temperature: Unknown");
        return;
    }
    int temp;
    if (fscanf(fp, "%d", &temp) == 1) {
        snprintf(buffer, BUFFER_SIZE, "CPU Temperature: %.2f °C", temp / 1000.0);
    } else {
        strcpy(buffer, "CPU Temperature: Unknown");
    }
    fclose(fp);
}

void get_cpu_frequency(char *buffer) {
    FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    if (fp == NULL) {
        perror("fopen");
        strcpy(buffer, "CPU Frequency: Unknown");
        return;
    }
    int freq;
    if (fscanf(fp, "%d", &freq) == 1) {
        snprintf(buffer, BUFFER_SIZE, "CPU Frequency: %.2f MHz", freq / 1000.0);
    } else {
        strcpy(buffer, "CPU Frequency: Unknown");
    }
    fclose(fp);
}

void get_current_time(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    tm_info->tm_hour += 8; // Adjust to Beijing time
    mktime(tm_info); // Normalize the tm structure
	strftime(buffer, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", tm_info);
    //strftime(buffer, BUFFER_SIZE, "Current Date and Time: %Y-%m-%d %H:%M:%S", tm_info);
}

void display_system_info(void) 
{
    char buffer[BUFFER_SIZE];
    int y = 0;
	
	BACK_COLOR = BLACK;
	POINT_COLOR = GREEN;
	
    get_system_version(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    y += 26;

    get_kernel_version(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    y += 26;

    get_system_load(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    y += 26;
	
	get_cpu_temperature(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    y += 26;

    get_cpu_frequency(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
	y += 26;

    get_uptime(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    y += 26;

    get_ddr_usage(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    y += 26;

    get_emmc_usage(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    y += 26;

    get_network_interfaces(buffer);
    LCD_ShowString(0, y, 16, buffer, 0);
    

    get_current_time(buffer);
    LCD_ShowString(180, 304, 16, buffer, 0);
}

