#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include "treasure_manager.h"

#define COMMAND_FILE "monitor_command.txt"

pid_t monitor_pid = -1;
int monitor_stopping = 0;
int monitor_pipe_fd = -1;

void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == monitor_pid) {
            printf("[Hub] Monitor has terminated. Status: %d\n", WEXITSTATUS(status));
            monitor_pid = -1;
            monitor_stopping = 0;
            close(monitor_pipe_fd);
            monitor_pipe_fd = -1;
        }
    }
}

void read_monitor_output() {
    if (monitor_pipe_fd < 0) return;

    char buf[256];
    int n = read(monitor_pipe_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("[Monitor]: %s", buf);
    }
}

void send_command(const char *command) {
    if (monitor_pid <= 0) {
        printf("[Hub] Monitor not running\n");
        return;
    }
    if (monitor_stopping && strcmp(command, "stop_monitor") != 0) {
        printf("[Hub] Monitor is stopping, please wait...\n");
        return;
    }

    int fd = open(COMMAND_FILE, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("[Hub] Failed to open command file");
        return;
    }

    write(fd, command, strlen(command));
    close(fd);

    kill(monitor_pid, SIGUSR1);
}

void start_monitor() {
    if (monitor_pid > 0) {
        printf("[Hub] Monitor already running\n");
        return;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid == 0) {
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]);
        execl("./treasure_monitor", "treasure_monitor", NULL);
        perror("exec monitor");
        exit(1);
    } else if (monitor_pid > 0) {
        close(pipefd[1]); 
        monitor_pipe_fd = pipefd[0];
        printf("[Hub] Monitor started. PID: %d\n", monitor_pid);
    } else {
        perror("fork failed");
    }
}

void stop_monitor() {
    if (monitor_pid > 0) {
        send_command("stop_monitor");
        monitor_stopping = 1;
    } else {
        printf("[Hub] No monitor running to stop\n");
    }
}

void list_hunts() {
    DIR *dir = opendir("HUNTS");
    if (!dir) {
        perror("[Hub] Failed to open HUNTS directory");
        return;
    }

    struct dirent *entry;
    printf("[Hub] Listing all hunts:\n");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {
            printf("- %s\n", entry->d_name);
        }
    }
    closedir(dir);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
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
            read_monitor_output();
            list_hunts();

        } else if (strncmp(input, "list_treasures", 14) == 0) {
            char *hunt = input + 15;
            if (hunt && strlen(hunt) > 0) {
                send_command("list_treasures");
                read_monitor_output();
		list_treasures(hunt);
            } else {
                printf("[Hub] Invalid list_treasures command\n");
            }

        } else if (strncmp(input, "view_treasure", 13) == 0) {
            char *args = input + 14;
            char *hunt = strtok(args, " ");
            char *treasure = strtok(NULL, " ");
            if (hunt && treasure) {
                send_command("view_treasure");
                read_monitor_output();
		view_treasure(hunt,treasure);
            } else {
                printf("[Hub] Invalid view_treasure command\n");
            }

        } else if (strcmp(input, "stop_monitor") == 0) {
            stop_monitor();

        } else if (strcmp(input, "exit") == 0) {
            if (monitor_pid > 0) {
                printf("[Hub] Error: Monitor still running. Use stop_monitor first.\n");
            } else {
                break;
            }

        } else if (strcmp(input, "calculate_score") == 0) {
            DIR *dir = opendir("HUNTS");
            if (!dir) {
                perror("[Hub] Failed to open HUNTS directory");
                continue;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR &&
                    strcmp(entry->d_name, ".") != 0 &&
                    strcmp(entry->d_name, "..") != 0) {

                    int pipefd[2];
                    if (pipe(pipefd) == -1) {
                        perror("pipe failed");
                        continue;
                    }

                    pid_t pid = fork();
                    if (pid == 0) {
                        close(pipefd[0]); 
                        dup2(pipefd[1], STDOUT_FILENO); 
                        close(pipefd[1]);

                        char path[1024];
                        snprintf(path, sizeof(path), "HUNTS/%s", entry->d_name);
                        execl("./score_calculator", "score_calculator", path, NULL);
                        perror("exec score_calculator");
                        exit(1);
                    } else if (pid > 0) {
                        close(pipefd[1]); 
                        char buf[256];
                        int n;
                        printf("[Hub] Scores for hunt '%s':\n", entry->d_name);
                        while ((n = read(pipefd[0], buf, sizeof(buf) - 1)) > 0) {
                            buf[n] = '\0';
                            printf("%s", buf);
                        }
                        close(pipefd[0]);
                        waitpid(pid, NULL, 0);
                    }
                }
            }
            closedir(dir);

        } else {
            printf("[Hub] Unknown command\n");
        }
    }

    return 0;
}
