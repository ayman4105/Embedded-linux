#pragma once 
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<iostream>
#include <string>
#include<memory>
#include"../INC/Temperature_Data.hpp"


class Logger
{
private:
    int data_fd;
public:
    Logger();
    void log_Data(std::shared_ptr<Temperature_Data> data);
    ~Logger();
};


