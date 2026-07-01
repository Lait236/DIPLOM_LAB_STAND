/*
 * seven_segment_test.c
 *
 * Created: 10.05.2026 21:12:50
 *  Author: Владислав
 */ 


#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_1/menu_board_1.h"
#include "../../seven_segment/seven_segment.h"
#include "seven_segment_test.h"

static char last_seven_segment_key = KEY_NONE; // последняя нажатая управляющая кнопка нужна для выхода по двойному нажатию центральной кнопки

static uint16_t seven_segment_test_value = 0; // текущее число, которое выводим на индикатор

#define SEVEN_SEGMENT_AUTO_DELAY_TICKS 20 // задержка автосчёта

static uint8_t seven_segment_auto_count = 0; // 0 — автосчёт выключен, 1 — автосчёт включен

static uint8_t seven_segment_auto_tick = 0; // счётчик задержки автосчёта нужен, чтобы число увеличивалось не слишком быстро

// отрисовываем текущее число на TFT

static void screen_seven_segment_test_draw_value(void)
{

	uint16_t value;
	uint8_t thousands;
	uint8_t hundreds;
	uint8_t tens;
	uint8_t units;

	value = seven_segment_test_value; // берём текущее значение

	thousands = (value / 1000) % 10;
	hundreds = (value / 100) % 10;
	tens = (value / 10) % 10;
	units = value % 10;

	lcd_fill_rect(0, 112, 128, 28, COLOR_BLACK); // очищаем строку значения на дисплее

	lcd_print_string(4, 118, "ЧИСЛО", COLOR_WHITE, COLOR_BLACK, 1); // подпись
	
	// показываем на TFT то же число, которое отправлено на семисегментный индикатор

	lcd_draw_char(52, 114, '0' + thousands, COLOR_GREEN, COLOR_BLACK, 2);
	lcd_draw_char(66, 114, '0' + hundreds, COLOR_GREEN, COLOR_BLACK, 2);
	lcd_draw_char(80, 114, '0' + tens, COLOR_GREEN, COLOR_BLACK, 2);
	lcd_draw_char(94, 114, '0' + units, COLOR_GREEN, COLOR_BLACK, 2); 
}

// автоматически увеличиваем число на индикаторе

static void screen_seven_segment_test_auto_update(void)
{
	if (seven_segment_auto_count == 0)
	{
		// если автосчёт выключен

		return; // завершаем обработку во время паузы
	}

	_delay_ms(5); // задержка автоматического счёта

	seven_segment_auto_tick++; // увеличиваем счётчик задержки

	if (seven_segment_auto_tick < SEVEN_SEGMENT_AUTO_DELAY_TICKS)
	{
		// если не прошло нужное количество тиков

		return; // пока число не увеличиваем
	}

	seven_segment_auto_tick = 0; // сбрасываем счётчик задержки

	if (seven_segment_test_value < 9999)
	{
		// если не дошли до максимума

		seven_segment_test_value++; // увеличиваем число на 1

		seven_segment_set_number(seven_segment_test_value); // отправляем число на семисегментный индикатор

		screen_seven_segment_test_draw_value(); // обновляем число на TFT-дисплее
	}
	else
	{
		// если дошли до 9999

		seven_segment_auto_count = 0; // останавливаем автосчёт
	}
}

