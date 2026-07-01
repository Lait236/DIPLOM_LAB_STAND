/*
 * lcd1602.c
 *
 * Created: 20.05.2026 1:52:18
 *  Author: Владислав
 */ 


#include "../main.h"
#include "lcd1602.h"

// назначение выводов LCD 16x2

#define LCD1602_PORT PORTC
#define LCD1602_DDR  DDRC

#define LCD1602_RS PC2
#define LCD1602_E  PC3

#define LCD1602_D4 PC4
#define LCD1602_D5 PC5
#define LCD1602_D6 PC6
#define LCD1602_D7 PC7

#define LCD1602_MASK ((1 << LCD1602_RS) | (1 << LCD1602_E) | (1 << LCD1602_D4) | (1 << LCD1602_D5) | (1 << LCD1602_D6) | (1 << LCD1602_D7))

// отключение JTAG

static void lcd1602_disable_jtag(void)
{
	uint8_t sreg;
	uint8_t mcucr_value;

	sreg = SREG; // сохраняем состояние флага глобальных прерываний

	cli();

	mcucr_value = MCUCR | (1 << JTD); // берём текущее значение MCUCR и добавляем бит JTD

	// дважды записываем JTD = 1
	MCUCR = mcucr_value;
	MCUCR = mcucr_value;

	SREG = sreg; // возвращаем состояние прерываний
}

// управление линией E

static void lcd1602_e_pulse(void)
{
	LCD1602_PORT |= (1 << LCD1602_E);
	_delay_us(50);

	LCD1602_PORT &= ~(1 << LCD1602_E);
	_delay_us(50);
}

// отправка половины байта, очищаем только линии D4..D7

static void lcd1602_send_halfbyte(uint8_t halfbyte)
{
	LCD1602_PORT &= ~((1 << LCD1602_D4) | (1 << LCD1602_D5) | (1 << LCD1602_D6) | (1 << LCD1602_D7));

	if (halfbyte & 0b00000001)
	{
		LCD1602_PORT |= (1 << LCD1602_D4);
	}

	if (halfbyte & 0b00000010)
	{
		LCD1602_PORT |= (1 << LCD1602_D5);
	}

	if (halfbyte & 0b00000100)
	{
		LCD1602_PORT |= (1 << LCD1602_D6);
	}

	if (halfbyte & 0b00001000)
	{
		LCD1602_PORT |= (1 << LCD1602_D7);
	}

	lcd1602_e_pulse();
}

// отправка байта

static void lcd1602_send_byte(uint8_t data, uint8_t mode)
{
	if (mode == 0)
	{
		LCD1602_PORT &= ~(1 << LCD1602_RS); // вывод RS = 0 — команда
	}
	else
	{
		LCD1602_PORT |= (1 << LCD1602_RS); // вывод RS = 1 — данные
	}

	lcd1602_send_halfbyte(data >> 4); // сначала старшая половина байта

	lcd1602_send_halfbyte(data & 0x0F); // потом младшая половина байта
}

// передаем команду

static void lcd1602_command(uint8_t command)
{
	lcd1602_send_byte(command, 0);
}

// передаем символ

static void lcd1602_char(uint8_t data)
{
	lcd1602_send_byte(data, 1);
}

// инициализация LCD 16x2

void lcd1602_init(void)
{
	lcd1602_disable_jtag(); // освобождаем PC2..PC5 от JTAG

	LCD1602_DDR |= LCD1602_MASK; // выводы RS, E, D4..D7 делаем выходами

	LCD1602_PORT &= ~LCD1602_MASK; // устанавливаем сначала всё в 0

	_delay_ms(15);

	lcd1602_send_halfbyte(0b00000011);
	_delay_ms(4);

	lcd1602_send_halfbyte(0b00000011);
	_delay_us(100);

	lcd1602_send_halfbyte(0b00000011);
	_delay_ms(1);

	lcd1602_send_halfbyte(0b00000010);
	_delay_ms(1);
	
	// переводим LCD в 4-битный режим

	lcd1602_command(0b00101000); // 4-битный режим, 2 строки, шрифт 5x8

	lcd1602_command(0b00001100); // дисплей включён, курсор выключен

	lcd1602_command(0b00000110); // автоинкремент адреса

	lcd1602_clear(); // очищаем экран
}

// очищаем LCD

void lcd1602_clear(void)
{
	lcd1602_command(0b00000001);
	_delay_ms(2);
}

// устанавливаем позицию

void lcd1602_set_pos(uint8_t x, uint8_t y)
{
	uint8_t address;

	if (y == 0)
	{
		address = 0x00 + x;
	}
	else
	{
		address = 0x40 + x;
	}

	lcd1602_command(0b10000000 | address);
}

// выводим строку

void lcd1602_print_string(char str[])
{
	uint8_t i;

	for (i = 0; str[i] != '\0'; i++)
	{
		lcd1602_char(str[i]);
	}
}

// выключаем LCD

void lcd1602_off(void)
{
	lcd1602_command(0b00001000); // значение DISPLAY OFF

	LCD1602_PORT &= ~LCD1602_MASK; // все линии LCD в 0

	LCD1602_DDR &= ~LCD1602_MASK; // освобождаем PC2..PC7
}
