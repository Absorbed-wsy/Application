#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 串口配置结构体
 */
typedef struct {
    int baud_rate;          /**< 波特率 */
    int data_bits;          /**< 数据位: 5,6,7,8 */
    int stop_bits;          /**< 停止位: 1,2 */
    char parity;            /**< 校验位: 'N'(无), 'O'(奇), 'E'(偶) */
    bool flow_control;      /**< 流控: true(启用), false(禁用) */
} uart_config_t;

/**
 * @brief 串口句柄类型
 */
typedef int uart_handle_t;

/**
 * @brief 串口错误码
 */
typedef enum {
    UART_SUCCESS = 0,           /**< 操作成功 */
    UART_ERROR_INVALID_PARAM,   /**< 参数错误 */
    UART_ERROR_OPEN_FAILED,     /**< 打开串口失败 */
    UART_ERROR_CONFIG_FAILED,   /**< 配置串口失败 */
    UART_ERROR_WRITE_FAILED,    /**< 写入数据失败 */
    UART_ERROR_READ_FAILED,     /**< 读取数据失败 */
    UART_ERROR_TIMEOUT,         /**< 操作超时 */
    UART_ERROR_NOT_OPENED,      /**< 串口未打开 */
    UART_ERROR_BUFFER_FULL      /**< 缓冲区已满 */
} uart_error_t;

/**
 * @brief 默认串口配置
 */
#define UART_CONFIG_DEFAULT { \
    .baud_rate = 9600, \
    .data_bits = 8, \
    .stop_bits = 1, \
    .parity = 'N', \
    .flow_control = false \
}

/**
 * @brief 打开串口
 * @param device_path 串口设备路径，如 "/dev/ttyS0"
 * @param config 串口配置
 * @return 成功返回串口句柄，失败返回负数错误码
 */
uart_handle_t uart_open(const char *device_path, const uart_config_t *config);

/**
 * @brief 关闭串口
 * @param handle 串口句柄
 * @return 成功返回UART_SUCCESS，失败返回错误码
 */
uart_error_t uart_close(uart_handle_t handle);

/**
 * @brief 写入数据到串口
 * @param handle 串口句柄
 * @param data 要写入的数据缓冲区
 * @param length 数据长度
 * @param timeout_ms 超时时间(毫秒)，0表示阻塞，-1表示非阻塞
 * @return 成功返回写入的字节数，失败返回负数错误码
 */
int uart_write(uart_handle_t handle, const uint8_t *data, size_t length, int timeout_ms);

/**
 * @brief 从串口读取数据
 * @param handle 串口句柄
 * @param buffer 接收数据缓冲区
 * @param buffer_size 缓冲区大小
 * @param timeout_ms 超时时间(毫秒)，0表示阻塞，-1表示非阻塞
 * @return 成功返回读取的字节数，失败返回负数错误码
 */
int uart_read(uart_handle_t handle, uint8_t *buffer, size_t buffer_size, int timeout_ms);

/**
 * @brief 清空串口缓冲区
 * @param handle 串口句柄
 * @return 成功返回UART_SUCCESS，失败返回错误码
 */
uart_error_t uart_flush(uart_handle_t handle);

/**
 * @brief 获取串口可读数据长度
 * @param handle 串口句柄
 * @return 可读数据字节数，失败返回负数错误码
 */
int uart_get_bytes_available(uart_handle_t handle);

/**
 * @brief 设置串口DTR信号
 * @param handle 串口句柄
 * @param state true(高电平), false(低电平)
 * @return 成功返回UART_SUCCESS，失败返回错误码
 */
uart_error_t uart_set_dtr(uart_handle_t handle, bool state);

/**
 * @brief 设置串口RTS信号
 * @param handle 串口句柄
 * @param state true(高电平), false(低电平)
 * @return 成功返回UART_SUCCESS，失败返回错误码
 */
uart_error_t uart_set_rts(uart_handle_t handle, bool state);

/**
 * @brief 获取串口CTS信号状态
 * @param handle 串口句柄
 * @param state 返回CTS状态
 * @return 成功返回UART_SUCCESS，失败返回错误码
 */
uart_error_t uart_get_cts(uart_handle_t handle, bool *state);

/**
 * @brief 获取串口DSR信号状态
 * @param handle 串口句柄
 * @param state 返回DSR状态
 * @return 成功返回UART_SUCCESS，失败返回错误码
 */
uart_error_t uart_get_dsr(uart_handle_t handle, bool *state);

/**
 * @brief 获取错误描述信息
 * @param error_code 错误码
 * @return 错误描述字符串
 */
const char *uart_get_error_string(uart_error_t error_code);

/**
 * @brief 检查串口是否已打开
 * @param handle 串口句柄
 * @return true(已打开), false(未打开)
 */
bool uart_is_opened(uart_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */