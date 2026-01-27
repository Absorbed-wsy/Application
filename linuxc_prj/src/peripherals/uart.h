#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UART configuration structure
 */
typedef struct {
    int baud_rate;          /**< Baud rate */
    int data_bits;          /**< Data bits: 5,6,7,8 */
    int stop_bits;          /**< Stop bits: 1,2 */
    char parity;            /**< Parity: 'N'(none), 'O'(odd), 'E'(even) */
    bool flow_control;      /**< Flow control: true(enabled), false(disabled) */
} uart_config_t;

/**
 * @brief UART handle type
 */
typedef int uart_handle_t;

/**
 * @brief UART error codes
 */
typedef enum {
    UART_SUCCESS = 0,           /**< Operation successful */
    UART_ERROR_INVALID_PARAM,   /**< Invalid parameter */
    UART_ERROR_OPEN_FAILED,     /**< Failed to open UART */
    UART_ERROR_CONFIG_FAILED,   /**< Failed to configure UART */
    UART_ERROR_WRITE_FAILED,    /**< Failed to write data */
    UART_ERROR_READ_FAILED,     /**< Failed to read data */
    UART_ERROR_TIMEOUT,         /**< Operation timeout */
    UART_ERROR_NOT_OPENED,      /**< UART not opened */
    UART_ERROR_BUFFER_FULL      /**< Buffer full */
} uart_error_t;

/**
 * @brief Default UART configuration
 */
#define UART_CONFIG_DEFAULT { \
    .baud_rate = 9600, \
    .data_bits = 8, \
    .stop_bits = 1, \
    .parity = 'N', \
    .flow_control = false \
}

/**
 * @brief Open UART
 * @param device_path UART device path, e.g. "/dev/ttyS0"
 * @param config UART configuration
 * @return Returns UART handle on success, negative error code on failure
 */
uart_handle_t uart_open(const char *device_path, const uart_config_t *config);

/**
 * @brief Close UART
 * @param handle UART handle
 * @return Returns UART_SUCCESS on success, error code on failure
 */
uart_error_t uart_close(uart_handle_t handle);

/**
 * @brief Write data to UART
 * @param handle UART handle
 * @param data Data buffer to write
 * @param length Data length
 * @param timeout_ms Timeout in milliseconds, 0 for blocking, -1 for non-blocking
 * @return Returns number of bytes written on success, negative error code on failure
 */
int uart_write(uart_handle_t handle, const uint8_t *data, size_t length, int timeout_ms);

/**
 * @brief Read data from UART
 * @param handle UART handle
 * @param buffer Receive data buffer
 * @param buffer_size Buffer size
 * @param timeout_ms Timeout in milliseconds, 0 for blocking, -1 for non-blocking
 * @return Returns number of bytes read on success, negative error code on failure
 */
int uart_read(uart_handle_t handle, uint8_t *buffer, size_t buffer_size, int timeout_ms);

/**
 * @brief Clear UART buffer
 * @param handle UART handle
 * @return Returns UART_SUCCESS on success, error code on failure
 */
uart_error_t uart_flush(uart_handle_t handle);

/**
 * @brief Get UART readable data length
 * @param handle UART handle
 * @return Returns number of readable bytes on success, negative error code on failure
 */
int uart_get_bytes_available(uart_handle_t handle);

/**
 * @brief Set UART DTR signal
 * @param handle UART handle
 * @param state true(high level), false(low level)
 * @return Returns UART_SUCCESS on success, error code on failure
 */
uart_error_t uart_set_dtr(uart_handle_t handle, bool state);

/**
 * @brief Set UART RTS signal
 * @param handle UART handle
 * @param state true(high level), false(low level)
 * @return Returns UART_SUCCESS on success, error code on failure
 */
uart_error_t uart_set_rts(uart_handle_t handle, bool state);

/**
 * @brief Get UART CTS signal status
 * @param handle UART handle
 * @param state Returns CTS status
 * @return Returns UART_SUCCESS on success, error code on failure
 */
uart_error_t uart_get_cts(uart_handle_t handle, bool *state);

/**
 * @brief Get UART DSR signal status
 * @param handle UART handle
 * @param state Returns DSR status
 * @return Returns UART_SUCCESS on success, error code on failure
 */
uart_error_t uart_get_dsr(uart_handle_t handle, bool *state);

/**
 * @brief Get error description
 * @param error_code Error code
 * @return Error description string
 */
const char *uart_get_error_string(uart_error_t error_code);

/**
 * @brief Check if UART is opened
 * @param handle UART handle
 * @return true(opened), false(not opened)
 */
bool uart_is_opened(uart_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */