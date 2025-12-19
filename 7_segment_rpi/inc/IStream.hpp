#ifndef ISTREAM_HPP
#define ISTREAM_HPP

#include"../inc/stream.hpp"


class IStream : virtual public stream
{
private:

public:
    IStream() = default;
    virtual ~IStream()= default;


    int read_digit();
};


#endif /*ISTREAM_HPP*/