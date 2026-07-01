/*
 * seven_segment.c
 *
 * Created: 10.05.2026 21:07:34
 *  Author: Владислав
 */ 

#include "../main.h"
#include "seven_segment.h"
#include <util/atomic.h>

#define SEG_PORT PORTA // порт сегментов

#define SEG_DDR  DDRA // регистр направления порта сегментов

#define DIGIT_PORT PORTB // порт управления разрядами

#define DIGIT_DDR  DDRB // регистр направления порта разрядов

#define DIGIT_UNITS_PIN     PB0 // разряд единиц

#define DIGIT_TENS_PIN      PB1 // разряд десятков

#define DIGIT_HUNDREDS_PIN  PB2 // разряд сотен

#define DIGIT_THOUSANDS_PIN PB3 // разряд тысяч

#define DIGIT_UNITS_MASK     (1 << DIGIT_UNITS_PIN) // маска разряда единиц

#define DIGIT_TENS_MASK      (1 << DIGIT_TENS_PIN) // маска разряда десятков

#define DIGIT_HUNDREDS_MASK  (1 << DIGIT_HUNDREDS_PIN) // маска разряда сотен

#define DIGIT_THOUSANDS_MASK (1 << DIGIT_THOUSANDS_PIN) // маска разряда тысяч

#define DIGITS_MASK (DIGIT_UNITS_MASK | DIGIT_TENS_MASK | DIGIT_HUNDREDS_MASK | DIGIT_THOUSANDS_MASK) // общая маска всех четырёх разрядов

// таблица цифр
// биты соответствуют сегментам
// коды сегментов рассчитаны для активного нуля
// 0 = сегмент горит
// 1 = сегмент выключен
// если цифры будут отображаться неправильно,
// значит порядок подключения сегментов на PORTA отличается,
// и нужно будет переставить биты в этой таблице

static const uint8_t seg_table[10] =
{
	0b11000000, // 0
	0b11111001, // 1
	0b10100100, // 2
	0b10110000, // 3
	0b10011001, // 4
	0b10010010, // 5
	0b10000010, // 6
	0b11111000, // 7
	0b10000000, // 8
	0b10010000 // 9
};

// переменные мультиплексирования

static volatile uint8_t seg_units = 0; // цифра единиц

static volatile uint8_t seg_tens = 0; // цифра десятков

static volatile uint8_t seg_hundreds = 0; // цифра сотен

static volatile uint8_t seg_thousands = 0; // цифра тысяч

static volatile uint8_t seg_current_digit = 0; // текущий разряд, который сейчас обновляет таймер

static volatile uint8_t seg_enabled = 0; // 0 — индикатор выключен, 1 — индикатор работает

// выключаем все разряды

static void seven_segment_digits_off(void)
{
	DIGIT_PORT |= DIGITS_MASK; // так как разряды активны нулём,
	// единица на PB0..PB3 выключает все разряды
}

// включаем один разряд

static void seven_segment_digit_on(uint8_t digit)
{
	if (digit == 0)
	{
		DIGIT_PORT &= ~(1 << DIGIT_UNITS_PIN);
	}
	// включаем разряд единиц

	if (digit == 1)
	{
		DIGIT_PORT &= ~(1 << DIGIT_TENS_PIN);
	}
	// включаем разряд десятков

	if (digit == 2)
	{
		DIGIT_PORT &= ~(1 << DIGIT_HUNDREDS_PIN);
	}
	// включаем разряд сотен

	if (digit == 3)
	{
		DIGIT_PORT &= ~(1 << DIGIT_THOUSANDS_PIN);
	}
	// включаем разряд тысяч
}

// выводим одну цифру на сегменты

static void seven_segment_write_digit(uint8_t digit)
{
	if (digit < 10)
	{
		SEG_PORT = seg_table[digit];
	}
	else
	{
		SEG_PORT = 0xFF;
	}
	// если цифра 0..9 — выводим её, если значение неправильное — гасим сегменты
}

