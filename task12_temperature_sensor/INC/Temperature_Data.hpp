#pragma once 
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<iostream>
#include <string>
#include"../INC/DHT22_Sensor.hpp"

class Temperature_Data
{
private:
    float Temperature;
    float Humidity;
public:
    float get_temprature();
    float get_humidity();
};


