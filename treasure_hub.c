#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "treasure_manager.h"

#define COMMAND_FILE "monitor_command.txt"

pid_t monitor_pid = -1;
int monitor_stopping = 0;

void sigchld_handler(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("Monitor has terminated. Status: %d\n", WEXITSTATUS(status));
    monitor_pid = -1;
    monitor_stopping = 0;
}

void start_monitor() {
    if (monitor_pid > 0) {
        printf("Monitor already running.\n");
        return;
    }
    monitor_pid = fork();
    if (monitor_pid == 0) {
        execl("./treasure_monitor", "treasure_monitor", NULL);
        perror("Failed to start monitor");
        exit(1);
    } else if (monitor_pid > 0) {
        printf("Monitor started. PID: %d\n", monitor_pid);
    } else {
        perror("fork failed");
    }
}

void stop_monitor() {
    if (monitor_pid > 0) {
        send_command("stop_monitor");
        monitor_stopping = 1;
    } else {
        printf("No monitor running to stop.\n");
    }
}


void send_command(const char *command) {
    if (monitor_pid <= 0) {
        printf("Monitor not running.\n");
        return;
    }
    if (monitor_stopping) {
        printf("Monitor is stopping, please wait...\n");
        return;
    }

    int fd = open(COMMAND_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
    if (fd < 0) {
        perror("Failed to open command file");
        return;
    }
    write(fd, command, strlen(command));
    close(fd);

    kill(monitor_pid, SIGUSR1);
}


void list_hunts() {
    DIR *dir = opendir("HUNTS");
    if (!dir) {
        perror("Failed to open HUNTS directory");
        return;
    }

    struct dirent *entry;
    printf("Listing all hunts:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("- %s\n", entry->d_name);
        }
    }
    closedir(dir);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char input[128];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin))
            break;
        
        input[strcspn(input, "\n")] = '\0'; 

        if (strcmp(input, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(input, "list_hunts") == 0) {
            send_command("list_hunts");
	    list_hunts();
        } else if (strcmp(input, "list_treasures") == 0) {
            send_command("list_treasures");
        } else if (strcmp(input, "view_treasure") == 0) {
            send_command("view_treasure");
        } else if (strcmp(input, "stop_monitor") == 0) {
	  //send_command("stop_monitor");
            monitor_stopping = 1;
	    stop_monitor();
        } else if (strcmp(input, "exit") == 0) {
            if (monitor_pid > 0) {
                printf("Error: Monitor still running.\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
