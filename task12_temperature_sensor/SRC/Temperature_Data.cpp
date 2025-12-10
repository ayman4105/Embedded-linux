#include "../INC/Temperature_Data.hpp"


float Temperature_Data:: get_temprature(){
    DHT22_Sensor sensor;          
    Temperature = sensor.read_Tempature();  
    return (Temperature / 1000.0f) ;
}

float Temperature_Data:: get_humidity(){
    DHT22_Sensor sensor;          
    Humidity = sensor.read_Humidity();  
    return (Humidity / 1000.0f) ;
}

