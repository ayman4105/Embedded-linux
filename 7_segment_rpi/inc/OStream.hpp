#ifndef OSTREAM_HPP
#define OSTREAM_HPP

#include"../inc/stream.hpp"

class OStream : virtual public stream
{
private:
    /* data */
public:
    OStream() = default;
   virtual ~OStream() = default;

    virtual void write_digite(int digit) = 0;
};





#endif /*OSTREAM_HPP*/