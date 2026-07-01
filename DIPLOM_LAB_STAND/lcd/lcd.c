/*
 * lcd.c
 *
 * Created: 02.04.2026 1:19:55
 *  Author: Владислав
 */ 

#include "../main.h"
#include "../font/font_5x7.h"
#include "lcd.h"

// все функции объявленные через static являются служебными и не подразумевают вызов извне

// функция отправки одного байта по SPI

static inline void lcd_spi_write(uint8_t data)
{
	SPDR = data; // записываем байт в регистр данных SPI, после записи аппаратный SPI сам начинает передачу

	while (!(SPSR & (1 << SPIF))); // ждём, пока передача закончится, регистр SPIF станет 1, когда байт полностью отправлен
}

// инициализируем порты

void lcd_port_init(void)
{
	DDRB |= (1 << LCD_CS) | (1 << LCD_MOSI) | (1 << LCD_SCK);
	
	// регистр DDRD задаёт направление пинов порта D
	// делаем выходами:
	// вывод PD1 = DC
	// вывод PD0 = RST

	DDRD |= (1 << LCD_DC) | (1 << LCD_RST);
	
	PORTB |= (1 << LCD_CS); // поднимаем CS в 1 - дисплей пока не выбран
	
	// поднимаем DC и RST в 1
	// значение DC = 1 — начальное состояние "данные"
	// значение RST = 1 — сброс не активен

	PORTD |= (1 << LCD_DC) | (1 << LCD_RST);
}

// выбрать дисплей на SPI-шине

static void lcd_select(void)
{
	PORTB &= ~(1 << LCD_CS); // значение CS = 0, дисплей слушает SPI
}

// снять выбор дисплея

static void lcd_unselect(void)
{
	PORTB |= (1 << LCD_CS); // значение CS = 1, дисплей не слушает SPI
}

// отправляем один байт как команду дисплея

static void lcd_write_cmd(uint8_t cmd)
{
	PORTD &= ~(1 << LCD_DC); // значение DC = 0, следующий байт дисплей воспримет как команду

	lcd_select(); // значение CS = 0, выбираем дисплей

	lcd_spi_write(cmd); // отправляем байт команды

	lcd_unselect(); // значение CS = 1, завершаем обмен
}

// отправляем один байт как данные дисплея

static void lcd_write_data(uint8_t data)
{
	PORTD |= (1 << LCD_DC); // значение DC = 1, следующий байт дисплей воспримет как данные

	lcd_select(); // выбираем дисплей

	lcd_spi_write(data); // отправляем байт данных

	lcd_unselect(); // завершаем обмен
}

// аппаратный сброс дисплея

static void lcd_reset(void)
{
	PORTD |= (1 << LCD_RST); // значение RST = 1, сброс не активен

	_delay_ms(5); // небольшая пауза перед сбросом

	PORTD &= ~(1 << LCD_RST); // значение RST = 0, активируем сброс дисплея

	_delay_ms(20); // держим дисплей в сбросе 20 мс

	PORTD |= (1 << LCD_RST); // значение RST = 1, выводим дисплей из сброса

	_delay_ms(120); // ждём, пока контроллер дисплея запустится
}

// задаем область дисплея, куда будут записываться пиксели

static void lcd_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	x0 += LCD_XSTART; // добавляем смещение по X к начальной координате

	x1 += LCD_XSTART; // добавляем смещение по X к конечной координате

	y0 += LCD_YSTART; // добавляем смещение по Y к начальной координате

	y1 += LCD_YSTART; // добавляем смещение по Y к конечной координате

	lcd_write_cmd(0x2A); // команда CASET, задаёт диапазон столбцов X

	lcd_write_data(0x00); // старший байт начальной координаты X, у нас координаты меньше 256, поэтому старший байт 0

	lcd_write_data(x0); // младший байт начальной координаты X

	lcd_write_data(0x00); // старший байт конечной координаты X
	lcd_write_data(x1); // младший байт конечной координаты X

	lcd_write_cmd(0x2B); // команда RASET, задаёт диапазон строк Y

	lcd_write_data(0x00); // старший байт начальной координаты Y

	lcd_write_data(y0); // младший байт начальной координаты Y

	lcd_write_data(0x00); // старший байт конечной координаты Y

	lcd_write_data(y1); // младший байт конечной координаты Y

	lcd_write_cmd(0x2C); // команда RAMWR, после неё дисплей ждёт поток пикселей
}

// инициализация дисплея ST7735

