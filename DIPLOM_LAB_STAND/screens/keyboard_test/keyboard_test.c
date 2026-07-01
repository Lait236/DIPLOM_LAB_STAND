/*
 * keyboard_test.c
 *
 * Created: 02.04.2026 2:03:21
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_2/menu_board_2.h"
#include "../../keyboard/keyboard.h"
#include "keyboard_test.h"


uint8_t input_x = 0; // координата X текущего символа в тесте клавиатуры

uint8_t input_y = 30; // координата Y текущего символа в тесте клавиатуры

char last_test_key = KEY_NONE; // последняя нажатая клавиша в тесте клавиатуры нужна, чтобы двойное нажатие 5 работало как выход

// открываем экран теста клавиатуры

void screen_keypad_test_start(void)
{
	lcd_fill_color(COLOR_BLACK); // очищаем экран

	input_x = 0; // ставим курсор ввода в начало строки

	input_y = 30; // ставим курсор ниже заголовка

	last_test_key = KEY_NONE; // сбрасываем последнюю клавишу

	lcd_print_string(4, 4, "ТЕСТ КЛАВИАТУРЫ", COLOR_WHITE, COLOR_BLACK, 1); // печатаем заголовок экрана
}

// обрабатываем экран теста клавиатуры

void screen_keypad_test_key(char nav_key)
{
	char keypad_key; // символ, реально нажатый на клавиатуре 4x3

	if (nav_key == '5' && last_test_key == '5')
	{
		// если два раза подряд нажали центральную кнопку стенда, выходим обратно в меню платы №2

		screen_mode = SCREEN_BOARD2_MENU; // возвращаемся именно в меню платы №2, а не на главный экран

		last_test_key = KEY_NONE; // сбрасываем последнюю управляющую кнопку

		menu_draw(); // рисуем меню платы №2

		return; // выходим из функции
	}

	if (nav_key != KEY_NONE)
	{
		// если была нажата любая управляющая кнопка стенда,
		// запоминаем её только для проверки двойного нажатия

		last_test_key = nav_key;
	}

	keypad_key = keypad_get_key(); // читаем клавиатуру 4x3

	if (keypad_key == KEY_NONE)
	{
		// если на клавиатуре 4x3 ничего не нажато, ничего не рисуем

		return;
	}

	lcd_draw_char(input_x, input_y, keypad_key, COLOR_GREEN, COLOR_BLACK, 2); // рисуем нажатую клавишу клавиатуры 4x3

	input_x += 14; // сдвигаем позицию следующего символа вправо

	if (input_x > 110)
	{
		// если дошли до правого края экрана

		input_x = 0; // возвращаем курсор в начало

		input_y += 18; // переволим курсор на следующую строку
	}

	if (input_y > 140)
	{
		// если дошли до нижнего края экрана

		lcd_fill_rect(0, 25, 128, 135, COLOR_BLACK); // очищаем область ввода, но не трогаем верхний заголовок

		input_x = 0; // возвращаем курсор в начало

		input_y = 30; // возвращаем курсор под заголовок
	}
}
