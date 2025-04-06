#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define ID_SIZE 16
#define MAX_PATH 512
#define NAME_SIZE 32
#define CLUE_SIZE 128

typedef struct GPS{
  float longitude;
  float latitude;
}GPS_T;

typedef struct treasure{
  char id[ID_SIZE];
  char name[NAME_SIZE];
  GPS_T coord;
  char clue[CLUE_SIZE];
  int value;
}Treasure_t;


void add_treasure(char *hunt_id){
  char director_path[MAX_PATH];
  snprintf(director_path, sizeof(director_path),"%s",hunt_id);

  if(mkdir(director_path,0755)==-1 && errno!=EEXIST){
    perror("Failed to create hunt directory");
    return;
  }

  char file_path[MAX_PATH];
  
  snprintf(file_path, sizeof(file_path),"%400s/treasures.dat",director_path);

  int fd=open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if(fd<0){
    perror("Failed to open treasures.dat");
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

  printf("Treasure log added successfully!\n");
  char log_message[256];
  snprintf(log_message,sizeof(log_message), "add: id:%s, username: %s", t.id, t.name);


  close(fd);
}


int main (int argc, char **argv){
  if(argc<3){
    printf("Not enough arguments\n");
    return 1;
  }

  if(strcmp(argv[1],"add") ==0 ){
    add_treasure(argv[2]);
  }
  
  return 0;
}
