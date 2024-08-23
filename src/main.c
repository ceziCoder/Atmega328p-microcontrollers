#include <avr/io.h>
#include <util/delay.h>
//#include <avr/iom328p.h>
//#ifndef __AVR_ATmega328P__
  //  #define __AVR_ATmega328P__
//#endif

int main(void)
{
    DDRB = DDRB | (1 << PORT5);

    while(1)

        {
            PORTB = PORTB | (1 << PORTB5);

            _delay_ms(2000);

            PORTB = PORTB & ~(1 << PORTB5);

            _delay_ms(2000);
        }
    return 0;
}