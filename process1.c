#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#define SHM_KEY 0x1234
#define SHM_SIZE 512

int main() {
    
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        printf("cannot create shared memory\n");
        return 1;
    }
    
    char *shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
        printf("cannot attach shared memory\n");
        return 1;
    }
    
    printf("Process 1: Writer started\n");
    printf("Enter messages to send (type 'exit' to quit):\n");
    
    char buffer[SHM_SIZE];
    int offset = 0;  
    
    while (1) {
        printf("Enter a message to send: ");
        fgets(buffer, sizeof(buffer), stdin);
        
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        
        
        sprintf(shared_memory + offset, "%s\n", buffer);
        offset += strlen(buffer) + 1;
        
        printf("Process 1: Wrote '%s' in Shared Memory.\n", buffer);
        
        if (strcmp(buffer, "exit") == 0)
            break;
        
        int k;
        for (k = 0; k < 50000000; k++) {}
    }
    
    shmdt(shared_memory);
    printf("Process 1: Exiting\n");
    
    return 0;
}
