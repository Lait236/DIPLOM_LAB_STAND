/*
 * menu.c
 *
 * Created: 02.04.2026 1:46:18
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../font/font_5x7.h"
#include "../../lcd/lcd.h"
#include "menu_board_2.h"

// отрисовываем один пункт меню

void menu_draw_item(uint8_t item_index)
{
	uint8_t visible_index; // индекс пункта внутри видимой части меню например, если menu_top = 2, то item_index = 2 будет visible_index = 0

	uint8_t y; // координата Y, где рисовать пункт

	uint16_t bg; // цвет фона пункта

	uint16_t text_color; // цвет текста пункта

	if (item_index < menu_top)
	{
		// если пункт находится выше видимой области

		return; // его не отрисовываем
	}

	if (item_index >= menu_top + MENU_VISIBLE)
	{
		// если пункт находится ниже видимой области

		return; // его также не отрисовываем
	}

	visible_index = item_index - menu_top; // переводим реальный индекс пункта в индекс на экране

	y = 8 + visible_index * 34; // вычисляем вертикальное положение пункта, первый пункт начинается с y = 8 каждый следующий ниже на 34 пикселя

	if (item_index == menu_selected)
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

	lcd_print_string(6, y + 10, menu_items[item_index], text_color, bg, 1); // печатаем текст пункта
	// x = 6 — небольшой отступ слева
	// y + 10 — вертикальное выравнивание внутри полоски
	// значение SCALE = 1 — обычный размер
}

// отрисовываем всё меню

void menu_draw(void)
{
	uint8_t i; // счётчик видимых пунктов

	uint8_t item_index; // реальный индекс пункта меню

	lcd_fill_color(COLOR_BLACK); // полностью очищаем экран перед полной перерисовкой меню

	for (i = 0; i < MENU_VISIBLE; i++)
	{
		// рисуем только видимые пункты

		item_index = menu_top + i; // получаем реальный индекс пункта

		if (item_index < MENU_COUNT)
		{
			// если такой пункт существует

			menu_draw_item(item_index); // рисуем этот пункт
		}
	}
}

// перемещаем выделение меню вверх

void menu_up(void)
{
	// если выбран не первый пункт
	if (menu_selected > 0)
	{
		menu_selected--; // переходим на пункт выше
	}

	if (menu_selected < menu_top)
	{
		// если выделение ушло выше видимой области

		menu_top = menu_selected; // скроллим меню вверх
	}
}

// перемещаем выделение меню вниз

void menu_down(void)
{
	if (menu_selected < MENU_COUNT - 1)
	{
		// если выбран не последний пункт

		menu_selected++; // переходим на пункт ниже
	}

	if (menu_selected >= menu_top + MENU_VISIBLE)
	{
		// если выделение ушло ниже видимой области

		menu_top = menu_selected - MENU_VISIBLE + 1; // скроллим меню вниз
	}
}
