
#include"../INC/DHT22_Sensor.hpp"



DHT22_Sensor::DHT22_Sensor(){
    temperature_fd = open("/sys/bus/iio/devices/iio:device0/in_temp_input",O_RDONLY);
    if(temperature_fd == -1){
        perror("error");
    }

    humidity_fd = open("/sys/bus/iio/devices/iio:device0/in_humidityrelative_input",O_RDONLY);
    if(humidity_fd == -1){
        perror("error");
    }
}


float DHT22_Sensor::read_Tempature(){
    char buff[10] = {0};
    lseek(humidity_fd, 0, SEEK_SET);
    read(humidity_fd ,buff , 10);
    int value = std::atoi(buff); 
    return value ; 
}


float DHT22_Sensor::read_Humidity(){
    char buff[10] = {0};
    lseek(temperature_fd, 0, SEEK_SET);
    read(temperature_fd ,buff , 10);
    int value = std::atoi(buff); 
    return value;

}

DHT22_Sensor::~DHT22_Sensor(){
    close(temperature_fd);
    close(humidity_fd);
}