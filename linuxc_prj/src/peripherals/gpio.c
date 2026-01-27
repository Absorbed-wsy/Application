// src/peripherals/gpio.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <gpiod.h>
#include <signal.h>
#include <pthread.h>
#include "gpio.h"

// GPIO interrupt related global variables
static struct gpiod_chip *interrupt_chip = NULL;
static struct gpiod_line_request *interrupt_requests[MAX_INTERRUPT_NUM] = {NULL};
static struct gpiod_edge_event_buffer *interrupt_buffers[MAX_INTERRUPT_NUM] = {NULL};
static gpio_interrupt_callback_t interrupt_callbacks[MAX_INTERRUPT_NUM] = {NULL};
static pthread_t interrupt_thread;
static volatile int interrupt_running = 0;

/**
 * @brief Set GPIO pin direction and initial value
 * @param fd GPIO device file descriptor
 * @param gpio GPIO pin number
 * @param value Initial output value (0 or 1)
 * @return Returns 0 on success, -1 on failure
 */
int gpio_set_direction_and_value(int fd, int gpio, int value) 
{
    struct gpiohandle_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = gpio;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = 1;

    req.default_values[0] = value;

    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("GPIO_GET_LINEHANDLE_IOCTL failed");
        return -1;
    }

    close(req.fd);
    return 0;
}

/**
 * @brief Get current GPIO pin value
 * @param fd GPIO device file descriptor
 * @param gpio GPIO pin number
 * @param value Pointer to store pin value
 * @return Returns 0 on success, -1 on failure
 */
int gpio_get_value(int fd, int gpio, int* value) 
{
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = gpio;
    req.flags = GPIOHANDLE_REQUEST_INPUT;
    req.lines = 1;

    if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("GPIO_GET_LINEHANDLE_IOCTL failed");
        return -1;
    }

    if (ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0) {
        perror("GPIOHANDLE_GET_LINE_VALUES_IOCTL failed");
        close(req.fd);
        return -1;
    }

    *value = data.values[0];

    close(req.fd);
    return 0;
}

/**
 * @brief GPIO control main function (set direction/get value)
 * @param gpiochip_path GPIO chip device path
 * @param gpio GPIO pin number
 * @param direction Direction setting ("in" or "out")
 * @param value Output value (only valid for output mode)
 * @return Returns 0 on success, -1 on failure
 */
int gpio_control(const char *gpiochip_path, int gpio, const char *direction, int value)
{
    int fd = open(gpiochip_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open gpiochip");
        return -1;
    }

    int dir_flag = strcmp(direction, "out") == 0 ? GPIOHANDLE_REQUEST_OUTPUT : GPIOHANDLE_REQUEST_INPUT;

    if (dir_flag == GPIOHANDLE_REQUEST_OUTPUT) {
        if (gpio_set_direction_and_value(fd, gpio, value) < 0) {
            close(fd);
            return -1;
        }
    } else {
        int read_value;
        if (gpio_get_value(fd, gpio, &read_value) < 0) {
            close(fd);
            return -1;
        }
        printf("GPIO %d value: %d\n", gpio, read_value);
    }

    close(fd);
    return 0;
}

// GPIO interrupt thread function
static void* interrupt_thread_func(void* arg) {
    (void)arg;
    
    while (interrupt_running) {
        for (int i = 0; i < MAX_INTERRUPT_NUM; i++) {
            if (interrupt_requests[i] != NULL && interrupt_callbacks[i] != NULL && interrupt_buffers[i] != NULL) {
                // Wait for interrupt event (non-blocking, timeout 100ms)
                if (gpiod_line_request_wait_edge_events(interrupt_requests[i], 100000000) > 0) {
                    int num_events = gpiod_line_request_read_edge_events(interrupt_requests[i],
                                                                         interrupt_buffers[i],
                                                                         gpiod_edge_event_buffer_get_capacity(interrupt_buffers[i]));
                    
                    if (num_events > 0) {
                        for (int j = 0; j < num_events; j++) {
                            struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(interrupt_buffers[i], j);
                            if (event != NULL) {
                                gpio_event_type_t event_type;
                                
                                switch (gpiod_edge_event_get_event_type(event)) {
                                    case GPIOD_EDGE_EVENT_RISING_EDGE:
                                        event_type = GPIO_EVENT_RISING_EDGE;
                                        break;
                                    case GPIOD_EDGE_EVENT_FALLING_EDGE:
                                        event_type = GPIO_EVENT_FALLING_EDGE;
                                        break;
                                    default:
                                        event_type = GPIO_EVENT_BOTH_EDGES;
                                        break;
                                }
                                
                                // Call callback function
                                uint64_t timestamp_ns = gpiod_edge_event_get_timestamp_ns(event);
                                interrupt_callbacks[i](i, event_type, timestamp_ns / 1000000000, timestamp_ns % 1000000000);
                            }
                        }
                    }
                }
            }
        }
    }
    
    return NULL;
}

/**
 * @brief Initialize GPIO interrupt
 * @param gpiochip_path GPIO chip device path
 * @param gpio GPIO pin number
 * @param event_type Interrupt event type
 * @return Returns 0 on success, -1 on failure
 */
