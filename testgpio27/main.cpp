#include "gpio_27.h"
#include <unistd.h>  // for usleep

int main()
{
    gpio_pin led(27, gpio_out);

    while (1)
    {
        led.write_value(1);   // LED ON
        usleep(500000);       // 500 ms

        led.write_value(0);   // LED OFF
        usleep(500000);
    }

    return 0;
}