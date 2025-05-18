#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "treasure_manager.h" 

#define MAX_USERS 100

typedef struct {
    char name[NAME_SIZE];
    int total_value;
} ScoreEntry;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_directory>\n", argv[0]);
        return 1;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/treasures.dat", argv[1]);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open treasures.dat");
        return 1;
    }

    ScoreEntry scores[MAX_USERS];
    int user_count = 0;

    Treasure_t t;
    while (read(fd, &t, sizeof(Treasure_t)) == sizeof(Treasure_t)) {
        int found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(scores[i].name, t.name) == 0) {
                scores[i].total_value += t.value;
                found = 1;
                break;
            }
        }
        if (!found && user_count < MAX_USERS) {
            strncpy(scores[user_count].name, t.name, NAME_SIZE - 1);
            scores[user_count].name[NAME_SIZE - 1] = '\0';
            scores[user_count].total_value = t.value;
            user_count++;
        }
    }

    close(fd);

    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", scores[i].name, scores[i].total_value);
    }

    return 0;
}
