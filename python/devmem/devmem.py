#!/usr/bin/env python3
import os
import mmap
import argparse
import struct
import sys

def access_physical_memory(address, width=32, write_value=None):
    PAGE_SIZE = 4096
    # 计算页对齐的基地址和偏移量
    base_address = address & ~(PAGE_SIZE - 1)
    offset = address - base_address
    
    # 验证地址对齐
    if address % (width // 8) != 0:
        print(f"Warning: Address 0x{address:X} not aligned for {width}-bit access")
    
    try:
        # 打开 /dev/mem 设备 (读写模式)
        with open("/dev/mem", "r+b") as f:
            # 映射整个内存页 (读写权限)
            mem = mmap.mmap(f.fileno(), PAGE_SIZE, flags=mmap.MAP_SHARED, 
                            prot=mmap.PROT_READ | mmap.PROT_WRITE, 
                            offset=base_address)
            
            # 处理写入操作
            if write_value is not None:
                # 根据宽度转换写入值
                if width == 8:
                    if write_value > 0xFF:
                        raise ValueError("8-bit value must be <= 0xFF")
                    mem[offset] = write_value & 0xFF
                elif width == 16:
                    if write_value > 0xFFFF:
                        raise ValueError("16-bit value must be <= 0xFFFF")
                    mem[offset:offset+2] = struct.pack("<H", write_value)
                elif width == 32:
                    if write_value > 0xFFFFFFFF:
                        raise ValueError("32-bit value must be <= 0xFFFFFFFF")
                    mem[offset:offset+4] = struct.pack("<I", write_value)
                elif width == 64:
                    mem[offset:offset+8] = struct.pack("<Q", write_value)
                else:
                    raise ValueError("Invalid width. Supported: 8, 16, 32, 64")
                
                print(f"Write successful: 0x{address:08X} = 0x{write_value:0{width//4}X}")
            
            # 读取数据 (无论是否写入都读取)
            if width == 8:
                value = mem[offset]
                fmt_char = 'B'
                hex_width = 2
            elif width == 16:
                value = struct.unpack("<H", mem[offset:offset+2])[0]
                fmt_char = 'H'
                hex_width = 4
            elif width == 32:
                value = struct.unpack("<I", mem[offset:offset+4])[0]
                fmt_char = 'I'
                hex_width = 8
            elif width == 64:
                value = struct.unpack("<Q", mem[offset:offset+8])[0]
                fmt_char = 'Q'
                hex_width = 16
            else:
                raise ValueError("Invalid width. Supported: 8, 16, 32, 64")
            
            # 打印结果
            print(f"Physical Address: 0x{address:08X}")
            print(f"Width: {width}-bit")
            print(f"Current Value: 0x{value:0{hex_width}X} ({value})")
            
            # 以不同格式显示值
            print("\nAlternative Formats:")
            print(f"  Hex: 0x{value:0{hex_width}X}")
            print(f"  Decimal: {value}")
            print(f"  Binary: {bin(value)}")
            
            return value
            
    except Exception as e:
        print(f"Error accessing memory: {str(e)}", file=sys.stderr)
        return None

if __name__ == "__main__":
    # 命令行参数解析
    parser = argparse.ArgumentParser(
        description="Physical Memory Read/Write Tool with 64-bit support",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument("address", type=lambda x: int(x, 0), 
                        help="Physical address (e.g. 0x1000, 0xFEDC0000)")
    parser.add_argument("-w", "--width", type=int, choices=[8, 16, 32, 64], default=32,
                        help="Access width in bits")
    parser.add_argument("-s", "--set", type=lambda x: int(x, 0), metavar="VALUE",
                        help="Value to write (e.g. 0x1234, 0xDEADBEEFCAFEBABE)")
    
    args = parser.parse_args()
    
    # 检查root权限
    if os.geteuid() != 0:
        print("Error: This tool requires root privileges!", file=sys.stderr)
        exit(1)
    
    # 执行内存访问
    access_physical_memory(args.address, args.width, args.set)