int gpio_interrupt_init(const char *gpiochip_path, int gpio, gpio_event_type_t event_type) {
    if (gpio < 0 || gpio >= MAX_INTERRUPT_NUM) {
        fprintf(stderr, "GPIO pin number out of range: %d\n", gpio);
        return -1;
    }
    
    // Open GPIO chip (if not already opened)
    if (interrupt_chip == NULL) {
        interrupt_chip = gpiod_chip_open(gpiochip_path);
        if (interrupt_chip == NULL) {
            perror("Failed to open gpiochip");
            return -1;
        }
    }
    
    // Configure interrupt event type
    enum gpiod_line_edge edge_detection;
    switch (event_type) {
        case GPIO_EVENT_RISING_EDGE:
            edge_detection = GPIOD_LINE_EDGE_RISING;
            break;
        case GPIO_EVENT_FALLING_EDGE:
            edge_detection = GPIOD_LINE_EDGE_FALLING;
            break;
        case GPIO_EVENT_BOTH_EDGES:
            edge_detection = GPIOD_LINE_EDGE_BOTH;
            break;
        default:
            fprintf(stderr, "Invalid event type: %d\n", event_type);
            return -1;
    }
    
    // Create line settings
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (settings == NULL) {
        perror("Failed to create line settings");
        return -1;
    }
    
    // Configure settings
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(settings, edge_detection);
    
    // Create line config
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (line_cfg == NULL) {
        perror("Failed to create line config");
        gpiod_line_settings_free(settings);
        return -1;
    }
    
    // Add line settings
    unsigned int offset = (unsigned int)gpio;
    if (gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings) < 0) {
        perror("Failed to add line settings");
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        return -1;
    }
    
    // Create request config
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    if (req_cfg == NULL) {
        perror("Failed to create request config");
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        return -1;
    }
    
    gpiod_request_config_set_consumer(req_cfg, "gpio-interrupt");
    
    // Request lines
    interrupt_requests[gpio] = gpiod_chip_request_lines(interrupt_chip, req_cfg, line_cfg);
    if (interrupt_requests[gpio] == NULL) {
        perror("Failed to request GPIO lines");
        gpiod_request_config_free(req_cfg);
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        return -1;
    }
    
    // Create edge event buffer
    interrupt_buffers[gpio] = gpiod_edge_event_buffer_new(16);
    if (interrupt_buffers[gpio] == NULL) {
        perror("Failed to create edge event buffer");
        gpiod_line_request_release(interrupt_requests[gpio]);
        interrupt_requests[gpio] = NULL;
        gpiod_request_config_free(req_cfg);
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        return -1;
    }
    
    // Clean up temporary objects
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    
    // Start interrupt thread (if not already started)
    if (!interrupt_running) {
        interrupt_running = 1;
        if (pthread_create(&interrupt_thread, NULL, interrupt_thread_func, NULL) != 0) {
            perror("Failed to create interrupt thread");
            interrupt_running = 0;
            gpiod_edge_event_buffer_free(interrupt_buffers[gpio]);
            interrupt_buffers[gpio] = NULL;
            gpiod_line_request_release(interrupt_requests[gpio]);
            interrupt_requests[gpio] = NULL;
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief Register GPIO interrupt callback function
 * @param gpio GPIO pin number
 * @param callback Callback function pointer
 * @return Returns 0 on success, -1 on failure
 */
int gpio_interrupt_register_callback(int gpio, gpio_interrupt_callback_t callback) {
    if (gpio < 0 || gpio >= MAX_INTERRUPT_NUM) {
        fprintf(stderr, "GPIO pin number out of range: %d\n", gpio);
        return -1;
    }
    
    if (interrupt_requests[gpio] == NULL) {
        fprintf(stderr, "GPIO %d not initialized for interrupt\n", gpio);
        return -1;
    }
    
    interrupt_callbacks[gpio] = callback;
    return 0;
}

/**
 * @brief Unregister GPIO interrupt callback function
 * @param gpio GPIO pin number
 * @return Returns 0 on success, -1 on failure
 */
int gpio_interrupt_unregister_callback(int gpio) {
    if (gpio < 0 || gpio >= MAX_INTERRUPT_NUM) {
        fprintf(stderr, "GPIO pin number out of range: %d\n", gpio);
        return -1;
    }
    
    interrupt_callbacks[gpio] = NULL;
    return 0;
}

/**
 * @brief Clean up GPIO interrupt resources
 */
void gpio_interrupt_cleanup(void) {
    // Stop interrupt thread
    interrupt_running = 0;
    if (interrupt_thread) {
        pthread_join(interrupt_thread, NULL);
    }
    
    // Release all GPIO resources
    for (int i = 0; i < MAX_INTERRUPT_NUM; i++) {
        if (interrupt_buffers[i] != NULL) {
            gpiod_edge_event_buffer_free(interrupt_buffers[i]);
            interrupt_buffers[i] = NULL;
        }
        if (interrupt_requests[i] != NULL) {
            gpiod_line_request_release(interrupt_requests[i]);
            interrupt_requests[i] = NULL;
        }
        interrupt_callbacks[i] = NULL;
    }
    
    // Close GPIO chip
    if (interrupt_chip != NULL) {
        gpiod_chip_close(interrupt_chip);
        interrupt_chip = NULL;
    }
}
