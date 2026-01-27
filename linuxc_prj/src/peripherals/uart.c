#include "uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>

/**
 * @brief Internal UART structure
 */
typedef struct {
    int fd;                     /**< File descriptor */
    bool is_opened;             /**< Whether opened */
    uart_config_t config;       /**< UART configuration */
    char device_path[256];      /**< Device path */
} uart_internal_t;

/**
 * @brief Error code description strings
 */
static const char *error_strings[] = {
    "Operation successful",
    "Invalid parameter",
    "Failed to open UART",
    "Failed to configure UART",
    "Failed to write data",
    "Failed to read data",
    "Operation timeout",
    "UART not opened",
    "Buffer full"
};

/**
 * @brief Baud rate mapping table
 */
static const struct {
    int baud_rate;
    speed_t speed;
} baud_rate_map[] = {
    {50, B50}, {75, B75}, {110, B110}, {134, B134}, {150, B150},
    {200, B200}, {300, B300}, {600, B600}, {1200, B1200}, {1800, B1800},
    {2400, B2400}, {4800, B4800}, {9600, B9600}, {19200, B19200},
    {38400, B38400}, {57600, B57600}, {115200, B115200}, {230400, B230400},
    {460800, B460800}, {500000, B500000}, {576000, B576000}, {921600, B921600},
    {1000000, B1000000}, {1152000, B1152000}, {1500000, B1500000},
    {2000000, B2000000}, {2500000, B2500000}, {3000000, B3000000},
    {3500000, B3500000}, {4000000, B4000000}
};

/**
 * @brief Get corresponding termios baud rate
 */
static speed_t get_baud_speed(int baud_rate) {
    for (size_t i = 0; i < sizeof(baud_rate_map) / sizeof(baud_rate_map[0]); i++) {
        if (baud_rate_map[i].baud_rate == baud_rate) {
            return baud_rate_map[i].speed;
        }
    }
    return B9600; // Default baud rate
}

/**
 * @brief Configure UART parameters
 */
static uart_error_t configure_uart(int fd, const uart_config_t *config) {
    struct termios options;
    
    // Get current UART settings
    if (tcgetattr(fd, &options) == -1) {
        return UART_ERROR_CONFIG_FAILED;
    }
    
    // Set input/output baud rate
    speed_t speed = get_baud_speed(config->baud_rate);
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);
    
    // Set data bits
    options.c_cflag &= ~CSIZE;
    switch (config->data_bits) {
        case 5: options.c_cflag |= CS5; break;
        case 6: options.c_cflag |= CS6; break;
        case 7: options.c_cflag |= CS7; break;
        case 8: options.c_cflag |= CS8; break;
        default: options.c_cflag |= CS8; break;
    }
    
    // Set stop bits
    if (config->stop_bits == 2) {
        options.c_cflag |= CSTOPB;
    } else {
        options.c_cflag &= ~CSTOPB;
    }
    
    // Set parity bit
    switch (config->parity) {
        case 'N': // No parity
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'O': // Odd parity
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            options.c_iflag |= INPCK;
            break;
        case 'E': // Even parity
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        default:
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
    }
    
    // Set flow control
    if (config->flow_control) {
        options.c_cflag |= CRTSCTS;
    } else {
        options.c_cflag &= ~CRTSCTS;
    }
    
    // Set other parameters
    options.c_cflag |= (CLOCAL | CREAD); // Local connection, enable receive
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw mode
    options.c_oflag &= ~OPOST; // Raw output
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    
    // Set timeout and minimum character count
    options.c_cc[VMIN] = 0;  // Minimum read character count
    options.c_cc[VTIME] = 0; // Timeout (tenths of seconds)
    
    // Apply settings
    if (tcsetattr(fd, TCSANOW, &options) == -1) {
        return UART_ERROR_CONFIG_FAILED;
    }
    
    // Clear buffer
    tcflush(fd, TCIOFLUSH);
    
    return UART_SUCCESS;
}

/**
 * @brief Open UART
 */
uart_handle_t uart_open(const char *device_path, const uart_config_t *config) {
    if (!device_path || !config) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    // Validate parameters
    if (config->data_bits < 5 || config->data_bits > 8) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    if (config->stop_bits != 1 && config->stop_bits != 2) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    if (config->parity != 'N' && config->parity != 'O' && config->parity != 'E') {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    // Open UART device
    int fd = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        return -UART_ERROR_OPEN_FAILED;
    }
    
    // Set file descriptor to non-blocking
    fcntl(fd, F_SETFL, O_NONBLOCK);
    
    // Configure UART
    uart_error_t ret = configure_uart(fd, config);
    if (ret != UART_SUCCESS) {
        close(fd);
        return -ret;
    }
    
    return fd;
}

/**
 * @brief Close UART
 */
