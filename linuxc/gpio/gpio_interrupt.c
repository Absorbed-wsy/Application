#include <stdio.h>
#include <gpiod.h>
#include <signal.h>

static volatile int running = 1;

void signal_handler(int sig) { running = 0; }

int main() {
    struct gpiod_line *line;
    struct gpiod_line_event event;
    
    // 打开 GPIO 芯片
    struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
    
    // 获取 GPIO 线（假设偏移量 7）
    line = gpiod_chip_get_line(chip, 7);
    
    // 配置中断（上升沿触发）
    gpiod_line_request_rising_edge_events(line, "gpio-interrupt");
    
    signal(SIGINT, signal_handler);
    
    while(running) {
        // 等待中断（超时 -1 表示永久等待）
        if(gpiod_line_event_wait(line, NULL) > 0) {
            gpiod_line_event_read(line, &event);
            printf("中断时间戳: %ld.%09ld\n", 
                   event.ts.tv_sec, event.ts.tv_nsec);
        }
    }
    
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}

