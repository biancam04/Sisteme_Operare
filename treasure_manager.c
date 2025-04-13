#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "treasure_manager.h"

#define ID_SIZE 16
#define MAX_PATH 1024
#define NAME_SIZE 32
#define CLUE_SIZE 128


void log_action(const char *hunt_id, const char *operation) {
    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "HUNTS/%s/logged_hunt", hunt_id);

    int log_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (log_fd < 0) {
        perror("Failed to open log file");
        return;
    }
    
    dprintf(log_fd, "%s\n", operation);
    close(log_fd);

    char link_name[MAX_PATH];
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);

    struct stat st;
    if (lstat(link_name, &st) == -1) {
        if (symlink(log_path, link_name) == -1) {
            perror("Failed to create symbolic link");
        }
    }
}



void remove_hunt(const char *hunt_id) {
    char dir_path[MAX_PATH];
    snprintf(dir_path, sizeof(dir_path), "HUNTS/%s", hunt_id);

    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Hunt not found");
        return;
    }

    struct dirent *entry;
    char filepath[MAX_PATH];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
            unlink(filepath);
        }
    }
    closedir(dir);

    if (rmdir(dir_path) == 0) {
        printf("Hunt '%s' removed.\n", hunt_id);
        //char log_msg[128];
        //snprintf(log_msg, sizeof(log_msg), "remove_hunt: %s", hunt_id);
        //log_action(hunt_id, log_msg);

        char link_path[MAX_PATH];
        snprintf(link_path, sizeof(link_path), "logged_hunt-%s", hunt_id);
        unlink(link_path);
    } else {
        perror("Failed to remove hunt directory");
    }
}



int check_directory_exists(char *dir_path) {
    struct stat statbuf;
    return (stat(dir_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)); 
}


void start_function(void){
  if (!check_directory_exists("HUNTS")) {
        if (mkdir("HUNTS", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            perror("Failed to create HUNTS directory");
            return;
        }
        printf("HUNTS directory created\n");
    } else {
        printf("HUNTS directory already exists\n");
    }
}


void list_treasures(char *hunt_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "HUNTS/%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open treasures.dat for reading");
        return;
    }

    Treasure_t t;
    printf("Listing treasures in hunt: %s\n", hunt_id);
    int count = 0;

    errno = 0;
    while (read(fd, &t, sizeof(Treasure_t)) == sizeof(Treasure_t)) {
        printf("Treasure %d: \n", ++count);
        printf("ID: %s\n", t.id);
        printf("Name: %s\n", t.name);
        printf("Longitude: %f\n", t.coord.longitude);
        printf("Latitude: %f\n", t.coord.latitude);
        printf("Clue: %s\n", t.clue);
        printf("Value: %d\n", t.value);
        printf("------------------------------\n");
    }

    if (count == 0) {
        printf("No treasures found in this hunt.\n");
    } else if (errno != 0) {
        perror("Error reading the file");
    }

    char log_message[256];
    snprintf(log_message, sizeof(log_message), "list: hunt_id=%s", hunt_id);
    log_action(hunt_id, log_message);

    close(fd);
}



void view_treasure(const char *hunt_id, const char *treasure_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "HUNTS/%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open treasure file");
        return;
    }

    Treasure_t t;
    while (read(fd, &t, sizeof(Treasure_t)) == sizeof(Treasure_t)) {
        if (strcmp(t.id, treasure_id) == 0) {
            printf("Treasure ID: %s\nName: %s\nLongitude: %f\nLatitude: %f\nClue: %s\nValue: %d\n",
                   t.id, t.name, t.coord.longitude, t.coord.latitude, t.clue, t.value);
            close(fd);

            char log_msg[128];
            snprintf(log_msg, sizeof(log_msg), "view: id=%s", treasure_id);
            log_action(hunt_id, log_msg);
            return;
        }
    }

    char log_message[256];
    snprintf(log_message, sizeof(log_message), "list: hunt_id:%s", hunt_id);
    log_action(hunt_id, log_message);

    printf("Treasure not found.\n");
    close(fd);
}


void remove_treasure(const char *hunt_id, const char *treasure_id) {
    char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "HUNTS/%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open original file");
        return;
    }

    char tmp_path[MAX_PATH];
    snprintf(tmp_path, sizeof(tmp_path), "HUNTS/%s/tmp.dat", hunt_id);
    int tmp_fd = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (tmp_fd < 0) {
        perror("Failed to create temp file");
        close(fd);
        return;
    }

    Treasure_t t;
    int found = 0;
    while (read(fd, &t, sizeof(Treasure_t)) == sizeof(Treasure_t)) {
        if (strcmp(t.id, treasure_id) != 0) {
            write(tmp_fd, &t, sizeof(Treasure_t));
        } else {
            found = 1;
        }
    }

    close(fd);
    close(tmp_fd);

    if (found) {
        rename(tmp_path, file_path);
        printf("Treasure removed.\n");

        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "remove_treasure: id=%s", treasure_id);
        log_action(hunt_id, log_msg);
	
    } else {
        unlink(tmp_path);
        printf("Treasure not found.\n");
    }
}


void add_treasure(char *hunt_id){

  start_function();
  
  char director_path[MAX_PATH];
  snprintf(director_path, sizeof(director_path),"HUNTS/%s",hunt_id);

  if(mkdir(director_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1 && errno!=EEXIST){
    perror("Failed to create directory");
    return;
  }

  char file_path[MAX_PATH];
  
  snprintf(file_path, sizeof(file_path),"HUNTS/%s/treasures.dat", hunt_id);
  

  int fd=open(file_path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if(fd<0){
    perror("Failed to open");
    return;
  }

  Treasure_t t;
  printf("Add ID: ");
  scanf("%15s",t.id);

  printf("Add name: ");
  scanf("%31s",t.name);

  printf("Add longitude: ");
  scanf("%f",&t.coord.longitude);

  printf("Add latitude: ");
  scanf("%f",&t.coord.latitude);

  printf("Add clue: ");
  getchar();
  fgets(t.clue,CLUE_SIZE,stdin);
  t.clue[strcspn(t.clue,"\n")]='\0';

  printf("Add value: ");
  scanf("%d",&t.value);
  

  if(write(fd, &t, sizeof(Treasure_t)) != sizeof(Treasure_t)){
    perror("Failed to write");
    return;
  }
  

  lseek(fd, 0, SEEK_SET);
  

  printf("Treasure log added successfully!\n");
  char log_message[256];
  snprintf(log_message,sizeof(log_message), "add: id:%s, username: %s", t.id, t.name);
  log_action(hunt_id, log_message);

  close(fd);
}


