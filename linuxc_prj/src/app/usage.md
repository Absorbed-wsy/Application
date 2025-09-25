# ==========================================================================================================================
# 关于线程池的使用方法
int main(int argc, char *argv[]) 
{
    int loop_status=0;
    
    //log init
    logger_init(NULL, LOG_LEVEL_INFO, 1);
    LOG_INFO("Program started");

    //config init
    config_initialize("app.conf");

    //command parsing
    if(argc > 1)
        loop_status = command_parsing(argv);

    //process thread task
    LOG_INFO("Making threadpool with 4 threads");
    threadpool thpool = thpool_init(2);

    LOG_INFO("Adding 4 tasks to threadpool");
	for (int i=0; i<4; i++){
		thpool_add_work(thpool, PrivateTask, (void*)(uintptr_t)i);
	};

    while (loop_status) {
        LOG_INFO("main loop");
        sleep(1);
    }

    LOG_INFO("Killing threadpool");
    thpool_wait(thpool);
	thpool_destroy(thpool);

    LOG_INFO("Program over");
    
    exit(0);
}

void PrivateTask(void* arg) {
    int value = (int)(uintptr_t)arg;
    for(;;) {
        printf("Thread #%lu working on %d\n", (unsigned long)pthread_self(), value);
        sleep(1);
    }
}
# ==========================================================================================================================
# 关于纸飞机调试助手使用网络绘制使用方法
char buf[20]={0};
static int test = 0;

int server_fd = tcp_server_init(9001);
int client_fd = tcp_server_accept(server_fd);

memset(buf, 0, 20);
sprintf(buf, "{apptest}%d\n", test);
tcp_server_send(client_fd, buf, strlen(buf), 100);
# ==========================================================================================================================