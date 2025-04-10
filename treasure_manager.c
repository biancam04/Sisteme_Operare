#include <stdio.h>
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

  close(fd);
}


void list_treasures(char *hunt_id){
  char file_path[MAX_PATH];
    snprintf(file_path, sizeof(file_path), "HUNTS/%s/treasures.dat", hunt_id);

    // Open the file for reading
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open treasures.dat for reading");
        return;
    }

    Treasure_t t;

    printf("Listing treasures in hunt: %s\n", hunt_id);
    int count = 0;
    while (read(fd, &t, sizeof(Treasure_t)) == sizeof(Treasure_t)) {
        printf("Treasure %d: \n", ++count);
        printf("ID: %s\n", t.id);
        printf("Name: %s\n", t.name);
        printf("Longitude: %f\n", t.coord.longitude);
        printf("Latitude: %f\n", t.coord.latitude);
        printf("Clue: %s\n", t.clue);
        printf("Value: %d\n", t.value);
        printf("----------------------------------------\n");
    }

  
    if (count == 0) {
        printf("No treasures found in this hunt.\n");
    } else if (errno != 0) {
        perror("Error reading the file");
    }

    close(fd);
}



int main(int argc, char **argv){
  if(argc<3){
    printf("Not enough arguments\n");
    return 1;
  }

  if(strcmp(argv[1],"add") ==0 ){
    add_treasure(argv[2]);
  }

  if(strcmp(argv[1],"list") == 0){
    list_treasures(argv[2]);
  }
  
  return 0;
}
