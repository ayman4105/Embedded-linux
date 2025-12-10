#pragma once 
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<iostream>
#include <string>
#include<memory>
#include"../INC/Temperature_Data.hpp"


class Display
{
private:
    float Temp;
    float Hum;
public:
    void Temperature(std::shared_ptr<Temperature_Data> data);  
    void Humidity(std::shared_ptr<Temperature_Data> data);
};


