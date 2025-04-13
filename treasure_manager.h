#ifndef TREASURE_MANAGER_H
#define TREASURE_MANAGER_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#define ID_SIZE 16
#define MAX_PATH 1024
#define NAME_SIZE 32
#define CLUE_SIZE 128


typedef struct GPS {
    float longitude;
    float latitude;
} GPS_T;


typedef struct treasure {
    char id[ID_SIZE];
    char name[NAME_SIZE];
    GPS_T coord;
    char clue[CLUE_SIZE];
    int value;
} Treasure_t;


void log_action(const char *hunt_id, const char *operation);
void remove_hunt(const char *hunt_id);
int check_directory_exists(char *dir_path);
void start_function(void);
void list_treasures(char *hunt_id);
void view_treasure(const char *hunt_id, const char *treasure_id);
void remove_treasure(const char *hunt_id, const char *treasure_id);
void add_treasure(char *hunt_id);

#endif
