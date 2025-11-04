#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *f;

    if (argc != 2) {
        printf("usage: %s [0|1]\n", argv[0]);
        return 1;
    }

    f = fopen("/sys/class/leds/input4::capslock/brightness", "w");
    if (f == NULL) {
        perror("error opening brightness file");
        return 1;
    }

    fprintf(f, "%s", argv[1]);
    fclose(f);

    printf("caps lock led set to %s\n", argv[1]);
    return 0;
}
