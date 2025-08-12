# GPIO


# GPIO 控制库 libgpiod 深度解析

## 一、libgpiod 库原理

### 1. 设计背景
libgpiod 是 Linux 内核 GPIO 字符设备的用户空间库，由内核 GPIO 子系统维护者开发，用于替代过时的 sysfs GPIO 接口。它直接与 `/dev/gpiochip*` 设备文件交互，提供了更高效、更安全的 GPIO 访问方式。

### 2. 核心架构
```
+-----------------------+
|   应用程序             |
| (C/Python/其他语言)    |
+-----------------------+
         ↓
+-----------------------+
|   libgpiod 库         |
| (提供用户空间API)      |
+-----------------------+
         ↓
+-----------------------+
|   Linux 内核          |
| (gpiolib子系统)        |
+-----------------------+
         ↓
+-----------------------+
|   硬件 GPIO 控制器     |
+-----------------------+
```

### 3. 关键特性
- **字符设备接口**：通过 `/dev/gpiochip*` 设备文件访问
- **多路复用支持**：允许多个应用安全共享 GPIO
- **事件驱动**：支持边缘检测（上升沿/下降沿）
- **超时处理**：支持带超时的 GPIO 事件等待
- **去抖动**：内置硬件/软件去抖动支持
- **权限控制**：基于 Linux 文件权限管理访问

### 4. 主要数据结构
struct gpiod_chip      // 代表一个GPIO控制器
struct gpiod_line      // 代表一个GPIO引脚
struct gpiod_line_request // GPIO引脚请求配置
struct gpiod_line_config  // GPIO引脚配置
struct gpiod_line_event   // GPIO事件结构

## 二、使用流程（C语言）

### 1. 基本操作流程

打开GPIO芯片 → 获取GPIO线 → 配置GPIO → 请求GPIO → 读写操作 → 释放资源


### 2. 详细API使用

#### (1) 包含头文件

#include <gpiod.h>


#### (2) 打开GPIO芯片

struct gpiod_chip *chip = gpiod_chip_open("/dev/gpiochip0");
if (!chip) {
    perror("打开GPIO芯片失败");
    return 1;
}


#### (3) 获取GPIO线

struct gpiod_line *line = gpiod_chip_get_line(chip, 17); // 获取GPIO17
if (!line) {
    perror("获取GPIO线失败");
    gpiod_chip_close(chip);
    return 1;
}


#### (4) 配置并请求GPIO
**输入模式配置：**

// 创建配置
struct gpiod_line_request_config input_cfg = {
    .consumer = "myapp",
    .request_type = GPIOD_LINE_REQUEST_DIRECTION_INPUT,
};

// 请求GPIO
if (gpiod_line_request(line, &input_cfg, 0) < 0) {
    perror("请求GPIO失败");
    gpiod_chip_close(chip);
    return 1;
}


**输出模式配置：**

// 创建配置
struct gpiod_line_request_config output_cfg = {
    .consumer = "myapp",
    .request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT,
};

// 请求GPIO并设置初始值
if (gpiod_line_request(line, &output_cfg, 1) < 0) { // 初始值为高电平
    perror("请求GPIO失败");
    gpiod_chip_close(chip);
    return 1;
}


#### (5) GPIO操作
**读取值：**

int value = gpiod_line_get_value(line);
if (value < 0) {
    perror("读取GPIO值失败");
} else {
    printf("GPIO值: %d\n", value);
}


**设置值：**

if (gpiod_line_set_value(line, 1) < 0) { // 设置为高电平
    perror("设置GPIO值失败");
}


#### (6) 事件检测（可选）

// 配置事件检测
struct gpiod_line_request_config event_cfg = {
    .consumer = "myapp",
    .request_type = GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES, // 双边沿触发
};

// 请求GPIO
if (gpiod_line_request(line, &event_cfg, 0) < 0) {
    perror("事件请求失败");
    return 1;
}

// 等待事件（带超时）
struct gpiod_line_event event;
if (gpiod_line_event_wait(line, &timeout) > 0) {
    if (gpiod_line_event_read(line, &event) == 0) {
        printf("事件类型: %s\n", 
               event.event_type == GPIOD_LINE_EVENT_RISING_EDGE ? 
               "上升沿" : "下降沿");
    }
}


