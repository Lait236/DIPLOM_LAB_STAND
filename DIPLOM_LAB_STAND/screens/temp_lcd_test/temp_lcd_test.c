/*
 * temp_lcd_test.h
 *
 * Created: 21.05.2026 1:55:21
 *  Author: Владислав
 */ 

#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_2/menu_board_2.h"
#include "../../lcd16x2/lcd1602.h"
#include "../../max31865/max31865.h"
#include "temp_lcd_test.h"

#define TEMP_COLOR_COLD   0 // холодно

#define TEMP_COLOR_NORMAL 1 // нормальная температура

#define TEMP_COLOR_HOT    2 // горячо или ошибка датчика

static char last_temp_key = KEY_NONE; // последняя кнопка для выхода по 55

static uint8_t temp_update_tick = 0; // счётчик обновления температуры

static uint8_t temp_last_color = 255; // последний цветовой режим 255 — невозможное значение,тчтобы при первом запуске экран точно залился цветом

static int16_t temp_last_value = MAX31865_ERROR_TEMP; // последняя отображённая температура

// определяем длину строки

static uint8_t temp_string_len(char text[])
{
	uint8_t len;

	len = 0;

	while (text[len] != '\0')
	{
		len++;
	}

	return len;
}

// форматируем температуру для LCD 16x2

static void temp_format_c10(int16_t temp_c10, char text[])
{
	uint8_t i;
	uint16_t value;
	uint16_t whole;
	uint8_t frac;

	i = 0;

	if (temp_c10 == MAX31865_ERROR_TEMP)
	{
		text[0] = 'E';
		text[1] = 'R';
		text[2] = 'R';
		text[3] = '\0';

		return;
	}
	// если датчик вернул ошибку, выводим ERR

	if (temp_c10 < 0)
	{
		text[i++] = '-';
		value = (uint16_t)(-temp_c10);
	}
	else
	{
		value = (uint16_t)temp_c10;
	}
	// значение TEMP_C10 хранит температуру в десятых долях градуса
	// например:
	// 253 = 25.3 C
	// -52 = -5.2 C

	whole = value / 10; // целая часть температуры

	frac = value % 10; // дробная часть температуры

	if (whole >= 100)
	{
		text[i++] = '0' + (whole / 100);
		text[i++] = '0' + ((whole / 10) % 10);
		text[i++] = '0' + (whole % 10);
	}
	else if (whole >= 10)
	{
		text[i++] = '0' + (whole / 10);
		text[i++] = '0' + (whole % 10);
	}
	else
	{
		text[i++] = '0' + whole;
	}

	text[i++] = '.'; // дробная часть отделяется точкой

	text[i++] = '0' + frac;

	text[i++] = ' ';
	text[i++] = 'C';

	text[i] = '\0';
}

// выбираем цветовую зону температуры

static uint8_t temp_get_color_mode(int16_t temp_c10)
{
	if (temp_c10 == MAX31865_ERROR_TEMP)
	{
		return TEMP_COLOR_HOT;
	}
	// ошибку показываем красным фоном

	if (temp_c10 < 200)
	{
		return TEMP_COLOR_COLD;
	}
	// ниже 20.0 C — холодно

	if (temp_c10 < 350)
	{
		return TEMP_COLOR_NORMAL;
	}
	// 20.0...34.9 C — нормально

	return TEMP_COLOR_HOT; // 35.0 C и выше — горячо
}

// выбираем цвет фона TFT

static uint16_t temp_get_tft_color(uint8_t color_mode)
{
	if (color_mode == TEMP_COLOR_COLD)
	{
		return COLOR_BLUE;
	}

	if (color_mode == TEMP_COLOR_NORMAL)
	{
		return COLOR_GREEN;
	}

	return COLOR_RED;
}

// выводим строку по центру LCD 16x2

static void temp_lcd1602_print_center_line(uint8_t line, char text[])
{
	uint8_t len;
	uint8_t x;

	len = temp_string_len(text);

	if (len >= 16)
	{
		x = 0;
	}
	else
	{
		x = (16 - len) / 2;
	}

	lcd1602_set_pos(0, line);
	lcd1602_print_string("                "); // очищаем строку LCD 16x2

	lcd1602_set_pos(x, line);
	lcd1602_print_string(text); // печатаем строку по центру
}

// выводим температуру на LCD 16x2

