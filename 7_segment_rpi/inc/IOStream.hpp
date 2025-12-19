#ifndef IOSTREAM_HPP
#define IOSTREAM_HPP

#include"../inc/IStream.hpp"
#include"../inc/OStream.hpp"


class IOStream : public IStream , public OStream
{
private:
    /* data */
public:
    IOStream(/* args */);
    ~IOStream();
     void write_digite(int digit);
};






#endif/*IOSTREAM_HPP*/