#### (7) 释放资源

gpiod_line_release(line);  // 释放GPIO线
gpiod_chip_close(chip);    // 关闭GPIO芯片


### 4. 编译与运行

# 安装依赖
sudo apt install libgpiod-dev

# 编译
gcc -o gpio_control gpio_control.c -lgpiod

# 运行示例
sudo ./gpio_control /dev/gpiochip0 23 in    # 读取GPIO23
sudo ./gpio_control /dev/gpiochip0 17 out 1 # 设置GPIO17为高电平```

## 三、Python 使用 gpiod 库

### 1. 安装 Python 绑定

sudo apt install python3-libgpiod
# 或
pip install gpiod


### 2. Python 使用流程

import gpiod

def control_gpio(chip_path, gpio_num, direction, value=None):
    try:
        # 1. 打开芯片
        with gpiod.Chip(chip_path) as chip:
            # 2. 获取GPIO线
            line = chip.get_line(gpio_num)
            
            # 3. 配置并请求GPIO
            if direction == "in":
                # 输入模式
                line.request(consumer="py-gpio", type=gpiod.LINE_REQ_DIR_IN)
                # 4. 读取值
                val = line.get_value()
                print(f"GPIO {gpio_num} 值: {val}")
                
            elif direction == "out":
                # 输出模式
                if value is None:
                    raise ValueError("输出模式需要值参数")
                line.request(consumer="py-gpio", 
                            type=gpiod.LINE_REQ_DIR_OUT,
                            default_val=value)
                # 4. 设置值
                print(f"GPIO {gpio_num} 设置为: {'高电平' if value else '低电平'}")
                
    except Exception as e:
        print(f"错误: {str(e)}")
        return 1
    return 0

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 4:
        print(f"用法: {sys.argv[0]} <gpiochip路径> <GPIO号> <方向: in/out> [<值: 0/1>]")
        sys.exit(1)
    
    chip_path = sys.argv[1]
    gpio_num = int(sys.argv[2])
    direction = sys.argv[3].lower()
    value = int(sys.argv[4]) if len(sys.argv) > 4 else None
    
    sys.exit(control_gpio(chip_path, gpio_num, direction, value))


## 四、最佳实践与注意事项

1. **权限管理**：
   - 推荐使用 udev 规则设置设备权限

   # /etc/udev/rules.d/99-gpiod.rules
   SUBSYSTEM=="gpio", KERNEL=="gpiochip*", GROUP="gpio", MODE="0660"

   - 添加用户到 gpio 组：`sudo usermod -aG gpio $USER`

2. **资源释放**：
   - 始终确保释放请求的GPIO线
   - 使用 try-finally 或 RAII 模式确保资源释放

3. **错误处理**：
   - 检查所有可能失败的函数调用
   - 处理 EACCES (权限不足)、EBUSY (GPIO已被占用) 等错误

4. **性能优化**：
   - 批量操作多个GPIO时使用多线请求
   - 事件驱动代替轮询提高效率

5. **去抖动处理**：

   struct gpiod_line_request_config cfg = {
       .consumer = "myapp",
       .request_type = GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES,
       .flags = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP |
                GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW,
   };
   ```

## 五、调试工具

1. **命令行工具**：

   # 查看可用GPIO芯片
   gpiodetect
   
   # 查看GPIO芯片信息
   gpioinfo /dev/gpiochip0
   
   # 设置GPIO值
   gpioset /dev/gpiochip0 17=1
   
   # 读取GPIO值
   gpioget /dev/gpiochip0 17
   
   # 监控GPIO事件
   gpiomon /dev/gpiochip0 17


2. **系统调试**：
   # 查看GPIO状态
   cat /sys/kernel/debug/gpio
   
   # 查看GPIO使用情况
   lsof /dev/gpiochip*


libgpiod 提供了现代、安全的GPIO访问方式，适用于各种嵌入式Linux平台。通过合理使用其API，可以构建高效可靠的GPIO控制应用。
