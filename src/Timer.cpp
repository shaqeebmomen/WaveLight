#include <Timer.h>

Timer::Timer(void (*int_ovf_cb)())
{
    timer_ptr = this;
    _int_ovf_cb = int_ovf_cb;
}

// Initializes timer1 for operation
void Timer::setup(uint32_t f_clk)
{
    // _ticks_per_us = f_clk / 8000000UL;
    // _us_per_ovf = 1000000UL * 256UL / f_clk;
    sei();
    TIMSK |= _BV(TOIE1); // Enable overflow interrupt
}

// Starts timer1
void Timer::start()
{
    TCCR1 |= _BV(CS12) | _BV(CS10); // Set prescale to 8 and start clock
}

// Stops timer1 and resets internal timing values
void Timer::stop()
{
    TCCR1 &= !(_BV(CS10) | _BV(CS11) | _BV(CS12) | _BV(CS13));
    TCNT1 = 0;
    _ticks = 0UL;
}

// Returns current time in us
uint32_t Timer::getTime()
{
    uint32_t tickSum;
    // save sreg
    uint8_t oldSREG = SREG;
    // stop interrupts
    cli();
    // add timer register value to timer variable
    tickSum = _ticks + TCNT1;
    // restore sreg
    SREG = oldSREG;

    return tickSum;
}

// Called by ISR to handle overflows and update timer
inline void Timer::handleOVF()
{
    if (_ticks > UINT32_MAX - 255UL)
    {
        // Trigger callback for reset
        _int_ovf_cb();
    }
    else
    {
        _ticks += 255UL;
    }
}

Timer *Timer::timer_ptr;
ISR(TIMER1_OVF_vect)
{
    Timer::timer_ptr->handleOVF();
}