// открываем экран теста семисегментных индикаторов
void screen_seven_segment_test_start(void)
{
	lcd_fill_color(COLOR_BLACK); // очищаем TFT-экран
	
	seven_segment_init(); // инициализируем семисегментный индикатор только при входе в его тест

	last_seven_segment_key = KEY_NONE; // сбрасываем последнюю кнопку

	seven_segment_test_value = 0; // начинаем тест с 0000
	
	seven_segment_auto_count = 0; // при входе в тест автосчёт выключен

	seven_segment_auto_tick = 0; // сбрасываем счётчик задержки

	seven_segment_enable(); // разрешаем работу семисегментного индикатора

	seven_segment_set_number(seven_segment_test_value); // выводим 0000 на индикатор
	
	lcd_print_string(4, 4, "ТЕСТ 7 СЕГМ", COLOR_WHITE, COLOR_BLACK, 1); // заголовок экрана

	lcd_print_string(4, 28, "ВЕРХ БОЛЬШЕ", COLOR_WHITE, COLOR_BLACK, 1); // верхняя кнопка увеличивает число

	lcd_print_string(4, 44, "НИЗ МЕНЬШЕ", COLOR_WHITE, COLOR_BLACK, 1); // нижняя кнопка уменьшает число

	lcd_print_string(4, 60, "ЛЕВО ВСЕ 8", COLOR_WHITE, COLOR_BLACK, 1); // левая кнопка показывает 8888
	
	lcd_print_string(4, 76, "ЦЕНТР СЧЕТ", COLOR_WHITE, COLOR_BLACK, 1); // центральная кнопка запускает автосчёт

	lcd_print_string(4, 144, "2Р ЦЕНТР ВЫХОД", COLOR_YELLOW, COLOR_BLACK, 1); // двойное нажатие центральной кнопки — выход

	screen_seven_segment_test_draw_value(); // рисуем текущее число на TFT
}

// обрабатываем управляющую кнопку на экране теста
void screen_seven_segment_test_key(char key)
{
	screen_seven_segment_test_auto_update(); // если автосчёт включен, обновляем число даже когда кнопки не нажимаются

	if (key == KEY_NONE)
	{
		return;
	}
	// если кнопка не нажата — дальше ничего не обрабатываем

	if (key == '5' && last_seven_segment_key == '5')
	{
		// если два раза подряд нажали центральную кнопку, выходим обратно в меню платы номер 1

		seven_segment_auto_count = 0; // останавливаем автосчёт

		seven_segment_disable(); // гасим семисегментный индикатор

		screen_mode = SCREEN_BOARD1_MENU; // возвращаемся в меню платы номер 1

		last_seven_segment_key = KEY_NONE; // сбрасываем последнюю кнопку

		menu_board_1_draw(); // рисуем меню платы номер 1

		return;
	}

	if (key == '5')
	{
		// одиночное нажатие центральной кнопки запускаем автоматический счёт до 9999

		seven_segment_auto_count = 1; // включаем автосчёт

		seven_segment_auto_tick = 0; // сбрасываем счётчик задержки

		last_seven_segment_key = key; // запоминаем кнопку, чтобы второе нажатие 5 считалось выходом

		return;
	}

	if (key == '2')
	{
		// верхняя кнопка — увеличить число вручную

		seven_segment_auto_count = 0; // останавливаем автосчёт при ручном управлении

		seven_segment_test_value++;

		if (seven_segment_test_value > 9999)
		{
			seven_segment_test_value = 0;
		}

		seven_segment_set_number(seven_segment_test_value);
		screen_seven_segment_test_draw_value();
	}

	if (key == '8')
	{
		// нижняя кнопка — уменьшить число вручную

		seven_segment_auto_count = 0; // останавливаем автосчёт при ручном управлении

		if (seven_segment_test_value == 0)
		{
			seven_segment_test_value = 9999;
		}
		else
		{
			seven_segment_test_value--;
		}

		seven_segment_set_number(seven_segment_test_value);
		screen_seven_segment_test_draw_value();
	}

	if (key == '4')
	{
		// левая кнопка — тест всех основных сегментов, 8888 включает все основные сегменты на всех разрядах

		seven_segment_auto_count = 0; // останавливаем автосчёт

		seven_segment_test_value = 8888;

		seven_segment_set_number(seven_segment_test_value);
		screen_seven_segment_test_draw_value();
	}

	if (key == '6')
	{
		// правая кнопка — тест разных цифр

		seven_segment_auto_count = 0; // останавливаем автосчёт

		seven_segment_test_value = 1234;

		seven_segment_set_number(seven_segment_test_value);
		screen_seven_segment_test_draw_value();
	}

	last_seven_segment_key = key; // запоминаем последнюю кнопку для выхода
}
