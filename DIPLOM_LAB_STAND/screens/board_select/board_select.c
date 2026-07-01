/*
 * board_select.c
 *
 * Created: 09.05.2026 22:28:33
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_2/menu_board_2.h"
#include "../../menus/menu_board_1/menu_board_1.h"
#include "../home/home_screen.h"
#include "board_select.h"

static uint8_t board_selected = 0; // выбранная плата
// 0 — плата номер 1
// 1 — плата номер 2
// модификатор static ограничивает область видимости файлом board_select.c
// файл main.c не изменяет выбранную плату напрямую

static char last_board_key = KEY_NONE; // последняя кнопка на экране выбора платы нужна для выхода по двойному нажатию центральной кнопки

// отрисовываем один пункт выбора платы

static void screen_board_select_draw_item(uint8_t item_index)
{
	uint8_t y;
	uint16_t bg;
	uint16_t text_color;
	
	// у первой платы строка выше, у второй платы строка ниже

	if (item_index == 0)
	{
		y = 42;
	}
	else
	{
		y = 78;
	}
	
	// если пункт выбран, рисуем его на синем фоне, если не выбран — на чёрном фоне
	
	if (item_index == board_selected)
	{
		bg = COLOR_BLUE;
		text_color = COLOR_WHITE;
	}
	else
	{
		bg = COLOR_BLACK;
		text_color = COLOR_WHITE;
	}
	

	lcd_fill_rect(0, y, 128, 26, bg); // очищаем область пункта нужным цветом

	// печатаем название платы
	
	if (item_index == 0)
	{
		lcd_print_string(8, y + 9, "ПЛАТА НОМЕР 1", text_color, bg, 1);
	}
	else
	{
		lcd_print_string(8, y + 9, "ПЛАТА НОМЕР 2", text_color, bg, 1);
	}
	
}

// полностью отрисовываем экран выбора платы

static void screen_board_select_draw(void)
{
	lcd_fill_color(COLOR_BLACK); // очищаем экран

	lcd_print_string(32, 8, "ВЫБОР ПЛАТЫ", COLOR_WHITE, COLOR_BLACK, 1); // заголовок экрана

	screen_board_select_draw_item(0); // рисуем пункт "ПЛАТА НОМЕР 1"

	screen_board_select_draw_item(1); // рисуем пункт "ПЛАТА НОМЕР 2"

	lcd_print_string(32, 132, "ЦЕНТР ВЫБОР", COLOR_YELLOW, COLOR_BLACK, 1); // подсказка
}

// открываем экран выбора платы

void screen_board_select_start(void)
{
	board_selected = 0; // по умолчанию выбираем первую плату

	screen_board_select_draw(); // рисуем экран
	
	last_board_key = KEY_NONE; // сбрасываем последнюю кнопку при входе на экран
}

// обрабатываем кнопку на экране выбора платы

void screen_board_select_key(char key)
{
	uint8_t old_selected; // старый выбранный пункт нужен, чтобы перерисовать только две строки, а не весь экран

	// если кнопка не нажата — завершаем обработку
	if (key == KEY_NONE)
	{
		return;
	}
	
	if (key == '2')
	{
		// верхняя кнопка выбираем плату номер 1

		old_selected = board_selected; // запоминаем старый выбор

		board_selected = 0; // выбираем первый пункт

		if (old_selected != board_selected)
		{
			screen_board_select_draw_item(old_selected);
			screen_board_select_draw_item(board_selected);
		}
		// если выбор изменился, перерисовываем старый и новый пункты
	}

	if (key == '8')
	{
		// нижняя кнопка выбираем плату номер 2

		old_selected = board_selected; // запоминаем старый выбор

		board_selected = 1; // выбираем второй пункт

		if (old_selected != board_selected)
		{
			screen_board_select_draw_item(old_selected);
			screen_board_select_draw_item(board_selected);
		}
		// если выбор изменился, перерисовываем старый и новый пункты
	}
	
	if (key == '5' && last_board_key == '5')
	{
		screen_mode = SCREEN_HOME;
		last_board_key = KEY_NONE;
		screen_home_start();
		return;
	}

	if (key == '5')
	{
		if (board_selected == 0)
		{
			// если выбрана плата номер 1, открываем меню платы номер 1

			screen_mode = SCREEN_BOARD1_MENU; // переключаемся в режим меню платы №1

			menu_board_1_reset(); // сбрасываем меню платы №1: выбран будет первый пункт

			menu_board_1_draw(); // рисуем меню платы №1
		}

		if (board_selected == 1)
		{
			screen_mode = SCREEN_BOARD2_MENU;

			menu_selected = 0;
			menu_top = 0;

			menu_draw();

			last_board_key = KEY_NONE;
			return;
		}
	}
}
