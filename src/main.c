#include <avr/io.h>
#include <util/delay.h>

//// state machine //////

// def states
typedef enum
{
    red,
    green,
    yellow,

} State;

// def time
#define red_time 5000
#define green_time 3000
#define yellow_time 1000

void changeState(State currentState)
{
    switch (currentState)
    {
    case red:
        PORTD |= (1 << PD7);
        PORTD &= ~((1 << PD6) | (1 << PB5));

        break;

    case green:
        PORTD |= (1 << PD6);
        PORTD &= ~((1 << PD7) | (1 << PB5));

        break;
    case yellow:
        PORTD |= (1 << PB5);
        PORTD &= ~((1 << PD6) | (1 << PD7));
        break;

    default:
        break;
    }
}

int main(void)
{

    // config pins
    DDRD |= ((1 << PD7) | (1 << PD6) | (1 << PD5));

    State currentState = red;

    uint16_t startTime = 0;
    uint16_t currentTime = 0;

    while (1)
        // read current time
        currentTime++;
    _delay_ms(1);
    {
        switch (currentState)
        {
        case red:
            if (currentTime - startTime >= red_time)
            {
                currentState = green;
                startTime = currentTime;
            }
            break;
        case green:
            if (currentTime - startTime >= green_time)
            {
                currentState = yellow;
                startTime = currentTime;
            }
            break;
        case yellow:
            if (currentTime - startTime >= yellow_time)
            {
                currentState = red;
                startTime = currentTime;
            }
            break;

        default:
            break;
        }
        changeState(currentState);
    }
    return 0;
}