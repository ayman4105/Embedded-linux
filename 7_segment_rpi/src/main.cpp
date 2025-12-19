#include"iostream"

#include"../inc/IOStream.hpp"
#include"../inc/SevenSegment.hpp"

int main(){

   SevenSegment seg(17, 18, 27, 22, 23, 24, 25);  

    while(1){
        int x = seg.read_digit(); 
        seg.write_digite(x);
    }
    
    return 0;
}