// настраиваем Timer0 для мультиплексирования

static void seven_segment_timer_init(void)
{
	TCCR0A = 0; // очищаем регистр настройки Timer0 A

	TCCR0B = 0; // очищаем регистр настройки Timer0 B

	TCNT0 = 0; // сбрасываем счётчик

	OCR0A = 124; // значение сравнения для периода 1 мс

	TCCR0A |= (1 << WGM01); // включаем режим CTC, таймер считает до OCR0A

	TCCR0B |= (1 << CS01) | (1 << CS00); // делитель Timer0 = 64

	TIMSK0 |= (1 << OCIE0A); // разрешаем прерывание по совпадению с OCR0A
}

// инициализируем семисегментный модуль

void seven_segment_init(void)
{
	SEG_DDR = 0xFF; // весь PORTA делаем выходом, на нём сидят сегменты индикатора

	SEG_PORT = 0xFF; // все сегменты выключены

	DIGIT_DDR |= DIGITS_MASK; // вывод PB0..PB3 делаем выходами, это управление транзисторами разрядов

	seven_segment_digits_off(); // все разряды выключены

	seven_segment_set_number(0); // по умолчанию число 0000

	seg_enabled = 0; // после инициализации индикатор выключен, пока явно не вызовем seven_segment_enable()

	seven_segment_timer_init(); // запускаем таймер мультиплексирования
}

// включаем отображение

void seven_segment_enable(void)
{
	seg_enabled = 1; // разрешаем таймеру включать разряды
}

// выключаем отображение

void seven_segment_disable(void)
{
	seg_enabled = 0; // запрещаем отображение

	TIMSK0 &= ~(1 << OCIE0A); // запрещаем прерывание Timer0 Compare A

	TCCR0A = 0; // сбрасываем настройки Timer0 A

	TCCR0B = 0; // останавливаем Timer0

	TIFR0 = (1 << OCF0A); // сбрасываем флаг совпадения Timer0 Compare A

	seven_segment_digits_off(); // выключаем все разряды

	SEG_PORT = 0xFF; // гасим все сегменты

	DIGIT_DDR &= ~DIGITS_MASK; // освобождаем PB0..PB3

	SEG_DDR = 0x00; // освобождаем PORTA
}

// выводим число 0..9999

void seven_segment_set_number(uint16_t num)
{
	num %= 10000; // ограничиваем число диапазоном 0..9999

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		seg_units = num % 10; // единицы

		seg_tens = (num / 10) % 10; // десятки

		seg_hundreds = (num / 100) % 10; // сотни

		seg_thousands = (num / 1000) % 10; // тысячи
	}
}

// задаём цифры вручную

void seven_segment_set_digits(uint8_t thousands, uint8_t hundreds, uint8_t tens, uint8_t units)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		seg_thousands = thousands;
		seg_hundreds = hundreds;
		seg_tens = tens;
		seg_units = units;
	}
}

// обрабатываем прерывание Timer0

ISR(TIMER0_COMPA_vect)
{
	if (seg_enabled == 0)
	{
		seven_segment_digits_off();
		SEG_PORT = 0xFF;
		return;
	}
	// если индикатор выключен, держим все разряды и сегменты погашенными

	seven_segment_digits_off(); // перед переключением цифры выключаем все разряды

	if (seg_current_digit == 0)
	{
		seven_segment_write_digit(seg_units);
		seven_segment_digit_on(0);
		seg_current_digit = 1;
	}
	else if (seg_current_digit == 1)
	{
		seven_segment_write_digit(seg_tens);
		seven_segment_digit_on(1);
		seg_current_digit = 2;
	}
	else if (seg_current_digit == 2)
	{
		seven_segment_write_digit(seg_hundreds);
		seven_segment_digit_on(2);
		seg_current_digit = 3;
	}
	else
	{
		seven_segment_write_digit(seg_thousands);
		seven_segment_digit_on(3);
		seg_current_digit = 0;
	}
}
