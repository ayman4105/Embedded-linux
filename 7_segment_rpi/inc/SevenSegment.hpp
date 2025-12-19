#include"../inc/gpio_27.hpp"
#include"../inc/IStream.hpp"
#include"../inc/OStream.hpp"

class SevenSegment : public IStream , public OStream
{
private:
    gpio_pin a;
    gpio_pin b;
    gpio_pin c;
    gpio_pin d;
    gpio_pin e;
    gpio_pin f;
    gpio_pin g;
public:
    SevenSegment(int pa, int pb, int pc, int pd,
                  int pe, int pf, int pg);

    void write_digite(int num) override;

    void clear();
};


