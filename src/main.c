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

// def states
typedef enum
{
    RED,
    GREEN,
    YELLOW
} State;

volatile uint16_t digitalCounter = 0;
volatile uint16_t stateCounter = 0;
volatile uint8_t digit = 0;
volatile State currentState = RED;

void display_digit(int digit)
{

    // Wyłącz segmenty wyświetlacza przed aktualizacją
    PORTD &= ~(SEG_A | SEG_B);
    PORTB &= ~(SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);

    switch (digit)
    {
    case 0:
        PORTD = SEG_A | SEG_B;
        PORTB = SEG_C | SEG_D | SEG_E | SEG_F;
        break;
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
    // Ustawienie pinów A0, A1, A2 jako wyjścia (odpowiadają PC0, PC1, PC2)
    DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2);

    // Resetowanie wszystkich pinów (wyłączanie wszystkich diod)
    PORTC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2));

    switch (currentState)
    {
    case RED:
        PORTC |= (1 << PC0); // Włącz diodę czerwoną (A0)
        break;
    case GREEN:
        PORTC |= (1 << PC1); // Włącz diodę zieloną (A1)
        break;
    case YELLOW:
        PORTC |= (1 << PC2); // Włącz diodę żółtą (A2)
        break;
    default:
        break;
    }
}

// conf Timer1
void timer1_init()
{
    TCCR1A = 0;                                        // Normal mode
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode, prescaler 1024
    OCR1A = 31249;                                     // Compare match value for 1 second
    TIMSK1 = (1 << OCIE1A);                            // Enable compare interrupt
    sei();                                             // Enable global interrupts
}

ISR(TIMER1_COMPA_vect)
{
    digitalCounter++;
    stateCounter++;

    // update the number  every 1 second
    if (digitalCounter < 1000)
    {
        display_digit(digit);
        digit = (digit + 1) % 10;
        digitalCounter = 0;
    }

    // update state traffic light
    if (stateCounter < 1000)
    { // 9 sec every state
        if (currentState == RED)
        {
            currentState = GREEN;
        }
        else if (currentState == GREEN)
        {
            currentState = YELLOW;
        }
        else if (currentState == YELLOW)
        {
            currentState = RED;
        }
        changeState(currentState);
        stateCounter = 0; // Reset licznika
    }

    // update the state every 9 seconds)
}

int main(void)
{
    DDRD |= 0xff;
    DDRB |= 0xff;

    changeState(currentState); // enable initial state
    timer1_init();             // timer init
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (1)
    {
        sleep_mode();
    }

    return 0;
}