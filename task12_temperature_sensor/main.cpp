#include<iostream>
#include <memory>
#include"INC/DHT22_Sensor.hpp"
#include"INC/Display.hpp"
#include"INC/Logger.hpp"
#include"INC/Temperature_Data.hpp"





int main(){


    std::shared_ptr<Temperature_Data> tempData = std::make_shared<Temperature_Data>();

    
    Display display;
    Logger logger;

    
    for (int i = 0; i < 5; i++) {

        display.Temperature(tempData);            // display temperature
        display.Humidity(tempData);               // display humidity
        logger.log_Data(tempData);                // log data

        sleep(1); 
    }

    return 0;
}