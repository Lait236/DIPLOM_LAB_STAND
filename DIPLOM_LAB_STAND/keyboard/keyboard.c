/*
 * keyboard.c
 *
 * Created: 02.04.2026 1:49:12
 *  Author: Владислав
 */ 


#include "../main.h"
#include "keyboard.h"

// все функции, объявленные через static,
// являются служебными и не подразумевают вызов извне

#define KEYPAD_COL1 PA0 // первый столбец клавиатуры
#define KEYPAD_COL2 PA1 // второй столбец клавиатуры
#define KEYPAD_COL3 PA2 // третий столбец клавиатуры
#define KEYPAD_ROW1 PA3 // первая строка клавиатуры
#define KEYPAD_ROW2 PA4 // вторая строка клавиатуры
#define KEYPAD_ROW3 PA5 // третья строка клавиатуры
#define KEYPAD_ROW4 PA6 // четвёртая строка клавиатуры

// настройка портов клавиатуры

void keypad_init(void)
{
	DDRA |= (1 << KEYPAD_COL1) | (1 << KEYPAD_COL2) | (1 << KEYPAD_COL3); // вывод PA0, PA1, PA2 устанавливаем на вход	- это столбцы клавиатуры

	PORTA |= (1 << KEYPAD_COL1) | (1 << KEYPAD_COL2) | (1 << KEYPAD_COL3); // устанавливаем все столбцы в 1 - это неактивное состояние

	DDRA &= ~((1 << KEYPAD_ROW1) | (1 << KEYPAD_ROW2) | (1 << KEYPAD_ROW3) | (1 << KEYPAD_ROW4)); // вывод PA4, PA5, PA6, PA7 устанавливаем на вход - это строки клавиатуры

	PORTA |= (1 << KEYPAD_ROW1) | (1 << KEYPAD_ROW2) | (1 << KEYPAD_ROW3) | (1 << KEYPAD_ROW4); // устанавливаем подтяжку на строки	|| пока кнопка не нажата, вход читает 1
}

// устанавливаем все столбцы клавиатуры в 1

static void keypad_all_columns_high(void)
{
	PORTA |= (1 << KEYPAD_COL1) | (1 << KEYPAD_COL2) | (1 << KEYPAD_COL3); // вывод PA0 = 1, PA1 = 1, PA2 = 1
}

// выбираем один столбец клавиатуры

static void keypad_select_column(uint8_t column)
{
	keypad_all_columns_high(); // сначала все столбцы устанавливаем в 1
	
	// если выбран столбец 0, опускаем PA0 в 0
	if (column == 0)
	{
		PORTA &= ~(1 << KEYPAD_COL1);
	}
	
	// если выбран столбец 1, опускаем PA1 в 0
	if (column == 1)
	{
		PORTA &= ~(1 << KEYPAD_COL2);
	}

	// если выбран столбец 2, опускаем PA2 в 0
	if (column == 2)
	{
		PORTA &= ~(1 << KEYPAD_COL3);
	}
	
}

// прочитать строки клавиатуры
// вследствие подтяжки:
// 1 — кнопка не нажата
// 0 — кнопка нажата

static uint8_t keypad_read_row(void)
{
	if (!(PINA & (1 << KEYPAD_ROW1)))
	{
		return 0; // если PA4 стал 0, нажата строка 0
	}
	if (!(PINA & (1 << KEYPAD_ROW2)))
	{
		return 1; // если PA5 стал 0, нажата строка 1
	}
	
	if (!(PINA & (1 << KEYPAD_ROW3)))
	{
		return 2; // если PA6 стал 0, нажата строка 2
	}
	
	if (!(PINA & (1 << KEYPAD_ROW4)))
	{
		return 3; // если PA7 стал 0, нажата строка 3
	}

	return 255; // если ни одна строка не нажата
}

// получаем одну нажатую клавишу, если ничего не нажато, возвращаем KEY_NONE

char keypad_get_key(void)
{
	uint8_t column; // номер проверяемого столбца

	uint8_t row; // номер найденной строки

	for (column = 0; column < 3; column++)
	{
		// по очереди проверяем 3 столбца

		keypad_select_column(column); // делаем выбранный столбец равным 0
		_delay_us(5); // задержка

		row = keypad_read_row(); // проверяем, какая строка стала 0

		if (row != 255)
		{
			// если найдена нажатая строка

			_delay_ms(20); // антидребезг - ждём 20 мс

			if (keypad_read_row() == row)
			{
				// проверяем, что кнопка всё ещё нажата

				while (keypad_read_row() != 255);
				
				_delay_ms(20); // антидребезг при отпускании

				keypad_all_columns_high(); // возвращаем все столбцы в 1

				return keymap[row][column]; // возвращаем символ из таблицы keymap
			}
		}
	}

	keypad_all_columns_high(); // если ничего не нашли, всё равно возвращаем столбцы в 1

	return KEY_NONE; // кнопка не нажата
}

// быстрое чтение клавиатуры без ожидания отпускания, нужно для режимов, где кнопка должна работать при удержании

static char keypad_scan_raw(void)
{
	uint8_t column;
	uint8_t row;

	for (column = 0; column < 3; column++)
	{
		keypad_select_column(column); // выбираем очередной столбец клавиатуры

		_delay_us(5);

		row = keypad_read_row(); // читаем строки клавиатуры

		if (row != 255)
		{
			// если какая-либо строка стала 0, значит кнопка нажата

			keypad_all_columns_high(); // возвращаем все столбцы в 1

			return keymap[row][column]; // возвращаем символ нажатой кнопки
		}
	}

	keypad_all_columns_high(); // если ничего не нажато, тоже возвращаем все столбцы в 1

	return KEY_NONE; // кнопка не нажата
}

// специальное чтение клавиатуры
// кнопки 4 и 6 разрешено удерживать
// остальные кнопки возвращаются один раз за нажатие

char keypad_get_key_stepper(void)
{
	static char last_key = KEY_NONE; // последняя прочитанная клавиша

	char key; // текущая прочитанная клавиша

	key = keypad_scan_raw(); // читаем клавиатуру без ожидания отпускания кнопки

	if (key == KEY_NONE)
	{
		// если кнопка не нажата

		last_key = KEY_NONE; // сбрасываем память о последней клавише

		return KEY_NONE; // ничего не возвращаем
	}

	_delay_ms(5); //  задержка

	if (key != keypad_scan_raw())
	{
		// если через 5 мс клавиша уже другая или исчезла, значит это мог быть дребезг

		return KEY_NONE; // игнорируем такое нажатие
	}

	if (key == '4' || key == '6')
	{
		// кнопки 4 и 6 разрешаем возвращать постоянно, пока кнопка удерживается

		last_key = key; // запоминаем последнюю клавишу

		return key; // возвращаем 4 или 6 каждый раз, пока кнопка зажата
	}

	if (key != last_key)
	{
		// если это новая клавиша, которую раньше не держали

		last_key = key; // запоминаем данную клавишу

		return key; // возвращаем один раз
	}

	return KEY_NONE; // если это не 4 и не 6, и кнопка всё ещё удерживается, повторно её не возвращаем
}
