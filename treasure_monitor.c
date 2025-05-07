#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "treasure_manager.h"


#define COMMAND_FILE "monitor_command.txt"

volatile sig_atomic_t command_ready = 0;

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        command_ready = 1;
    }
}

void handle_sigint(int sig) {
    printf("\n[Monitor] Caught SIGINT, exiting...\n");
    fflush(stdout);
    exit(0);
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

	    buf[strcspn(buf, "\r\n")] = 0;

            if (strcmp(buf, "list_hunts") == 0) {
	        printf("[Monitor] Listing hunts...\n");
		fflush(stdout);
            } else if (strcmp(buf, "list_treasures") == 0) {
                printf("[Monitor] Listing treasures...\n");
		fflush(stdout);
            } else if (strcmp(buf, "view_treasure") == 0) {
                printf("[Monitor] Viewing treasure...\n");
		fflush(stdout);
            } else if (strcmp(buf, "stop_monitor") == 0) {
                printf("[Monitor] Stopping...\n");
		fflush(stdout);
                usleep(3000000); 
                exit(0);
            } else {
                printf("[Monitor] Unknown command: %s\n", buf);
		fflush(stdout);
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

    signal(SIGINT, handle_sigint);
    
    printf("Monitor started (PID: %d)\n", getpid());
    monitor_loop();
}
