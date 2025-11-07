#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#define SHM_KEY 0x1234
#define SHM_SIZE 512

int main() {
    
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        printf("cannot access shared memory\n");
        printf("run Process 1 first\n");
        return 1;
    }
    
    char *shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
        printf("cannot attach shared memory\n");
        return 1;
    }
    
    printf("process 2: reader started\n");
    printf("waiting for messages...\n");
    
    int last_index = 0;
    
    while (1) {
        if (shared_memory[last_index] != '\0') {
            char message[SHM_SIZE];
            int i = 0;
            
            while (shared_memory[last_index] != '\n' && shared_memory[last_index] != '\0') {
                message[i++] = shared_memory[last_index++];
            }
            message[i] = '\0';
            if (shared_memory[last_index] == '\n')
                last_index++;

            printf("process 2: read '%s' from shared memory.\n", message);

            if (strcmp(message, "exit") == 0)
                break;
        }
        
        int k;
        for (k = 0; k < 10000000; k++) {}
    }
    
    shmdt(shared_memory);
    printf("process 2: exiting\n");
    
    return 0;
}
