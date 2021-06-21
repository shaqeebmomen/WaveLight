#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

class Timer
{
private:
    volatile uint32_t _ticks; // local variable to hold ticks count
    uint8_t _us_per_ovf;
    uint8_t _ticks_per_us;
    void (*_int_ovf_cb)();

public:
    Timer(void (*int_ovf_cb)());
    static Timer *timer_ptr;
    void setup(uint32_t);
    void start();
    void stop();
    uint32_t getTime();
    inline void handleOVF();
};
