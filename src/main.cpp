// TODO top comments
/**
 * 
 * 
 * 
 * 
 */

#include <stdint.h>
#include <avr/io.h>
#include <Timer.h>
#include <AnalogConverter.h>
#include <util/delay.h>
#include <avr/wdt.h>

#define RELAY_PIN PB0
#define TRIGGER_PIN PB1
#define ECHO_PIN PB2

#define MILLIS_TO_MICROS(s) s * 1000UL

#define DISABLED_TIME MILLIS_TO_MICROS(700UL)
#define IDLE_LOOP_TIME MILLIS_TO_MICROS(40UL)
#define ARMED_TIME MILLIS_TO_MICROS(10UL)

// Software reset
void softReset()
{
    wdt_enable(WDTO_15MS);
    while (1)
        ;
}

Timer timer = Timer(&softReset);
AnalogConverter analogConverter = AnalogConverter();

// Map the pot value to a threshold from 2-60 cm
uint16_t getThres()
{
    return analogConverter.readSync() * 58U / 1024U + 2U;
}

// Ultrasonic
volatile uint32_t ultraTimer = 0;
volatile uint32_t ultraPulse = 0;
volatile bool pulseEnd = false;
volatile bool triggered = false;

void setupUltrasonic()
{
    DDRB |= _BV(TRIGGER_PIN);
    DDRB &= ~_BV(ECHO_PIN);
    PORTB &= ~_BV(ECHO_PIN);
    // Interrupt enabling
    sei();               // enable interrupts
    GIMSK |= _BV(INT0);  // external interrupt enable
    MCUCR |= _BV(ISC00); // Pin change trigger interrupt
}

void triggerUltrasonic()
{
    PORTB &= ~_BV(TRIGGER_PIN);
    _delay_us(10);
    // write pin high
    PORTB |= _BV(TRIGGER_PIN);
    _delay_us(10);
    // write pin low
    PORTB &= ~_BV(TRIGGER_PIN);
    triggered = true;
}

uint16_t readUltrasonic()
{
    pulseEnd = false;
    triggerUltrasonic();
    while (!pulseEnd)
        ;
    return (float)ultraPulse * 0.017f;
}

ISR(INT0_vect)
{
    // On rising, start timing
    if (triggered)
    {
        ultraTimer = timer.getTime();
        triggered = false;
    }
    // on falling, save timing
    else
    {
        ultraPulse = timer.getTime() - ultraTimer;
        pulseEnd = true;
    }
}

enum
{
    IDLE,
    TRIGGERED,
    DISABLED
} currentState;

int main()
{
    // Disable watchdog timer if previously reset by it
    MCUSR = 0;
    wdt_disable();

    // Setup and start timer
    timer.setup(F_CPU);
    timer.start();
    uint32_t sysTime = 0;

    setupUltrasonic();

    // Set relay pin as output
    DDRB |= _BV(RELAY_PIN);

    while (1)
    {

        switch (currentState)
        {
        case IDLE:
            if (timer.getTime() - sysTime > IDLE_LOOP_TIME)
            {

                if (readUltrasonic() < getThres())
                {
                    currentState = TRIGGERED;
                }

                sysTime = timer.getTime();
            }
            break;

        case TRIGGERED:
            PORTB ^= _BV(RELAY_PIN); // Toggle Relay Pin
            currentState = DISABLED;
            sysTime = timer.getTime();
            break;

        case DISABLED:
            if (timer.getTime() - sysTime > DISABLED_TIME)
            {
                currentState = IDLE;
                sysTime = timer.getTime();
            }
            break;
        }
    }

    return 0;
}