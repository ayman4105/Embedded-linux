#include"../INC/Display.hpp"


    
  
void Display::Temperature(std::shared_ptr<Temperature_Data> data){
    if(data){
        Temp = data->get_temprature();
        std::cout<< "Temperature: " << Temp << " °C" << std::endl;
    }
    else {
        std::cout << "Error: Temperature_Data pointer is null!" << std::endl;
    }

}

void Display::Humidity(std::shared_ptr<Temperature_Data> data){
     if(data){
        Hum = data->get_humidity();
        std::cout<< "Humidity: " << Hum << " °C" << std::endl;
    }
    else {
        std::cout << "Error: Humidity_Data pointer is null!" << std::endl;
    }

}