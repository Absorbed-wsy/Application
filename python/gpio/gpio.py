#!/usr/bin/env python3
import gpiod
import sys

def control_gpio():
    if len(sys.argv) < 4:
        print(f"Usage: {sys.argv[0]} <gpiochip path> <gpio number> <direction: in/out> [<value: 0/1>]")
        sys.exit(1)

    chip_path = sys.argv[1]
    try:
        gpio_num = int(sys.argv[2])
        direction = sys.argv[3].lower()
        value = 0
    except ValueError:
        print("Error: GPIO number must be an integer")
        sys.exit(1)

    # 验证方向参数
    if direction not in ['in', 'out']:
        print("Error: Direction must be 'in' or 'out'")
        sys.exit(1)
    
    # 如果是输出方向，检查值参数
    if direction == 'out':
        if len(sys.argv) < 5:
            print("Error: Output direction requires a value (0 or 1)")
            sys.exit(1)
        try:
            value = int(sys.argv[4])
            if value not in [0, 1]:
                raise ValueError
        except ValueError:
            print("Error: Value must be 0 or 1")
            sys.exit(1)

    try:
        # 打开GPIO芯片
        with gpiod.Chip(chip_path) as chip:
            # 配置GPIO线
            line = chip.get_line(gpio_num)
            
            if direction == 'in':
                # 输入模式
                line.request(consumer=sys.argv[0], type=gpiod.LINE_REQ_DIR_IN)
                current_value = line.get_value()
                print(f"GPIO {gpio_num} value: {current_value}")
                
            else:  # 输出模式
                line.request(consumer=sys.argv[0], 
                            type=gpiod.LINE_REQ_DIR_OUT,
                            default_val=value)
                print(f"Set GPIO {gpio_num} to {'HIGH' if value else 'LOW'}")
                
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    control_gpio()

