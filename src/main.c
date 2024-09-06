#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

//// state machine //////

#define SEG_A (1 << PD6)
#define SEG_B (1 << PD7)
#define SEG_C (1 << PB0)
#define SEG_D (1 << PB1)
#define SEG_E (1 << PB2)
#define SEG_F (1 << PB3)
#define SEG_G (1 << PB4)
#define DISABLE_D PORTD &= ~(SEG_A | SEG_B)
#define DISABLE_B PORTB &= ~(SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)

// def states
typedef enum
{
    RED,
    GREEN,
    YELLOW
} State;

volatile uint16_t digitalCounter = 0;
volatile uint16_t secondCounter = 0;
volatile uint8_t digit = 9;
volatile State currentState = RED;
volatile uint8_t yellowCounter = 0;
volatile uint8_t inYellowState = 0;

void display_digit(int digit)
{

    // Wyłącz segmenty wyświetlacza przed aktualizacją
    DISABLE_B;
    DISABLE_D;

    switch (digit)
    {
        /* case 0:
             PORTD = SEG_A | SEG_B;
             PORTB = SEG_C | SEG_D | SEG_E | SEG_F;
             break;   */
    case 1:
        PORTD = SEG_B;
        PORTB = SEG_C;
        break;
    case 2:
        PORTD = SEG_A | SEG_B;
        PORTB = SEG_D | SEG_E | SEG_G;
        break;
    case 3:
        PORTD = SEG_A | SEG_B;
        PORTB = SEG_C | SEG_D | SEG_G;
        break;
    case 4:
        PORTD = SEG_B;
        PORTB = SEG_C | SEG_F | SEG_G;
        break;
    case 5:
        PORTD = SEG_A;
        PORTB = SEG_C | SEG_D | SEG_F | SEG_G;
        break;
    case 6:
        PORTD = SEG_A;
        PORTB = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
        break;
    case 7:
        PORTD = SEG_A | SEG_B;
        PORTB = SEG_C;
        break;
    case 8:
        PORTD = SEG_A | SEG_B;
        PORTB = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
        break;
    case 9:
        PORTD = SEG_A | SEG_B;
        PORTB = SEG_C | SEG_D | SEG_F | SEG_G;
        break;
    default:
        PORTD = 0;
        PORTB = 0;
        break;
    }
}

void changeState(State currentState)
{
    // enable pins
    DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2);

    // reset  pins
    PORTC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2));

    switch (currentState)
    {
    case RED:
        PORTC |= (1 << PC0);
        _delay_ms(1000); // enable red led (A0)
        break;
    case GREEN:
        PORTC |= (1 << PC1);
        _delay_ms(1000); // enable green led (A1)
        break;
    case YELLOW:
        PORTC |= (1 << PC2);
        _delay_ms(1000); // enable yellow led (A2)

        break;
    default:
        break;
    }
}

// conf Timer1
void timer1_init()
{
    TCCR1A = 0; // Normal mode (CTC bez PWM)
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode, prescaler 1024
    OCR1A = 15624;                                      // Compare match value for 1 second
    TIMSK1 |= (1 << OCIE1A);                            // Enable compare interrupt for Timer1
    sei();                                              // Enable global interrupts
}

ISR(TIMER1_COMPA_vect)
{

    digitalCounter++;
    secondCounter++;
    PORTC ^= (1 << PC3); // reprezent 1 sec

    // Update the number every 1 second, except in the YELLOW state
    if (digitalCounter >= 1 && currentState != YELLOW)
    {
        display_digit(digit); // display digit
        PORTC ^= (1 << PC3);
        digit--;
        if (digit < 1) // reset counter
        {
            digit = 9;
            
        }
        digitalCounter = 0;
    }   else if (currentState == YELLOW)
    {
        DISABLE_B;
        DISABLE_D;
    }
        // change state
        if (currentState == YELLOW)
        {
           
            // change state  3sec for yellow,
        
            if (secondCounter >= 3)
            {
                currentState = RED;
                changeState(currentState);
                secondCounter = 0;
            }
        }
        else
        {
            // change state  9sec for green red,
            if (secondCounter >= 9)
            {
                if (currentState == RED)
                {
                    currentState = GREEN;
                }
                else if (currentState == GREEN)
                {
                    currentState = YELLOW;
                }

                changeState(currentState);
                secondCounter = 0; // reset counter
            }
        }
    }


int main(void)
{
    DDRD |= 0xff;
    DDRB |= 0xff;
    DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3);

    changeState(currentState); // enable initial state
    timer1_init();             // timer init
                               // set_sleep_mode(SLEEP_MODE_IDLE);
    while (1)
    {
    }

    return 0;
}