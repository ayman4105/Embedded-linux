#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cstdio> 

using std::pair;
using std::initializer_list;
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;

class file_actions
{
private:
    int *fd;
    int*& fdref = fd;
    vector<pair< string, int>> actions_t;
public:

    file_actions(const string& path){
        fd = new int ;
        fdref = fd;
        *fd = open(path.c_str(),O_RDWR);
        if (*fd < 0) {
            perror("Failed to open file");
        }
    }
    file_actions(const file_actions& copy){
        fd = copy.fd;
        fdref = copy.fdref;
        actions_t = copy.actions_t;
    }

    void register_actions(initializer_list<pair<string, int >> list){
        for (auto &&i : list)
        {
           actions_t.push_back(i); 
        }
        
    }

    void execution_actions(){
        for (auto &&i : actions_t)
        {
            if(i.first == "write"){
                write(*fdref , to_string(i.second).c_str(),1);
            }
            else if(i.first == "read"){
                char temp[256]{0};
                int n = read(*fd , temp , 256);
                cout << "read: " <<string(temp , n)<< endl; 
            }
            else if(i.first == "close"){
                if(*fd >= 0){
                    close(*fd);
                    *fd = -1;
                }
            }
        }
        
    }
    ~file_actions(){
        if(fd && *fd >= 0){
            close(*fd);
        }
        delete fd;
        fd = nullptr;
        fdref = nullptr;
    }  
};




int main(){
    string caps_lock = "/sys/class/leds/input4::capslock/brightness";
    file_actions caps_led(caps_lock);
    caps_led.register_actions({{"write" , 1} , {"close" , 0}});
    caps_led.execution_actions();
    return 0 ;
}





