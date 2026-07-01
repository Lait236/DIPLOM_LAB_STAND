/*
 * nav_buttons.c
 *
 * Created: 09.05.2026 22:06:19
 *  Author: Владислав
 */ 

#include "../main.h"
#include "nav_buttons.h"

// кнопки подключены к GND
// внутренние подтяжки включены
// не нажата -> на входе 1
// нажата    -> на входе 0

static char nav_buttons_read_raw(void)
{
	if (!(PIND & (1 << BTN_UP)))
	{
		return '2'; // верхняя кнопка заменяет старую клавишу 2
	

	if (!(PIND & (1 << BTN_DOWN)))
	{
		return '8'; // нижняя кнопка заменяет старую клавишу 8
	}
	
	if (!(PIND & (1 << BTN_LEFT)))
	{
		return '4'; // левая кнопка заменяет старую клавишу 4
	}
	
	if (!(PIND & (1 << BTN_RIGHT)))
	{
		return '6'; // правая кнопка заменяет старую клавишу 6
	}
	
	if (!(PIND & (1 << BTN_CENTER)))
	{
		return '5'; // центральная кнопка заменяет старую клавишу 5
	}
	
	return KEY_NONE; // ничего не нажато
}

void nav_buttons_init(void)
{
	DDRD &= ~((1 << BTN_LEFT) | (1 << BTN_UP) | (1 << BTN_CENTER) | (1 << BTN_DOWN) | (1 << BTN_RIGHT)); // выводы PD2..PD6 делаем входами

	PORTD |= (1 << BTN_LEFT) | (1 << BTN_UP) | (1 << BTN_CENTER) | (1 << BTN_DOWN) | (1 << BTN_RIGHT); // включаем внутренние подтяжки
}

char nav_buttons_get_key(void)
{
	char key;

	key = nav_buttons_read_raw(); // читаем кнопки

	if (key == KEY_NONE)
	{
		return KEY_NONE; // если ничего не нажато выходим
	}
	
	_delay_ms(20); // антидребезг

	if (nav_buttons_read_raw() != key)
	{
		return KEY_NONE; // если через 20 мс кнопка изменилась, считаем это дребезгом
	}
	
	while (nav_buttons_read_raw() != KEY_NONE); // ждём отпускания кнопки

	_delay_ms(20); // антидребезг при отпускании

	return key; // возвращаем виртуальную клавишу
}