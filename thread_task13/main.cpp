#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <chrono>
#include <condition_variable>
#include<future>

class file_actions
{
private:
    int *fd;

public:
    file_actions(const std::string& path) {
        fd = new int;
        *fd = open(path.c_str(), O_RDWR);
        if (*fd < 0) {
            perror("Failed to open file");
        }
    }

    int read_byte(char &c) {
        char temp;
        int n = read(*fd, &temp, 1);
        if(n > 0){
            c = temp;
        }
        lseek(*fd, 0, SEEK_SET); // reset pointer for next read
        return n;
    }

    void write_byte(int val) {
        std::string s = std::to_string(val);
        write(*fd, s.c_str(), s.size());
        lseek(*fd, 0, SEEK_SET); // reset pointer after write
    }

    ~file_actions() {
        if(fd && *fd >= 0){
            close(*fd);
        }
        delete fd;
    }
};


std::mutex mx;
std::condition_variable cv;
bool state_changed = false; // flag to indicate LED state changed
char current_state = '0';

void write_log(const std::string &msg) {
    std::lock_guard<std::mutex> lock(mx);
    std::ofstream logfile("log1.txt", std::ios::app);
    if(logfile.is_open()) {
        logfile << msg << std::endl;
    }
}


// void monitor_caps(file_actions &led) {
//     char last_state = '0';
//     while(true){
//         std::unique_lock<std::mutex> lock(mx);
//         cv.wait(lock, [](){ return state_changed; }); // wait for signal
//         state_changed = false; // reset flag

//         if(current_state != last_state){ // detect change
//             if(current_state == '1'){
//                 std::cout << "Caps Lock turned ON" << std::endl;
//                 write_log("Caps Lock turned ON");
//             } else {
//                 std::cout << "Caps Lock turned OFF" << std::endl;
//                 write_log("Caps Lock turned OFF");
//             }
//             last_state = current_state;
//         }
//     }
// }


// Monitor thread
void monitor_caps(file_actions &led) {
    char state;
    char last_state = '0';
    while(true){
        if(led.read_byte(state) > 0){
            if(state != last_state){ // detect change
                if(state == '1'){
                    std::cout << "Caps Lock turned ON" << std::endl;
                    write_log("Caps Lock turned ON");
                } else {
                    std::cout << "Caps Lock turned OFF" << std::endl;
                    write_log("Caps Lock turned OFF");
                }
                last_state = state;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

int main() {
    std::string caps_lock_path = "/sys/class/leds/input4::capslock/brightness";
    file_actions caps_led(caps_lock_path); // open file for read/write

    // Start monitor thread
    std::thread monitorThread(monitor_caps, std::ref(caps_led));



    // Main thread toggles LED 5 times
    for(int i = 0; i < 5; i++) {
        {
            std::lock_guard<std::mutex> lock(mx);
            current_state = '1'; // turn LED ON
            state_changed = true;
        }
        cv.notify_one();
        caps_led.write_byte(1); // actually turn LED ON
        std::this_thread::sleep_for(std::chrono::seconds(1));

        {
            std::lock_guard<std::mutex> lock(mx);
            current_state = '0'; // turn LED OFF
            state_changed = true;
        }
        cv.notify_one();
        caps_led.write_byte(0); // actually turn LED OFF
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


    // // Main thread: toggle LED ON/OFF
    // for(int i = 0; i < 5; i++){
    //     std::cout << "Main thread running..." << std::endl;
    //     caps_led.write_byte(1); // turn LED ON
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    //     caps_led.write_byte(0); // turn LED OFF
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    monitorThread.detach(); // let monitor thread run independently

    return 0;
}
