#define write_t 0x01

void my_printf(const char *format, int size);
void write(unsigned int fd ,const char *buf , int count);


int main() {
 

    char text[] = "Hello World\n";

    my_printf(text, 12);

    return 0;
}


void write(unsigned int fd ,const char *buf , int count){
    asm volatile(
    "mov $1, %%rax\n\t"
    "mov %[fd], %%edi\n\t"
    "mov %[buf], %%rsi\n\t"
    "mov %[count], %%edx\n\t"
    "syscall\n\t"
    :
    : [fd]"r"(fd), [buf]"r"(buf), [count]"r"(count)
);

}



void my_printf(const char *format, int size) {
    write(write_t, format, size);
}
