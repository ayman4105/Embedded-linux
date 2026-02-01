#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#define SERVICE_ID  0x1234
#define INSTANCE_ID 0x0020
#define METHOD_ID   0x0001

int main() {
    auto rtm = vsomeip::runtime::get();
    auto app = rtm->create_application("CapsClient");
    app->init();

    app->register_state_handler([&](vsomeip::state_type_e e) {
        if (e == vsomeip::state_type_e::ST_REGISTERED) {
            app->request_service(SERVICE_ID, INSTANCE_ID);
            std::cout << "Client registered\n";
        }
    });

    bool caps_state = false;

    app->register_availability_handler(SERVICE_ID, INSTANCE_ID,[&](vsomeip::service_t s, vsomeip::instance_t i, bool avail) {
            if (avail) {
                std::cout << "[CLIENT] Service available\n";

                while (true) {
                    caps_state = !caps_state;

                    auto msg = rtm->create_message();
                    msg->set_service(s);
                    msg->set_instance(i);
                    msg->set_method(METHOD_ID);
                    msg->set_message_type(vsomeip::message_type_e::MT_REQUEST);

                    auto payload = rtm->create_payload();
                    payload->set_data({ static_cast<vsomeip::byte_t>(caps_state) });
                    msg->set_payload(payload);

                    if (caps_state)
                        std::cout << "[CLIENT] Sending CapsLock ON\n";
                    else
                        std::cout << "[CLIENT] Sending CapsLock OFF\n";

                    app->send(msg);

                    std::this_thread::sleep_for(std::chrono::seconds(2));
             }
        }
    });


    app->register_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID,[&](const std::shared_ptr<vsomeip::message> &) {
            std::cout << "Response received\n";
        });

    app->start();
    return 0;
}
