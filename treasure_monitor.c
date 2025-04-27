#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define COMMAND_FILE "monitor_command.txt"

volatile sig_atomic_t command_ready = 0;

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        FILE *f = fopen("monitor_command.txt", "r");
        if (f) {
            char command[256];
            if (fgets(command, sizeof(command), f)) {
                printf("Monitor received command: %s\n", command);
            }
            fclose(f);
        }
    }
}


void monitor_loop() {
    while (1) {
        if (command_ready) {
            command_ready = 0;

            int fd = open(COMMAND_FILE, O_RDONLY);
            if (fd < 0) {
                perror("Cannot open command file");
                continue;
            }
            char buf[128] = {0};
            read(fd, buf, sizeof(buf)-1);
            close(fd);

            if (strcmp(buf, "list_hunts") == 0) {
	        printf("[Monitor] Listing hunts...\n");
            } else if (strcmp(buf, "list_treasures") == 0) {
                printf("[Monitor] Listing treasures...\n");
            } else if (strcmp(buf, "view_treasure") == 0) {
                printf("[Monitor] Viewing treasure...\n");
            } else if (strcmp(buf, "stop_monitor") == 0) {
                printf("[Monitor] Stopping...\n");
                usleep(4000000); 
                exit(0);
            } else {
                printf("[Monitor] Unknown command: %s\n", buf);
            }
        }
        usleep(100000); 
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    printf("Monitor started (PID: %d)\n", getpid());
    monitor_loop();
}