uart_error_t uart_close(uart_handle_t handle) {
    if (handle < 0) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    if (close(handle) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    return UART_SUCCESS;
}

/**
 * @brief Write data to UART
 */
int uart_write(uart_handle_t handle, const uint8_t *data, size_t length, int timeout_ms) {
    if (handle < 0 || !data || length == 0) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    if (timeout_ms < -1) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    fd_set write_fds;
    struct timeval timeout;
    struct timeval *timeout_ptr = NULL;
    
    // Set timeout
    if (timeout_ms >= 0) {
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        timeout_ptr = &timeout;
    }
    
    size_t total_written = 0;
    
    while (total_written < length) {
        // Wait for write ready
        FD_ZERO(&write_fds);
        FD_SET(handle, &write_fds);
        
        int select_ret = select(handle + 1, NULL, &write_fds, NULL, timeout_ptr);
        
        if (select_ret == -1) {
            return -UART_ERROR_WRITE_FAILED;
        } else if (select_ret == 0) {
            return -UART_ERROR_TIMEOUT;
        }
        
        // Write data
        ssize_t written = write(handle, data + total_written, length - total_written);
        
        if (written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return -UART_ERROR_WRITE_FAILED;
        }
        
        total_written += written;
    }
    
    return total_written;
}

/**
 * @brief Read data from UART
 */
int uart_read(uart_handle_t handle, uint8_t *buffer, size_t buffer_size, int timeout_ms) {
    if (handle < 0 || !buffer || buffer_size == 0) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    if (timeout_ms < -1) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    fd_set read_fds;
    struct timeval timeout;
    struct timeval *timeout_ptr = NULL;
    
    // Set timeout
    if (timeout_ms >= 0) {
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        timeout_ptr = &timeout;
    }
    
    // Wait for data readable
    FD_ZERO(&read_fds);
    FD_SET(handle, &read_fds);
    
    int select_ret = select(handle + 1, &read_fds, NULL, NULL, timeout_ptr);
    
    if (select_ret == -1) {
        return -UART_ERROR_READ_FAILED;
    } else if (select_ret == 0) {
        return -UART_ERROR_TIMEOUT;
    }
    
    // Read data
    ssize_t bytes_read = read(handle, buffer, buffer_size);
    
    if (bytes_read == -1) {
        return -UART_ERROR_READ_FAILED;
    }
    
    return bytes_read;
}

/**
 * @brief Clear UART buffer
 */
uart_error_t uart_flush(uart_handle_t handle) {
    if (handle < 0) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    if (tcflush(handle, TCIOFLUSH) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    return UART_SUCCESS;
}

/**
 * @brief Get UART readable data length
 */
int uart_get_bytes_available(uart_handle_t handle) {
    if (handle < 0) {
        return -UART_ERROR_INVALID_PARAM;
    }
    
    int bytes_available = 0;
    if (ioctl(handle, FIONREAD, &bytes_available) == -1) {
        return -UART_ERROR_NOT_OPENED;
    }
    
    return bytes_available;
}

/**
 * @brief Set UART DTR signal
 */
uart_error_t uart_set_dtr(uart_handle_t handle, bool state) {
    if (handle < 0) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    int flags;
    if (ioctl(handle, TIOCMGET, &flags) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    if (state) {
        flags |= TIOCM_DTR;
    } else {
        flags &= ~TIOCM_DTR;
    }
    
    if (ioctl(handle, TIOCMSET, &flags) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    return UART_SUCCESS;
}

/**
 * @brief Set UART RTS signal
 */
uart_error_t uart_set_rts(uart_handle_t handle, bool state) {
    if (handle < 0) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    int flags;
    if (ioctl(handle, TIOCMGET, &flags) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    if (state) {
        flags |= TIOCM_RTS;
    } else {
        flags &= ~TIOCM_RTS;
    }
    
    if (ioctl(handle, TIOCMSET, &flags) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    return UART_SUCCESS;
}

/**
 * @brief Get UART CTS signal status
 */
uart_error_t uart_get_cts(uart_handle_t handle, bool *state) {
    if (handle < 0 || !state) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    int flags;
    if (ioctl(handle, TIOCMGET, &flags) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    *state = (flags & TIOCM_CTS) != 0;
    
    return UART_SUCCESS;
}

/**
 * @brief Get UART DSR signal status
 */
uart_error_t uart_get_dsr(uart_handle_t handle, bool *state) {
    if (handle < 0 || !state) {
        return UART_ERROR_INVALID_PARAM;
    }
    
    int flags;
    if (ioctl(handle, TIOCMGET, &flags) == -1) {
        return UART_ERROR_NOT_OPENED;
    }
    
    *state = (flags & TIOCM_DSR) != 0;
    
    return UART_SUCCESS;
}

/**
 * @brief Get error description
 */
const char *uart_get_error_string(uart_error_t error_code) {
    if (error_code < 0 || error_code >= (int)(sizeof(error_strings) / sizeof(error_strings[0]))) {
        return "Unknown error";
    }
    
    return error_strings[error_code];
}

/**
 * @brief Check if UART is opened
 */
bool uart_is_opened(uart_handle_t handle) {
    if (handle < 0) {
        return false;
    }
    
    // Try to get file status
    return fcntl(handle, F_GETFD) != -1;
}