#include"../INC/Logger.hpp"


Logger::Logger()
{
    data_fd = open("/home/ayman/DHT22_sensor/log.txt",O_CREAT| O_APPEND | O_RDWR);
    if(data_fd == -1){
        perror("error");
    }

}


void Logger::log_Data(std::shared_ptr<Temperature_Data> data){

    if (!data || data_fd == -1) return; // safety check

        float temp = data->get_temprature();
        float hum = data->get_humidity();

        // format string to write
        std::string log_str = "Temperature: " + std::to_string(temp) + " °C, Humidity: " + std::to_string(hum) + " %\n";

        // write to file
        ssize_t bytes_written = write(data_fd, log_str.c_str(), log_str.size());
        if (bytes_written == -1) {
            perror("Error writing to file");
        }

}

Logger::~Logger()
{
    close(data_fd);
}