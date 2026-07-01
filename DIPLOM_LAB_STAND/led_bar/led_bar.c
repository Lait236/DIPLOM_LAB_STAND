/*
 * led_bar.c
 *
 * Created: 13.05.2026 23:36:58
 *  Author: Владислав
 */ 


#include "../main.h"
#include "led_bar.h"

// отключаем jtag

static void led_bar_disable_jtag(void)
{

	MCUCR |= (1 << JTD);
	MCUCR |= (1 << JTD);
}

// инициализируем 8 светодиодов

void led_bar_init(void)
{
	led_bar_disable_jtag(); // отключаем JTAG, чтобы PC2..PC5 стали обычными выводами PORTC

	DDRC = 0xFF; // вывод PC7..PC0 становятся выходами

	PORTC = 0xFF;
	// все светодиоды выключены, так как светодиоды активны нулём:
	// 1 на пине = светодиод погашен
	// 0 на пине = светодиод горит
}

// выключаем все светодиоды

void led_bar_off(void)
{
	PORTC = 0xFF; // все пины PC7..PC0 в 1, при активном нуле это выключает все светодиоды
}

// инвертируем биты

static uint8_t led_bar_reverse_bits(uint8_t value)
{
	uint8_t result = 0; // инвертированное значение

	uint8_t i; // счётчик битов

	for (i = 0; i < 8; i++)
	{
		result <<= 1; // сдвигаем результат влево, освобождая младший бит

		if (value & 0x01)
		{
			result |= 0x01; // если младший бит исходного числа равен 1, записываем 1 в младший бит результата
		}

		value >>= 1; // сдвигаем исходное число вправо, чтобы проверить следующий бит
	}

	return result; // возвращаем число с перевёрнутым порядком битов
}

// устанавливаем значение

void led_bar_set_value(uint8_t value)
{
	uint8_t reversed_value; // значение с перевёрнутым порядком битов

	reversed_value = led_bar_reverse_bits(value);
	// разворачиваем биты:
	// значение BIT0 становится bit7
	// значение BIT1 становится bit6
	// значение BIT2 становится bit5
	// значение BIT3 становится bit4
	// значение BIT4 становится bit3
	// значение BIT5 становится bit2
	// значение BIT6 становится bit1
	// значение BIT7 становится bit0

	PORTC = ~reversed_value;

}