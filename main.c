#include "treasure_manager.h"

int main(int argc, char **argv){
  if(argc<3){
    printf("Not enough arguments\n");
    return 1;
  }

  if (strcmp(argv[1], "--add") == 0) {
        add_treasure(argv[2]);
    }
  else if (strcmp(argv[1], "--list") == 0) {
        list_treasures(argv[2]);
    }
  else if (strcmp(argv[1], "--view") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: 'view' requires <hunt_id> and <treasure_id>\n");
            return 1;
        }
        view_treasure(argv[2], argv[3]);
    }
  else if (strcmp(argv[1], "--remove_treasure") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: 'remove_treasure' requires <hunt_id> and <treasure_id>\n");
            return 1;
        }
        remove_treasure(argv[2], argv[3]);
    }
  else if (strcmp(argv[1], "--remove_hunt") == 0) {
        remove_hunt(argv[2]);
    }
  else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }
  
  return 0;
}
