#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

class AnalogConverter
{
private:
public:
    AnalogConverter();
    void setup();
    uint16_t readSync();
};
