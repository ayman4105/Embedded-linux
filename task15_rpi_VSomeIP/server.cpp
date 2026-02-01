
#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#define SERVICE_ID  0x1234
#define INSTANCE_ID 0x0020
#define METHOD_ID   0x0001

std::mutex mx;
std::condition_variable cv;
bool state_changed = false;
char current_state = '0';

class file_actions {
private:
    std::string path;

public:
    file_actions(const std::string &p) : path(p) {}

    void write_byte(int val) {
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "failed to open file\n";
            return;
        }

        file << val;
        file.close();
    }
};



void monitor_caps() {
    char last = '0';

    while (true) {
        std::unique_lock<std::mutex> lock(mx);
        cv.wait(lock, [] { return state_changed; });
        state_changed = false;

        if (current_state != last) {
            if (current_state == '1')
                std::cout << "[MONITOR] Caps Lock ON detected\n";
            else
                std::cout << "[MONITOR] Caps Lock OFF detected\n";

            last = current_state;
        }
    }
}


int main() {
    file_actions caps_led("/sys/class/leds/input4::capslock/brightness");

    std::thread monitor_thread(monitor_caps);

    auto rtm = vsomeip::runtime::get();
    auto app = rtm->create_application("CapsServer");
    app->init();

    app->register_state_handler([&](vsomeip::state_type_e e) {
        if (e == vsomeip::state_type_e::ST_REGISTERED) {
            app->offer_service(SERVICE_ID, INSTANCE_ID);
            std::cout << "Service offered\n";
        }
    });

app->register_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID,[&](const std::shared_ptr<vsomeip::message> &req) {

        std::cout << "\n[SERVER] Request received\n";

        auto data = req->get_payload()->get_data();
        int val = data[0];

        std::cout << "[SERVER] Payload value = " << val << std::endl;

        {
            std::lock_guard<std::mutex> lock(mx);

            if (val == 1) {
                std::cout << "[SERVER] Turning CapsLock ON\n";
                current_state = '1';
                caps_led.write_byte(1);
            } else {
                std::cout << "[SERVER] Turning CapsLock OFF\n";
                current_state = '0';
                caps_led.write_byte(0);
            }

            state_changed = true;
        }

        cv.notify_one();

        auto resp = rtm->create_response(req);
        app->send(resp);

        std::cout << "[SERVER] Response sent\n";
    });


    app->start();

    monitor_thread.join();
    return 0;
}
