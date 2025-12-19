#include"../inc/IStream.hpp"

int IStream :: read_digit(){

    int digit;
    while(1){
        std :: cin >> digit;

        if((digit < 0) || (digit > 9)){
            std::cout <<"worng number" <<std::endl;
            std::cout <<"agin" <<std::endl;
        }
        else{
            return digit;
        }
    }
}