static void temp_lcd1602_print(int16_t temp_c10)
{
	char text[12];

	temp_format_c10(temp_c10, text);

	temp_lcd1602_print_center_line(0, "MAX31865 PT100"); // название устройства на первой строке

	temp_lcd1602_print_center_line(1, text); // температура на второй строке
}

// форматируем температуру для TFT

static void temp_format_tft_c10(int16_t temp_c10, char text[])
{
	uint8_t i;
	uint16_t value;
	uint16_t whole;
	uint8_t frac;

	i = 0;

	if (temp_c10 == MAX31865_ERROR_TEMP)
	{
		text[0] = 'О';
		text[1] = 'Ш';
		text[2] = 'И';
		text[3] = 'Б';
		text[4] = '\0';

		return;
	}

	if (temp_c10 < 0)
	{
		text[i++] = '-';
		value = (uint16_t)(-temp_c10);
	}
	else
	{
		value = (uint16_t)temp_c10;
	}

	whole = value / 10;
	frac = value % 10;

	if (whole >= 100)
	{
		text[i++] = '0' + (whole / 100);
		text[i++] = '0' + ((whole / 10) % 10);
		text[i++] = '0' + (whole % 10);
	}
	else if (whole >= 10)
	{
		text[i++] = '0' + (whole / 10);
		text[i++] = '0' + (whole % 10);
	}
	else
	{
		text[i++] = '0' + whole;
	}

	text[i++] = '.'; 
	text[i++] = '0' + frac;
	text[i] = '\0';
}

// выводим температуру на TFT

static void temp_tft_draw(int16_t temp_c10)
{
	char text[12];
	uint8_t color_mode;
	uint16_t bg;
	uint8_t len;
	uint8_t x;

	temp_format_tft_c10(temp_c10, text); // для TFT получаем строку без латинской C

	color_mode = temp_get_color_mode(temp_c10);
	bg = temp_get_tft_color(color_mode);

	if (color_mode != temp_last_color)
	{
		lcd_fill_color(bg); // меняем фон только если изменилась цветовая зона

		temp_last_color = color_mode;

		lcd_print_string(4, 4, "МАХ31865", COLOR_WHITE, bg, 1); // Заголовок
	
		lcd_print_string(4, 20, "РТ100", COLOR_WHITE, bg, 1); // тип датчика

		lcd_print_string(4, 136, "2Р ЦЕНТР ВЫХОД", COLOR_WHITE, bg, 1); // подсказка выхода
	}

	lcd_fill_rect(0, 64, 128, 42, bg); // очищаем только область температуры

	len = temp_string_len(text); // считаем длину строки температуры

	if ((len * 12) >= 128)
	{
		x = 0;
	}
	else
	{
		x = (128 - len * 12) / 2;
	}
	// значение SCALE = 2, один символ примерно 12 пикселей

	lcd_print_string(x, 76, text, COLOR_WHITE, bg, 2); // используем стандартную функцию вывода на TFT
}

// обновляем показания температуры

static void temp_update(void)
{
	int16_t temp_c10;

	temp_c10 = max31865_read_temp_c10();

	if (temp_c10 != temp_last_value || temp_last_color == 255)
	{
		temp_tft_draw(temp_c10);
		temp_lcd1602_print(temp_c10);

		temp_last_value = temp_c10;
	}
}

// открываем экран температуры

void screen_temp_lcd_test_start(void)
{
	lcd_fill_color(COLOR_BLACK); // очищаем TFT

	last_temp_key = KEY_NONE;
	temp_update_tick = 0;
	temp_last_color = 255;
	temp_last_value = MAX31865_ERROR_TEMP;

	lcd1602_init(); // включаем LCD 16x2

	max31865_init(); // инициализируем MAX31865

	temp_update(); // первое измерение и вывод
}

// обрабатываем кнопки на экране температуры

void screen_temp_lcd_test_key(char key)
{
	if (key == KEY_NONE)
	{
		_delay_ms(100);

		temp_update_tick++;

		if (temp_update_tick >= 5)
		{
			// примерно раз в 500 мс обновляем температуру

			temp_update_tick = 0;
			temp_update();
		}

		return;
	}

	if (key == '5' && last_temp_key == '5')
	{
		lcd1602_off(); // отключаем LCD 16x2

		screen_mode = SCREEN_BOARD2_MENU; // возвращаемся в меню платы №2

		last_temp_key = KEY_NONE;

		menu_draw(); // рисуем меню платы №2

		return;
	}

	last_temp_key = key;
}



