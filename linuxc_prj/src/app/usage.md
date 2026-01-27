# ==========================================================================================================================
# Thread Pool Usage Guide
//process thread task
LOG_INFO("Making threadpool with 4 threads");
threadpool thpool = thpool_init(4);
LOG_INFO("Adding 4 tasks to threadpool");
for (int i=0; i<4; i++) {thpool_add_work(thpool, PrivateTask, (void*)(uintptr_t)i);};
LOG_INFO("Killing threadpool");
thpool_wait(thpool);
thpool_destroy(thpool);

void PrivateTask(void* arg) {
    int value = (int)(uintptr_t)arg;
    for(;;) {
        printf("Thread #%lu working on %d\n", (unsigned long)pthread_self(), value);
        sleep(1);
    }
}
# ==========================================================================================================================
# Paper Airplane Debug Assistant Network Drawing Usage
char buf[20]={0};
static int test = 0;

int server_fd = tcp_server_init(9001);
int client_fd = tcp_server_accept(server_fd);

memset(buf, 0, 20);
sprintf(buf, "{apptest}%d\n", test);
tcp_server_send(client_fd, buf, strlen(buf), 100);
# ==========================================================================================================================
# PID Algorithm Usage
PID_Controller pid;
PID_Init(&pid, 2.5f, 0.5f, 0.8f, 100.0f, 50.0f);
PID_SetTarget(&pid, 30.0f);
PID_Calculate(&pid, actual, time);
float actual = System_Response(control);
# ==========================================================================================================================
# Code Execution Time Measurement Solution
struct timespec start, end;
clock_gettime(CLOCK_MONOTONIC, &start);
measured_function();
clock_gettime(CLOCK_MONOTONIC, &end);

double elapsed = (end.tv_sec - start.tv_sec) * 1e9; 
elapsed += (end.tv_nsec - start.tv_nsec);
printf("Execution time: %.6f milliseconds\n", elapsed / 1e6);
# ==========================================================================================================================
# SSD1306 0.96 OLED Usage
LOG_INFO("Initializing OLED...\n");
if (OLED_Init() < 0) {
    fprintf(stderr, "Failed to initialize OLED\n");
    return -1;
}
for(;;) {
    OLED_ShowString(0, 0, (u8*)"Hello World!", 16);
    OLED_Refresh();
    sleep(1);
}
# ==========================================================================================================================
# UART Peripheral Usage
// Define UART configuration
uart_config_t config = UART_CONFIG_DEFAULT;
config.baud_rate = 115200; // Set baud rate to 115200

// Open UART (modify path according to actual device)
const char *device_path = "/dev/ttyAS4"; // UART device path
uart_handle_t uart = uart_open(device_path, &config);

// Example 1: Send data
const char *send_data = "Hello UART!\r\n";
int send_len = strlen(send_data);
uart_write(uart, (const uint8_t *)send_data, send_len, 1000);

// Example 2: Receive data (with timeout)
uint8_t recv_buffer[256];
int ret=0;

ret = uart_read(uart, recv_buffer, sizeof(recv_buffer) - 1, 5000);
if (ret >= 0) {
    recv_buffer[ret] = '\0'; // Add string terminator
    printf("Receive successful, bytes: %d, data: %s\n", ret, recv_buffer);
}
# ==========================================================================================================================
# GPIO Interrupt Trigger Usage
void gpio_interrupt_cb(int gpio, gpio_event_type_t event_type, 
                      unsigned long sec, unsigned long nsec) {
    const char *event_str;
    
    switch (event_type) {
        case GPIO_EVENT_RISING_EDGE:
            event_str = "Rising Edge";
            break;
        case GPIO_EVENT_FALLING_EDGE:
            event_str = "Falling Edge";
            break;
        case GPIO_EVENT_BOTH_EDGES:
            event_str = "Both Edges";
            break;
        default:
            event_str = "Unknown";
            break;
    }
    
    printf("GPIO %d Interrupt: %s, Timestamp: %ld.%09ld\n",
           gpio, event_str, sec, nsec);
}

gpio_interrupt_init("/dev/gpiochip0", 39, GPIO_EVENT_RISING_EDGE);
gpio_interrupt_register_callback(39, gpio_interrupt_cb);
# ==========================================================================================================================