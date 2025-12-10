#pragma once
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<iostream>
#include <string>

class DHT22_Sensor
{
private:
    int temperature_fd;
    int humidity_fd;
public:
    DHT22_Sensor();
    float read_Tempature();
    float read_Humidity();
    ~DHT22_Sensor();
};