void lcd_init(void)
{
	lcd_reset(); // сначала делаем аппаратный сброс

	lcd_write_cmd(0x01); // команда SWRESET — программный сброс контроллера дисплея

	_delay_ms(150); // ждём после программного сброса

	lcd_write_cmd(0x11); // команда SLPOUT — выйти из режима сна

	_delay_ms(255); // ждём после выхода из сна

	lcd_write_cmd(0xB1); // команда FRMCTR1 — настройка частоты кадров для normal mode

	lcd_write_data(0x01); // первый параметр FRMCTR1

	lcd_write_data(0x2C); // второй параметр FRMCTR1

	lcd_write_data(0x2D); // третий параметр FRMCTR1

	lcd_write_cmd(0xB2); // команда FRMCTR2 — настройка частоты кадров для idle mode

	lcd_write_data(0x01); // первый параметр FRMCTR2

	lcd_write_data(0x2C); // второй параметр FRMCTR2

	lcd_write_data(0x2D); // третий параметр FRMCTR2

	lcd_write_cmd(0xB3); // команда FRMCTR3 — настройка частоты кадров для partial mode

	lcd_write_data(0x01); // первый параметр FRMCTR3

	lcd_write_data(0x2C); // второй параметр FRMCTR3

	lcd_write_data(0x2D); // третий параметр FRMCTR3

	lcd_write_data(0x01); // четвёртый параметр FRMCTR3

	lcd_write_data(0x2C); // пятый параметр FRMCTR3

	lcd_write_data(0x2D); // шестой параметр FRMCTR3

	lcd_write_cmd(0xB4); // команда INVCTR — управление инверсией строк/точек

	lcd_write_data(0x07); // параметр INVCTR

	lcd_write_cmd(0xC0); // команда PWCTR1 — настройка питания контроллера

	lcd_write_data(0xA2); // первый параметр PWCTR1

	lcd_write_data(0x02); // второй параметр PWCTR1

	lcd_write_data(0x84); // третий параметр PWCTR1

	lcd_write_cmd(0xC1); // команда PWCTR2 — дополнительная настройка питания

	lcd_write_data(0xC5); // параметр PWCTR2

	lcd_write_cmd(0xC2); // команда PWCTR3 — настройка питания в normal mode

	lcd_write_data(0x0A); // первый параметр PWCTR3

	lcd_write_data(0x00); // второй параметр PWCTR3

	lcd_write_cmd(0xC3); // команда PWCTR4 — настройка питания в idle mode

	lcd_write_data(0x8A); // первый параметр PWCTR4

	lcd_write_data(0x2A); // второй параметр PWCTR4

	lcd_write_cmd(0xC4); // команда PWCTR5 — настройка питания в partial mode

	lcd_write_data(0x8A); // первый параметр PWCTR5

	lcd_write_data(0xEE); // второй параметр PWCTR5

	lcd_write_cmd(0xC5); // команда VMCTR1 — настройка VCOM

	lcd_write_data(0x0E); // параметр VMCTR1
	
	lcd_write_cmd(0x20); // команда INVOFF — выключить инверсию цветов

	lcd_write_cmd(0x36); // команда MADCTL — управление ориентацией и порядком цветов

	lcd_write_data(LCD_MADCTL); // передаём заранее заданное значение ориентации

	lcd_write_cmd(0x3A); // команда COLMOD — выбор формата пикселя

	lcd_write_data(0x05); // 0x05 означает 16 бит на пиксель, формат RGB565

	lcd_write_cmd(0x13); // команда NORON — нормальный режим отображения

	_delay_ms(10); // небольшая пауза после NORON

	lcd_write_cmd(0x29); // команда DISPON — включить отображение

	_delay_ms(100); // ждём после включения дисплея
}

// заливаем весь экран одним цветом

void lcd_fill_color(uint16_t color)
{
	uint32_t i; // счётчик пикселей, на экране 128 * 160 = 20480 пикселей, значение UINT32_T взято с запасом

	lcd_set_addr_window(0, 0, 127, 159); // задаём окно на весь экран

	PORTD |= (1 << LCD_DC); // значение DC = 1, дальше будут данные пикселей

	lcd_select(); // выбираем дисплей один раз перед длинной передачей

	for (i = 0; i < 128UL * 160UL; i++)
	{
		// повторяем для каждого пикселя экрана

		lcd_spi_write(color >> 8); // отправляем старший байт цвета

		lcd_spi_write(color & 0xFF); // отправляем младший байт цвета
	}

	lcd_unselect(); // завершаем передачу
}

