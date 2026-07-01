/*
 * servo_test.c
 *
 * Created: 14.04.2026 23:35:17
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_2/menu_board_2.h"
#include "../../servo/servo.h"
#include "servo_test.h"

static char last_servo_key = KEY_NONE; // последняя нажатая клавиша на экране теста сервы нужна для выхода по двойному нажатию 5

// отрисовываем текущий угол сервопривода

static void screen_servo_test_draw_angle(void)
{
	uint8_t angle;
	uint8_t hundreds;
	uint8_t tens;
	uint8_t ones;
	uint8_t x;

	angle = servo_get_angle(); // получаем текущий угол сервопривода

	lcd_fill_rect(0, 70, 128, 25, COLOR_BLACK); // очищаем строку, где показывается угол

	lcd_print_string(4, 74, "УГОЛ", COLOR_WHITE, COLOR_BLACK, 1); // пишем слово

	hundreds = angle / 100; // сотни

	tens = (angle / 10) % 10; // десятки

	ones = angle % 10; // единицы

	x = 40; // начальная координата для вывода числа

	if (hundreds > 0)
	{
		lcd_draw_char(x, 72, '0' + hundreds, COLOR_GREEN, COLOR_BLACK, 2);
		x += 14;
	}
	// если угол 100..180, рисуем сотни

	if (hundreds > 0 || tens > 0)
	{
		lcd_draw_char(x, 72, '0' + tens, COLOR_GREEN, COLOR_BLACK, 2);
		x += 14;
	}
	// рисуем десятки если угол меньше 10, десятки не рисуем

	lcd_draw_char(x, 72, '0' + ones, COLOR_GREEN, COLOR_BLACK, 2); // рисуем единицы
}

// открываем экран теста сервопривода

void screen_servo_test_start(void)
{
	servo_init();

	lcd_fill_color(COLOR_BLACK); // очищаем экран

	last_servo_key = KEY_NONE; // сбрасываем последнюю клавишу

	lcd_print_string(4, 4, "ТЕСТ СЕРВО", COLOR_WHITE, COLOR_BLACK, 1); // заголовок экрана

	lcd_print_string(4, 28, "ЛЕВО", COLOR_WHITE, COLOR_BLACK, 1); // подсказка: кнопка 4 двигает серву влево

	lcd_print_string(4, 44, "ПРАВО", COLOR_WHITE, COLOR_BLACK, 1); // подсказка: кнопка 6 двигает серву вправо

	lcd_print_string(4, 120, "ЦЕНТР ВЫХОД", COLOR_YELLOW, COLOR_BLACK, 1); // подсказка: двойное нажатие 5 — выход в меню

	screen_servo_test_draw_angle(); // показываем текущий угол
}

// обрабатываем кнопку на экране теста сервопривода

void screen_servo_test_key(char key)
{
	// если кнопка не нажата
	if (key == KEY_NONE)
	{
		return; // завершаем обработку без нажатой кнопки
	}

	if (key == '5' && last_servo_key == '5')
	{
		// если два раза подряд нажали 5, выходим обратно в меню

		servo_stop(); // останавливаем сервосигнал на PD7, чтобы после выхода серва не дрожала

		screen_mode = SCREEN_BOARD2_MENU; // переключаемся обратно в меню платы №2

		last_servo_key = KEY_NONE; // сбрасываем последнюю клавишу

		menu_draw(); // рисуем меню

		return;
	}

	if (key == '4')
	{
		// кнопка 4 — поворот влево

		servo_left(); // меняем угол сервопривода физически servo_right() двигает влево

		screen_servo_test_draw_angle(); // обновляем отображение угла на экране
	}

	if (key == '6')
	{
		// кнопка 6 — поворот вправо

		servo_right(); // меняем угол сервопривода физически servo_left() двигает вправо

		screen_servo_test_draw_angle(); // обновляем отображение угла на экране
	}

	last_servo_key = key; // запоминаем последнюю нажатую клавишу для выхода
}
