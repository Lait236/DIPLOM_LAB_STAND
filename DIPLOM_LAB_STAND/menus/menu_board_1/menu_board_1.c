/*
 * menu_board_1.c
 *
 * Created: 10.05.2026 21:29:41
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../font/font_5x7.h"
#include "../../lcd/lcd.h"
#include "menu_board_1.h"

// переменные меню платы №1

static uint8_t menu_board_1_selected = 0; // индекс выбранного пункта меню платы №1
// 0 — первый пункт
// 1 — второй пункт
// 2 — третий пункт

static uint8_t menu_board_1_top = 0; // индекс верхнего видимого пункта меню платы №1, нужен для скролла

// сбросить меню платы №1 в начальное состояние

void menu_board_1_reset(void)
{
	menu_board_1_selected = 0; // выбираем первый пункт

	menu_board_1_top = 0; // верхним видимым пунктом делаем первый пункт
}

// вернуть индекс выбранного пункта меню платы №1

uint8_t menu_board_1_get_selected(void)
{
	return menu_board_1_selected;
}

// нарисовать один пункт меню платы №1

void menu_board_1_draw_item(uint8_t item_index)
{
	uint8_t visible_index; // индекс пункта внутри видимой части меню например, если menu_board_1_top = 2, то item_index = 2 будет visible_index = 0

	uint8_t y; // координата Y, где рисовать пункт

	uint16_t bg; // цвет фона пункта

	uint16_t text_color; // цвет текста пункта

	if (item_index < menu_board_1_top)
	{
		// если пункт находится выше видимой области

		return; // его не отрисовываем
	}

	if (item_index >= menu_board_1_top + BOARD1_MENU_VISIBLE)
	{
		// если пункт находится ниже видимой области

		return; // его так же не отрисовываем
	}

	visible_index = item_index - menu_board_1_top; // переводим реальный индекс пункта в индекс на экране

	y = 8 + visible_index * 34; // вычисляем вертикальное положение пункта, первый пункт начинается с y = 8, каждый следующий ниже на 34 пикселя

	if (item_index == menu_board_1_selected)
	{
		// если это выбранный пункт

		bg = COLOR_BLUE; // фон выбранного пункта — синий

		text_color = COLOR_WHITE; // текст выбранного пункта — белый
	}
	else
	{
		// если пункт не выбран

		bg = COLOR_BLACK; // фон обычного пункта — чёрный

		text_color = COLOR_WHITE; // текст обычного пункта — белый
	}

	lcd_fill_rect(0, y, 128, 26, bg); // закрашиваем область пункта меню

	lcd_print_string(6, y + 10, menu_items_board_1[item_index], text_color, bg, 1); // печатаем текст пункта
	// x = 6 — небольшой отступ слева
	// y + 10 — вертикальное выравнивание внутри полоски
	// значение SCALE = 1 — обычный размер
}

// отрисовываем всё меню платы №1

void menu_board_1_draw(void)
{
	uint8_t i; // счётчик видимых пунктов

	uint8_t item_index; // реальный индекс пункта меню

	lcd_fill_color(COLOR_BLACK); // полностью очищаем экран перед полной перерисовкой меню

	for (i = 0; i < BOARD1_MENU_VISIBLE; i++)
	{
		// рисуем только видимые пункты

		item_index = menu_board_1_top + i; // получаем реальный индекс пункта

		if (item_index < BOARD1_MENU_COUNT)
		{
			// если такой пункт существует

			menu_board_1_draw_item(item_index); // рисуем этот пункт
		}
	}
}

// перемещаем выделение меню платы №1 вверх

void menu_board_1_up(void)
{
	if (menu_board_1_selected > 0)
	{
		// если выбран не первый пункт

		menu_board_1_selected--; // переходим на пункт выше
	}
	if (menu_board_1_selected < menu_board_1_top)
	{
		// если выделение ушло выше видимой области

		menu_board_1_top = menu_board_1_selected; // скроллим меню вверх
	}
}

// перемещаем выделение меню платы №1 вниз

void menu_board_1_down(void)
{
	if (menu_board_1_selected < BOARD1_MENU_COUNT - 1)
	{
		// если выбран не последний пункт

		menu_board_1_selected++; // переходим на пункт ниже
	}

	if (menu_board_1_selected >= menu_board_1_top + BOARD1_MENU_VISIBLE)
	{
		// если выделение ушло ниже видимой области

		menu_board_1_top = menu_board_1_selected - BOARD1_MENU_VISIBLE + 1; // скроллим меню вниз
	}
}
