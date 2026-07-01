/*
 * home_screen.c
 *
 * Created: 09.05.2026 22:28:07
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../board_select/board_select.h"
#include "home_screen.h"

// дополнительные цвета главного экрана

#define HOME_COLOR_CYAN       0x07FF // голубой цвет дорожек

#define HOME_COLOR_DARK_GRAY  0x2104 // тёмно-серый цвет корпуса микросхемы

#define HOME_COLOR_GRAY       0x8410 // серый цвет рамки и ножек

#define HOME_COLOR_LIGHT_GRAY 0xC618 // светло-серый цвет бликов

// настройки микросхемы

#define HOME_CHIP_X 46 // координата X корпуса микросхемы
#define HOME_CHIP_Y 58 // координата Y корпуса микросхемы
#define HOME_CHIP_W 36 // ширина корпуса микросхемы
#define HOME_CHIP_H 50 // высота корпуса микросхемы

#define HOME_PIN_W 6 // ширина ножки микросхемы
#define HOME_PIN_H 4 // высота ножки микросхемы

// отрисовываем одну дорожку слева от микросхемы

static void screen_home_draw_left_signal(uint8_t y)
{
	lcd_fill_rect(8, y, 14, 2, HOME_COLOR_CYAN);
	lcd_fill_rect(22, y - 6, 2, 8, HOME_COLOR_CYAN);
	lcd_fill_rect(24, y - 6, 8, 2, HOME_COLOR_CYAN);
	lcd_fill_rect(32, y - 6, 2, 8, HOME_COLOR_CYAN);
	lcd_fill_rect(34, y, 12, 2, HOME_COLOR_CYAN);

	lcd_draw_pixel(7, y, COLOR_WHITE);
	lcd_draw_pixel(7, y + 1, HOME_COLOR_CYAN);
}

// отрисовываем одну дорожку справа от микросхемы

static void screen_home_draw_right_signal(uint8_t y)
{
	lcd_fill_rect(82, y, 12, 2, HOME_COLOR_CYAN);
	lcd_fill_rect(94, y - 6, 2, 8, HOME_COLOR_CYAN);
	lcd_fill_rect(96, y - 6, 8, 2, HOME_COLOR_CYAN);
	lcd_fill_rect(104, y - 6, 2, 8, HOME_COLOR_CYAN);
	lcd_fill_rect(106, y, 14, 2, HOME_COLOR_CYAN);

	lcd_draw_pixel(121, y, COLOR_WHITE);
	lcd_draw_pixel(121, y + 1, HOME_COLOR_CYAN);
}

// отрисовываем все дорожки

static void screen_home_draw_signals(void)
{
	screen_home_draw_left_signal(66);
	screen_home_draw_left_signal(76);
	screen_home_draw_left_signal(86);
	screen_home_draw_left_signal(96);
	screen_home_draw_left_signal(106);

	screen_home_draw_right_signal(66);
	screen_home_draw_right_signal(76);
	screen_home_draw_right_signal(86);
	screen_home_draw_right_signal(96);
	screen_home_draw_right_signal(106);
}

// отрисовываем ножки микросхемы

static void screen_home_draw_chip_pins(void)
{
	uint8_t y;
	uint8_t i;

	for (i = 0; i < 5; i++)
	{
		y = 64 + i * 10;
		
		// левая и правая ножки
		lcd_fill_rect(HOME_CHIP_X - HOME_PIN_W, y, HOME_PIN_W, HOME_PIN_H, HOME_COLOR_GRAY);
		lcd_fill_rect(HOME_CHIP_X + HOME_CHIP_W, y, HOME_PIN_W, HOME_PIN_H, HOME_COLOR_GRAY); 

		lcd_draw_pixel(HOME_CHIP_X - 1, y, HOME_COLOR_LIGHT_GRAY);
		lcd_draw_pixel(HOME_CHIP_X + HOME_CHIP_W, y, HOME_COLOR_LIGHT_GRAY); 
	}
}

// отрисовываем корпус микросхемы

static void screen_home_draw_chip_body(void)
{
	lcd_fill_rect(HOME_CHIP_X, HOME_CHIP_Y, HOME_CHIP_W, HOME_CHIP_H, HOME_COLOR_DARK_GRAY); // корпус микросхемы

	// рамка микросхемы
	lcd_fill_rect(HOME_CHIP_X, HOME_CHIP_Y, HOME_CHIP_W, 2, HOME_COLOR_LIGHT_GRAY);
	lcd_fill_rect(HOME_CHIP_X, HOME_CHIP_Y + HOME_CHIP_H - 2, HOME_CHIP_W, 2, HOME_COLOR_GRAY);
	lcd_fill_rect(HOME_CHIP_X, HOME_CHIP_Y, 2, HOME_CHIP_H, HOME_COLOR_LIGHT_GRAY);
	lcd_fill_rect(HOME_CHIP_X + HOME_CHIP_W - 2, HOME_CHIP_Y, 2, HOME_CHIP_H, HOME_COLOR_GRAY); 

	lcd_draw_pixel(HOME_CHIP_X + 7, HOME_CHIP_Y + 8, COLOR_BLACK);
	lcd_draw_pixel(HOME_CHIP_X + 8, HOME_CHIP_Y + 7, COLOR_BLACK);
	lcd_draw_pixel(HOME_CHIP_X + 8, HOME_CHIP_Y + 8, COLOR_BLACK);
	lcd_draw_pixel(HOME_CHIP_X + 9, HOME_CHIP_Y + 8, HOME_COLOR_GRAY);
	lcd_draw_pixel(HOME_CHIP_X + 8, HOME_CHIP_Y + 9, HOME_COLOR_GRAY);
}

// отрисовываем микросхему целиком

static void screen_home_draw_chip(void)
{
	screen_home_draw_signals(); // дорожки

	screen_home_draw_chip_pins(); // ножки микросхемы

	screen_home_draw_chip_body(); // корпус микросхемы
}

// открываем главный экран

void screen_home_start(void)
{
	lcd_fill_color(COLOR_BLACK); // очищаем экран чёрным цветом

	lcd_print_string(43, 8, "ГЛАВНЫЙ", COLOR_WHITE, COLOR_BLACK, 1); // первая строка заголовка

	lcd_print_string(49, 24, "ЭКРАН", COLOR_WHITE, COLOR_BLACK, 1); // вторая строка заголовка

	screen_home_draw_chip(); // рисуем статичную микросхему

	lcd_print_string(34, 136, "ЦЕНТР МЕНЮ", COLOR_YELLOW, COLOR_BLACK, 1); // подсказка по центру
}

// обрабатываем кнопку на главном экране

void screen_home_key(char key)
{
	if (key == KEY_NONE)
	{
		return; // если кнопка не нажата, ничего не обновляем
	}

	if (key == '5')
	{
		screen_mode = SCREEN_BOARD_SELECT; // меняем текущий режим экрана

		screen_board_select_start(); // рисуем экран выбора платы
	}
}
