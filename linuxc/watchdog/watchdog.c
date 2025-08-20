#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

int main() {
    int fd;
    int timeout = 20;  // 看门狗超时时间（秒）
    int feed_interval = 10; // 喂狗间隔（秒）

    // 1. 打开看门狗设备
    fd = open("/dev/watchdog", O_RDWR);
    if (fd == -1) {
        perror("无法打开看门狗设备");
        exit(EXIT_FAILURE);
    }

    // 2. 设置超时时间（可选）
    if (ioctl(fd, WDIOC_SETTIMEOUT, &timeout) != 0) {
        perror("无法设置超时时间");
        // 注意：某些硬件可能不支持此操作
    }

    // 3. 获取实际超时时间（验证设置）
    int actual_timeout;
    if (ioctl(fd, WDIOC_GETTIMEOUT, &actual_timeout) == 0) {
        printf("看门狗超时时间设置为：%d秒\n", actual_timeout);
    }

    printf("看门狗已启动 按Ctrl+C停止喂狗触发系统重启...\n");

    // 4. 主循环定期喂狗
    while (1) {
        // 写入任意字符喂狗（保持系统活跃）
        write(fd, "\0", 1);
        fflush(stdout);
        
        // 可选：发送保活指令（等效于write）
        // ioctl(fd, WDIOC_KEEPALIVE, 0);
        
        printf("[%ld] 喂狗成功\n", time(NULL));
        sleep(feed_interval);  // 喂狗间隔需小于超时时间
    }

    // 5. 关闭设备（正常情况下不会执行到这里）
    close(fd);
    return 0;
}

//正常关闭程序
//int flags = WDIOS_DISABLECARD;
//ioctl(fd, WDIOC_SETOPTIONS, &flags);
//close(fd);

