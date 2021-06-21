#include <AnalogConverter.h>

AnalogConverter::AnalogConverter()
{
    setup();
}

// Setup adc
void AnalogConverter::setup()
{
    // Select input for ADC (ADC2) from mux
    ADMUX |= _BV(MUX1);
    // Turn on ADC, set prescaler (32)
    ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0);
    // Disable digital input buffer for pin
    DIDR0 |= _BV(ADC2D);
}

// Read adc in a blocking fashion
uint16_t AnalogConverter::readSync()
{
    // Trigger a read
    ADCSRA |= _BV(ADSC);
    // wait for read flag to finish
    while (ADCSRA & (1 << ADSC))
        ; //Wait for conversion to complete.
    uint8_t low = ADCL;
    uint8_t high = ADCH;

    return (high << 8) | low;
}
