#include <Arduino.h>
#include <avr/io.h>
#include "lcd.h"

#define PIN_CS 10
#define PIN_DC 9
#define PIN_RST 8
#define PIN_SS 53

#define CMD_SWRESET 0x01
#define CMD_SLPOUT 0x11
#define CMD_DISPON 0x29
#define CMD_CASET 0x2A
#define CMD_PASET 0x2B
#define CMD_RAMWR 0x2C
#define CMD_MADCTL 0x36
#define CMD_PIXFMT 0x3A

#define MADCTL_LANDSCAPE 0x28
#define PIXFMT_16BIT 0x55

static void spi_send(uint8_t byte)
{
    SPDR = byte;
    while (!(SPSR & (1 << SPIF))) {
    }
}

static void lcd_command(uint8_t command)
{
    digitalWrite(PIN_DC, LOW);
    spi_send(command);
    digitalWrite(PIN_DC, HIGH);
}

static void lcd_command_byte(uint8_t command, uint8_t value)
{
    lcd_command(command);
    spi_send(value);
}

static void send_range(uint8_t command, int first, int last)
{
    lcd_command(command);
    spi_send(first >> 8);
    spi_send(first & 0xFF);
    spi_send(last >> 8);
    spi_send(last & 0xFF);
}

void lcd_init(void)
{
    pinMode(PIN_CS, OUTPUT);
    pinMode(PIN_DC, OUTPUT);
    pinMode(PIN_RST, OUTPUT);
    pinMode(PIN_SS, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SCK, OUTPUT);

    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = 1 << SPI2X;

    digitalWrite(PIN_CS, LOW);
    digitalWrite(PIN_RST, LOW);
    delay(10);
    digitalWrite(PIN_RST, HIGH);
    delay(120);

    lcd_command(CMD_SWRESET);
    delay(150);
    lcd_command(CMD_SLPOUT);
    delay(120);
    lcd_command_byte(CMD_PIXFMT, PIXFMT_16BIT);
    lcd_command_byte(CMD_MADCTL, MADCTL_LANDSCAPE);
    lcd_command(CMD_DISPON);
}

void lcd_push_rows(int top, int rows, const uint16_t *pixels)
{
    send_range(CMD_CASET, 0, LCD_W - 1);
    send_range(CMD_PASET, top, top + rows - 1);
    lcd_command(CMD_RAMWR);

    long count = (long)LCD_W * rows;
    for (long i = 0; i < count; i++) {
        spi_send(pixels[i] >> 8);
        spi_send(pixels[i] & 0xFF);
    }
}
