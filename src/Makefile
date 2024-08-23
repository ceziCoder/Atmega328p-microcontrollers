# MCU and tools
MCU = atmega328p
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Port and programmer settings
PORT = /dev/ttyUSB0
PROGRAMMER = arduino
BAUDRATE = 115200

# Compiler and linker options
CFLAGS = -mmcu=$(MCU) -Os -Wall -DF_CPU=16000000UL
LDFLAGS = -mmcu=$(MCU)

# Output file name
TARGET = main

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET).hex

# Compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linking
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

# Convert ELF to HEX
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Flashing the program onto the microcontroller
flash: $(TARGET).hex
	$(AVRDUDE) -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUDRATE) -U flash:w:$<:i

# Cleaning up generated files
clean:
	rm -f src/*.o $(TARGET).elf $(TARGET).hex
