/*
 * led_bar_test.c
 *
 * Created: 13.05.2026 23:39:04
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_1/menu_board_1.h"
#include "../../led_bar/led_bar.h"
#include "led_bar_test.h"

#define LED_BAR_AUTO_DELAY_TICKS 10 // скорость счёта

static char last_led_bar_key = KEY_NONE; // последняя нажатая управляющая кнопка нужна для выхода по двойному нажатию 5

static uint8_t led_bar_value = 0; // текущее значение светодиодов, счёт идёт от 0 до 255

static uint8_t led_bar_auto_tick = 0; // счётчик задержки автоматического счёта

static uint8_t led_bar_pause = 0; // 0 — счёт идёт, 1 — пауза

// отрисовываем двоичное значение на TFT

static void screen_led_bar_test_draw_binary(void)
{
	uint8_t i;
	uint8_t mask;
	uint8_t x;

	lcd_fill_rect(0, 88, 128, 24, COLOR_BLACK); // очищаем строку двоичного значения

	x = 16; // начальная позиция вывода битов

	mask = 0x80; // начинаем со старшего бита значение BIT7 соответствует PC7 и первому светодиоду

	for (i = 0; i < 8; i++)
	{
		if (led_bar_value & mask)
		{
			lcd_draw_char(x, 88, '1', COLOR_GREEN, COLOR_BLACK, 1);
		}
		else
		{
			lcd_draw_char(x, 88, '0', COLOR_WHITE, COLOR_BLACK, 1);
		}

		x += 12;

		mask >>= 1; 
	}
}

// обновляем значение на TFT

static void screen_led_bar_test_draw_value(void)
{
	screen_led_bar_test_draw_binary(); // показываем число в двоичном виде

}

// выполняем автоматический счёт от 0 до 255

static void screen_led_bar_test_auto_update(void)
{
	// если стоит пауза
	if (led_bar_pause)
	{
		return; // завершаем обработку во время паузы
	}

	_delay_ms(5); // задержка автоматического счёта

	led_bar_auto_tick++; // увеличиваем счётчик задержки

	if (led_bar_auto_tick < LED_BAR_AUTO_DELAY_TICKS)
	{

		return;
	}

	led_bar_auto_tick = 0; // сбрасываем счётчик задержки

	led_bar_set_value(led_bar_value); // выводим текущее значение на светодиоды

	screen_led_bar_test_draw_value(); // обновляем отображение на TFT

	led_bar_value++; // увеличиваем значение, так как led_bar_value — uint8_t, после 255 он сам переполнится в 0
}

// открываем экран теста светодиодов

void screen_led_bar_test_start(void)
{
	lcd_fill_color(COLOR_BLACK); // очищаем TFT-экран

	last_led_bar_key = KEY_NONE; // сбрасываем последнюю кнопку

	led_bar_value = 0; // начинаем счёт с 0

	led_bar_auto_tick = 0; // сбрасываем задержку автосчёта

	led_bar_pause = 0; // счёт сразу запущен

	led_bar_init(); // настраиваем PORTC под светодиоды

	led_bar_set_value(led_bar_value); // выводим 00000000

	lcd_print_string(4, 4, "ТЕСТ ДИОДОВ", COLOR_WHITE, COLOR_BLACK, 1); // заголовок экрана

	lcd_print_string(4, 28, "СЧЕТ ОТ 0 ДО 255", COLOR_WHITE, COLOR_BLACK, 1); // подсказка

	lcd_print_string(4, 44, "ЛЕВО ПАУЗА", COLOR_WHITE, COLOR_BLACK, 1); // правая кнопка — пауза / продолжить

	lcd_print_string(4, 60, "ПРАВО СБРОС", COLOR_WHITE, COLOR_BLACK, 1); // левая кнопка — сбросить счётчик

	lcd_print_string(4, 144, "ДВА РАЗА ЦЕНТР ВЫХОД", COLOR_YELLOW, COLOR_BLACK, 1); // двойное нажатие центральной кнопки — выход

	screen_led_bar_test_draw_value(); // рисуем начальное значение на TFT
}

// обрабатываем кнопку на экране теста светодиодов

void screen_led_bar_test_key(char key)
{
	screen_led_bar_test_auto_update(); // счёт должен идти даже тогда, когда кнопки не нажимаются

	// если кнопка не нажата, дальше ничего не обрабатываем
	if (key == KEY_NONE)
	{
		return;
	}
	if (key == '5' && last_led_bar_key == '5')
	{
		// если два раза подряд нажали центральную кнопку, выходим обратно в меню платы номер 1

		led_bar_off(); // выключаем все светодиоды

		screen_mode = SCREEN_BOARD1_MENU; // возвращаемся в меню платы номер 1

		last_led_bar_key = KEY_NONE; // сбрасываем последнюю кнопку

		menu_board_1_draw(); // рисуем меню платы номер 1

		return;
	}

	if (key == '4')
	{
		// левая кнопка — сброс счёта

		led_bar_value = 0; // возвращаемся к 0

		led_bar_auto_tick = 0; // сбрасываем задержку

		led_bar_set_value(led_bar_value); // выводим 0 на светодиоды

		screen_led_bar_test_draw_value(); // обновляем TFT
	}

	if (key == '6')
	{
		// правая кнопка — пауза / продолжить

		if (led_bar_pause)
		{
			led_bar_pause = 0;
		}
		else
		{
			led_bar_pause = 1;
		}
	}

	last_led_bar_key = key; // запоминаем последнюю кнопку, это нужно для двойного нажатия 5
}
