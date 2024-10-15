#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BAUD 9600

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
#define ADC_PIN 4
#define LED_PIN PC5
#define LIGHT_THRESHOLD 112

// state enum
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

    // clear pins
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
        /*  for (int i = 0; i < 100; i++) // Miganie przez 10 cykli
          {
              PORTC |= (1 << PC3);  // enable yellow led (A3)
              _delay_ms(1000);      // delay 1000 ms
              PORTC &= ~(1 << PC3); // disable yellow led (A3)
              _delay_ms(1000);      // delay 1000ms
          }  */
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
    // PORTC ^= (1 << PC3); // reprezent 1 sec

    // Update the number every 1 second, except in the YELLOW state
    if (digitalCounter >= 1 && currentState != YELLOW)
    {
        display_digit(digit); // display digit

        digit--;
        if (digit < 1) // reset counter
        {
            digit = 9;
        }
        digitalCounter = 0;
    }
    else if (currentState == YELLOW)
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

////////// fotoresistor setup

void adc_init()
{
    ADMUX = (1 << REFS0);                                              // Set reference voltage to AVCC
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, set prescaler to 128
}

uint16_t read_adc(uint8_t adc_pin)
{
    // Select ADC channel (PC3 in this case)
    ADMUX = (ADMUX & 0xF0) | (adc_pin & 0x0F);
    // Start conversion
    ADCSRA |= (1 << ADSC);
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC))
        ;
    // Return ADC value
    return ADC;
}

void setup_output_pin()
{
    DDRC |= (1 << LED_PIN); // Ustaw wyjściowy pin jako wyjście (np. dla diody LED)
}

void control_device_based_on_light(uint16_t light_value)
{
    if (light_value < LIGHT_THRESHOLD)
    {
        // Turn off the LED if the light value is below the threshold
        PORTC &= ~(1 << LED_PIN);
    }
    else
    {
        // Turn on the LED if the light value is above the threshold
        PORTC |= (1 << LED_PIN);
    }
}

// Inicjalizacja portu szeregowego
void uart_init(unsigned long baud)
{
    unsigned int ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);                  // Włącz nadajnik
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Ustawienie na 8 bitów danych
}

// Funkcja wysyłająca dane przez port szeregowy
void uart_transmit(unsigned char data)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;        // Czekaj na opróżnienie bufora
    UDR0 = data; // Wyślij dane
}

// Funkcja do wysyłania tekstu (łańcuch znaków) przez UART
void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_transmit(*str++);
    }
}

// Funkcja do wysyłania wartości ADC jako ciągu znaków
void uart_send_value(uint16_t value)
{
    char buffer[10];
    sprintf(buffer, "%u", value);
    uart_send_string(buffer);
    uart_send_string("\r\n");
}

// main

int main(void)
{
    DDRD |= 0xff;
    DDRB |= 0xff;
    DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << LED_PIN);

    // Initialize ADC and set up the LED pin
    adc_init();
    setup_output_pin();
    uart_init(115200);
    changeState(currentState); // enable initial state
    timer1_init();             // timer init

    while (1)
    {

        // Read light value from PC3 (ADC3)
        uint16_t light_value = read_adc(ADC_PIN);
        // Control the LED based on the light value
        // Wyślij wartość ADC do portu szeregowego
        uart_send_string("ADC value: ");
        uart_send_value(light_value);
        control_device_based_on_light(light_value);
        _delay_ms(500); // Delay for stability
    }

    return 0;
}