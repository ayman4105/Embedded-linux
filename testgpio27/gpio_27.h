#pragma once

#include <string>

typedef enum{
    gpio_in = 0,  //read
    gpio_out = 1  // write
}gpio_mode_t;

class gpio_pin {
    private:
        int fd;
        int pin_num;
        gpio_mode_t mode;
    public:
        gpio_pin(int num, gpio_mode_t _mode);
        ~gpio_pin();

        int read_value();
        void write_value(int value);
};
