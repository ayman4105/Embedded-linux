#include"../inc/SevenSegment.hpp"



SevenSegment::SevenSegment(int pa, int pb, int pc, int pd,int pe, int pf, int pg)
     :a(pa, gpio_out),
      b(pb, gpio_out),
      c(pc, gpio_out),
      d(pd, gpio_out),
      e(pe, gpio_out),
      f(pf, gpio_out),
      g(pg, gpio_out)
      
{

}


void SevenSegment::write_digite(int num)
{
    clear();

    switch (num)
    {
     case 0:
        a.write_value(1);
        b.write_value(1);
        c.write_value(1);
        d.write_value(1);
        e.write_value(1);
        f.write_value(0);
        g.write_value(1);
        break;

    case 1:
        a.write_value(0);
        b.write_value(1);
        c.write_value(1);
        d.write_value(0);
        e.write_value(0);
        f.write_value(0);
        g.write_value(0);
        break;

    case 2:
        a.write_value(1);
        b.write_value(1);
        c.write_value(0);
        d.write_value(1);
        e.write_value(1);
        f.write_value(1);
        g.write_value(0);
        break;

    case 3:
        a.write_value(1);
        b.write_value(1);
        c.write_value(1);
        d.write_value(1);
        e.write_value(0);
        f.write_value(1);
        g.write_value(0);
        break;

    case 4:
        b.write_value(1);
        c.write_value(1);
        f.write_value(1);
        g.write_value(1);
        break;

    case 5:
        a.write_value(1);
        c.write_value(1);
        d.write_value(1);
        f.write_value(1);
        g.write_value(1);
        break;

    case 6:
        a.write_value(1);
        c.write_value(1);
        d.write_value(1);
        e.write_value(1);
        f.write_value(1);
        g.write_value(1);
        break;

    case 7:
        a.write_value(1);
        b.write_value(1);
        c.write_value(1);
        break;

    case 8:
        a.write_value(1);
        b.write_value(1);
        c.write_value(1);
        d.write_value(1);
        e.write_value(1);
        f.write_value(1);
        g.write_value(1);
        break;

    case 9:
        a.write_value(1);
        b.write_value(1);
        c.write_value(1);
        d.write_value(1);
        f.write_value(1);
        g.write_value(1);
        break;

    default:
        clear();
        break;
    }
}


void SevenSegment :: clear(){
    a.write_value(0);
    b.write_value(0);
    c.write_value(0);
    d.write_value(0);
    e.write_value(0);
    f.write_value(0);
    g.write_value(0);
}