// отрисовываем один пиксель

void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color)
{
	if (x >= 128 || y >= 160)
	{
		// если координаты вышли за экран, ничего не рисуем

		return; // выходим из функции
	}

	lcd_set_addr_window(x, y, x, y); // задаём окно размером 1x1 пиксель

	PORTD |= (1 << LCD_DC); // значение DC = 1, дальше передаём данные цвета

	lcd_select(); // выбираем дисплей

	lcd_spi_write(color >> 8); // отправляем старший байт цвета

	lcd_spi_write(color & 0xFF); // отправляем младший байт цвета

	lcd_unselect(); // завершаем передачу
}

// нарисовать символ 5x7

static void lcd_draw_5x7(uint8_t x, uint8_t y, const uint8_t *glyph, uint16_t color, uint16_t bg, uint8_t scale)
{
	uint8_t col; // номер столбца символа, символ имеет 5 столбцов + 1 пустой столбец для пробела между буквами

	uint8_t row; // номер строки символа, символ имеет 7 строк

	uint8_t sx; // счётчик масштабирования по X

	uint8_t sy; // счётчик масштабирования по Y

	uint8_t line; // один байт из шрифта, в нём закодирован вертикальный столбец символа

	uint16_t pixel_color; // цвет текущего пикселя: либо цвет буквы, либо цвет фона
	
	// задаём прямоугольное окно под символ:
	// ширина = 6 * scale - 5 столбцов буквы + 1 пустой столбец между символами
	// высота = 7 * scale
	// почему 6, а не 5?

	lcd_set_addr_window(x, y, x + 6 * scale - 1, y + 7 * scale - 1);

	PORTD |= (1 << LCD_DC); // значение DC = 1, дальше пойдут пиксельные данные

	lcd_select(); // выбираем дисплей один раз, это быстрее, чем выбирать дисплей для каждого пикселя

	for (row = 0; row < 7; row++)
	{
		// проходим по строкам символа сверху вниз
		for (sy = 0; sy < scale; sy++)
		{
			// повторяем каждую строку scale раз, так получается увеличение по высоте

			for (col = 0; col < 6; col++)
			{
				// проходим по столбцам символа, значение COL = 0..4 — настоящие столбцы буквы, значение COL = 5 — пустой столбец-пробел

				if (col < 5)
				{
					// если это один из 5 настоящих столбцов буквы

					line = pgm_read_byte(&glyph[col]); // читаем байт столбца из Flash, нельзя писать просто glyph[col], потому что glyph лежит в PROGMEM

					if (line & (1 << row))
					{
						// проверяем бит строки row, если бит = 1, значит в этом месте есть пиксель буквы

						pixel_color = color; // рисуем цветом буквы
					}
					else
					{
						// если бит = 0, значит это фон

						pixel_color = bg; // рисуем цветом фона
					}
				}
				else
				{
					// если col = 5, это пустой столбец между символами

					pixel_color = bg; // он всегда рисуется цветом фона
				}

				for (sx = 0; sx < scale; sx++)
				{
					// повторяем каждый пиксель scale раз по ширине, так получается увеличение по X

					lcd_spi_write(pixel_color >> 8); // отправляем старший байт цвета

					lcd_spi_write(pixel_color & 0xFF); // отправляем младший байт цвета
				}
			}
		}
	}

	lcd_unselect(); // заканчиваем передачу символа
}

// преводим utf8 к кириллице

static uint8_t utf8_ru_to_index(uint8_t b1, uint8_t b2)
{
	if (b1 == ' ')
	{
		// если это пробел

		return 0; // индекс пробела в font_letters равен 0
	}

	if (b1 == 0xD0)
	{
		// большинство заглавных кириллических символов в UTF-8 начинаются с байта 0xD0

		if (b2 == 0x81)
		{
			// ё в UTF-8 = 0xD0 0x81

			return 33; // ё лежит в font_letters под индексом 33
		}

		if (b2 >= 0x90 && b2 <= 0xAF)
		{
			// а..Я в UTF-8 идут как:
			// а = 0xD0 0x90
			// б = 0xD0 0x91
			// промежуточные символы идут последовательно 
			// я = 0xD0 0xAF

			// переводим код символа в индекс font_letters
			// а: 1 + (0x90 - 0x90) = 1
			// б: 1 + (0x91 - 0x90) = 2
			
			return 1 + (b2 - 0x90);
		}
	}
	return 0; // если символ неизвестен, рисуем пробел
}

// отрисовываем один ASCII-символ

void lcd_draw_char(uint8_t x, uint8_t y, char c, uint16_t color, uint16_t bg, uint8_t scale)
{
	if (c >= '0' && c <= '9')
	{
		// если символ — цифра от 0 до 9

		lcd_draw_5x7(x, y, font_digit[c - '0'], color, bg, scale);
		// берём нужную цифру из font_digit
		// например:
		// '0' - '0' = 0
		// '5' - '0' = 5
	}
	else if (c == '*')
	{
		// если символ — звёздочка

		lcd_draw_5x7(x, y, font_service_symb[0], color, bg, scale); // рисуем символ *
	}
	else if (c == '#')
	{
		// если символ — решётка

		lcd_draw_5x7(x, y,  font_service_symb[1], color, bg, scale); // рисуем символ #
	}
	else if (c == '.')
	{
		// если символ — точка

		lcd_draw_5x7(x, y, font_service_symb[2], color, bg, scale); // рисуем символ .
	}
	else
	{
		// если символ неизвестен

		lcd_draw_5x7(x, y, font_letters[0], color, bg, scale); // рисуем пробел
	}
}

// печатаем кириллическую строку на дисплее

void lcd_print_string(uint8_t x, uint8_t y, const char *text, uint16_t color, uint16_t bg, uint8_t scale)
{
	uint8_t index; // индекс символа в таблице font_ru

	uint8_t b1; // первый байт UTF-8 символа

	uint8_t b2; // второй байт UTF-8 символа

	uint16_t pos = 0; // текущая позиция в строке - это индекс байта, а не номер буквы

	while (text[pos] != 0)
	{
		// пока не дошли до конца строки

		if (text[pos] >= '0' && text[pos] <= '9')
		{
			// если текущий байт — ASCII-цифра

			lcd_draw_char(x, y, text[pos], color, bg, scale); // рисуем цифру

			pos++; // цифра занимает 1 байт

			x += 6 * scale; // сдвигаем координату X вправо на ширину символа
		}
		else if (text[pos] == '.' || text[pos] == '*' || text[pos] == '#')
		{
			// если текущий символ — служебный ASCII-символ
			// например: точка в температуре 23.5

			lcd_draw_char(x, y, text[pos], color, bg, scale); // рисуем '.', '*' или '#'

			pos++; // символ ASCII-символ занимает 1 байт

			x += 6 * scale; // сдвигаемся вправо
		}
		else if (text[pos] == ' ')
		{
			// если текущий символ — пробел

			lcd_draw_5x7(x, y, font_letters[0], color, bg, scale); // рисуем пробел из таблицы font_ru

			pos++; // пробел занимает 1 байт

			x += 6 * scale; // сдвигаемся вправо
		}
		else
		{
			// иначе считаем, что это кириллический символ UTF-8

			b1 = text[pos]; // читаем первый байт символа

			b2 = text[pos + 1]; // читаем второй байт символа

			index = utf8_ru_to_index(b1, b2); // переводим два байта UTF-8 в индекс таблицы font_letters

			lcd_draw_5x7(x, y, font_letters[index], color, bg, scale); // рисуем найденный кириллический символ

			pos += 2; // кириллический символ в UTF-8 занимает 2 байта

			x += 6 * scale; // сдвигаемся вправо
		}
	}
}

// заливаем прямоугольник одним цветом

void lcd_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
	uint16_t i; // счётчик пикселей внутри прямоугольника

	uint16_t pixels; // общее количество пикселей в прямоугольнике

	if (x >= 128 || y >= 160)
	{
		// если начало прямоугольника за пределами экрана

		return; // ничего не рисуем
	}

	if (x + w > 128) w = 128 - x; // если прямоугольник выходит за правую границу, уменьшаем ширину

	if (y + h > 160) h = 160 - y; // если прямоугольник выходит за нижнюю границу, уменьшаем высоту

	lcd_set_addr_window(x, y, x + w - 1, y + h - 1); // задаём окно под прямоугольник

	PORTD |= (1 << LCD_DC); // значение DC = 1, дальше идут данные пикселей

	lcd_select(); // выбираем дисплей

	pixels = (uint16_t)w * h; // считаем количество пикселей например, 10 * 5 = 50 пикселей

	for (i = 0; i < pixels; i++)
	{
		// отправляем цвет для каждого пикселя

		lcd_spi_write(color >> 8); // старший байт цвета

		lcd_spi_write(color & 0xFF); // младший байт цвета
	}

	lcd_unselect(); // завершаем передачу
}
