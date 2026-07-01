/*
 * rtc_test.c
 *
 * Created: 18.05.2026 23:34:55
 *  Author: Владислав
 */ 


#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_2/menu_board_2.h"
#include "../../rtc/rtc.h"
#include "rtc_test.h"

// режимы экрана DS1307

#define RTC_SCREEN_VIEW      0 // режим просмотра текущей даты и времени

#define RTC_SCREEN_EDIT_DATE 1 // режим настройки даты

#define RTC_SCREEN_EDIT_TIME 2 // режим настройки времени

// поля даты

#define RTC_DATE_FIELD_DATE  0 // число месяца

#define RTC_DATE_FIELD_MONTH 1 // месяц

#define RTC_DATE_FIELD_YEAR  2 // год

#define RTC_DATE_FIELD_COUNT 3 // количество полей даты

// поля времени

#define RTC_TIME_FIELD_HOUR 0 // часы

#define RTC_TIME_FIELD_MIN  1 // минуты

#define RTC_TIME_FIELD_SEC  2 // секунды

#define RTC_TIME_FIELD_COUNT 3 // количество полей времени

// глобальные переменные экрана

static char last_rtc_key = KEY_NONE; // последняя нажатая кнопка, нужна для двойного нажатия 5

static rtc_time_t rtc_current; // текущая дата и время, считанные из DS1307

static rtc_time_t rtc_edit; // редактируемая дата и время

static uint8_t rtc_screen_mode = RTC_SCREEN_VIEW; // текущий режим экрана DS1307

static uint8_t rtc_selected_field = 0; // выбранное поле при настройке даты или времени

static uint8_t rtc_saved_message = 0; // 0 — сообщение не показывается, 1 — показываем "ЗАПИСАНО"

static uint8_t rtc_view_update_tick = 0; // счётчик обновления времени на экране просмотра

static uint8_t rtc_last_drawn_sec = 255; // последняя секунда, которая была нарисована на экране, 255 — невозможное значение, чтобы при первом входе экран точно обновился

// определяем количество дней в месяце

static uint8_t rtc_days_in_month(uint8_t month, uint8_t year)
{
	// возвращаем количество дней в месяце
	// значение YEAR хранится как 0..99
	// условно считаем, что это 2000..2099
	
	if (month == 2)
	{
		if ((year % 4) == 0)
		{
			return 29;
		}
		else
		{
			return 28;
		}
	}

	if (month == 4 || month == 6 || month == 9 || month == 11)
	{
		return 30;
	}

	return 31;
}

// ограничиваем число месяца допустимым диапазоном

static void rtc_limit_date(void)
{
	uint8_t max_date;

	if (rtc_edit.month < 1)
	{
		rtc_edit.month = 1;
	}

	if (rtc_edit.month > 12)
	{
		rtc_edit.month = 12;
	}

	max_date = rtc_days_in_month(rtc_edit.month, rtc_edit.year);

	if (rtc_edit.date < 1)
	{
		rtc_edit.date = 1;
	}

	if (rtc_edit.date > max_date)
	{
		rtc_edit.date = max_date;
	}
}

// отрисовываем две цифры обычным цветом

static void rtc_draw_2digits(uint8_t x, uint8_t y, uint8_t value, uint16_t color, uint16_t bg, uint8_t scale)
{
	lcd_draw_char(x, y, '0' + (value / 10), color, bg, scale);
	lcd_draw_char(x + 6 * scale, y, '0' + (value % 10), color, bg, scale);
}

// отрисовываем две цифры с выделением выбранного поля даты

static void rtc_draw_date_field_2digits(uint8_t x, uint8_t y, uint8_t value, uint8_t field)
{
	uint16_t fg;
	uint16_t bg;

	if (rtc_selected_field == field)
	{
		fg = COLOR_BLACK;
		bg = COLOR_YELLOW;
	}
	else
	{
		fg = COLOR_GREEN;
		bg = COLOR_BLACK;
	}

	rtc_draw_2digits(x, y, value, fg, bg, 2);
}

// отрисовываем две цифры с выделением выбранного поля времени

static void rtc_draw_time_field_2digits(uint8_t x, uint8_t y, uint8_t value, uint8_t field)
{
	uint16_t fg;
	uint16_t bg;

	if (rtc_selected_field == field)
	{
		fg = COLOR_BLACK;
		bg = COLOR_YELLOW;
	}
	else
	{
		fg = COLOR_GREEN;
		bg = COLOR_BLACK;
	}

	rtc_draw_2digits(x, y, value, fg, bg, 2);
}


// очищаем рабочую область экрана

static void screen_rtc_test_clear_work_area(void)
{
	lcd_fill_rect(0, 20, 128, 136, COLOR_BLACK); // очищаем только рабочую область экрана
}

// отрисовываем статичную часть экрана просмотра

static void screen_rtc_test_draw_view_static(void)
{
	lcd_print_string(4, 24, "ТЕКУЩЕЕ ВРЕМЯ", COLOR_WHITE, COLOR_BLACK, 1);

	lcd_draw_char(28, 44, '.', COLOR_WHITE, COLOR_BLACK, 2);
	lcd_draw_char(64, 44, '.', COLOR_WHITE, COLOR_BLACK, 2);

	lcd_draw_char(76, 44, '2', COLOR_GREEN, COLOR_BLACK, 2);
	lcd_draw_char(88, 44, '0', COLOR_GREEN, COLOR_BLACK, 2);

	lcd_draw_char(40, 76, ':', COLOR_WHITE, COLOR_BLACK, 2);
	lcd_draw_char(76, 76, ':', COLOR_WHITE, COLOR_BLACK, 2);

	lcd_print_string(4, 112, "ЛЕВ НАСТР ДАТЫ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(4, 128, "ПРАВ НАСТР ВРЕМ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(4, 144, "2Р ЦЕНТР ВЫХОД", COLOR_YELLOW, COLOR_BLACK, 1);
}

// отрисовываем текущие значения даты и времени

static void screen_rtc_test_draw_view_values(void)
{

	rtc_draw_2digits(4, 44, rtc_current.date, COLOR_GREEN, COLOR_BLACK, 2);
	rtc_draw_2digits(40, 44, rtc_current.month, COLOR_GREEN, COLOR_BLACK, 2);
	rtc_draw_2digits(100, 44, rtc_current.year, COLOR_GREEN, COLOR_BLACK, 2);

	rtc_draw_2digits(16, 76, rtc_current.hour, COLOR_GREEN, COLOR_BLACK, 2);
	rtc_draw_2digits(52, 76, rtc_current.min, COLOR_GREEN, COLOR_BLACK, 2);
	rtc_draw_2digits(88, 76, rtc_current.sec, COLOR_GREEN, COLOR_BLACK, 2);

	if (rtc_saved_message)
	{
		lcd_print_string(70, 144, "ЗАПИСЬ", COLOR_GREEN, COLOR_BLACK, 1);
	}
	else
	{
		lcd_print_string(70, 144, "     ", COLOR_WHITE, COLOR_BLACK, 1);
	}
}

// отрисовываем экран просмотра DS1307

static void screen_rtc_test_draw_view(void)
{
	screen_rtc_test_draw_view_static(); // рисуем статичные надписи

	screen_rtc_test_draw_view_values(); // рисуем только значения даты и времени
}

// отрисовываем экран настройки даты

static void screen_rtc_test_draw_edit_date(void)
{
	screen_rtc_test_clear_work_area();

	lcd_print_string(4, 24, "НАСТРОЙКА ДАТЫ", COLOR_WHITE, COLOR_BLACK, 1);

	rtc_draw_date_field_2digits(4, 44, rtc_edit.date, RTC_DATE_FIELD_DATE);
	lcd_draw_char(28, 44, '.', COLOR_WHITE, COLOR_BLACK, 2);

	rtc_draw_date_field_2digits(40, 44, rtc_edit.month, RTC_DATE_FIELD_MONTH);
	lcd_draw_char(64, 44, '.', COLOR_WHITE, COLOR_BLACK, 2);

	lcd_draw_char(76, 44, '2', COLOR_GREEN, COLOR_BLACK, 2);
	lcd_draw_char(88, 44, '0', COLOR_GREEN, COLOR_BLACK, 2);
	rtc_draw_date_field_2digits(100, 44, rtc_edit.year, RTC_DATE_FIELD_YEAR);

	lcd_print_string(4, 88, "ВЕРХ/НИЗ ПОЛЕ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(4, 104, "ЛЕВ/ПРАВ ЗНАЧ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(4, 120, "ЦЕНТР ЗАПИСЬ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(70, 120, "2Р ЦЕНТР ВЫХ", COLOR_YELLOW, COLOR_BLACK, 1);
}

// отрисовываем экран настройки времени

static void screen_rtc_test_draw_edit_time(void)
{
	screen_rtc_test_clear_work_area(); // очищаем рабочую область перед отрисовкой настройки времени

	lcd_print_string(4, 24, "НАСТРОЙКА ВРЕМ", COLOR_WHITE, COLOR_BLACK, 1);

	rtc_draw_time_field_2digits(16, 52, rtc_edit.hour, RTC_TIME_FIELD_HOUR);
	lcd_draw_char(40, 52, ':', COLOR_WHITE, COLOR_BLACK, 2);

	rtc_draw_time_field_2digits(52, 52, rtc_edit.min, RTC_TIME_FIELD_MIN);
	lcd_draw_char(76, 52, ':', COLOR_WHITE, COLOR_BLACK, 2);

	rtc_draw_time_field_2digits(88, 52, rtc_edit.sec, RTC_TIME_FIELD_SEC);

	lcd_print_string(4, 104, "ВЕРХ/НИЗ ПОЛЕ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(4, 120, "ЛЕВО/ПРАВО ЗНАЧ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(4, 136, "ЦЕНТР ЗАПИСЬ", COLOR_WHITE, COLOR_BLACK, 1);
	lcd_print_string(70, 136, "2РАЗА ЦЕНТР ВЫХ", COLOR_YELLOW, COLOR_BLACK, 1);
}

// обновляем экран по текущему режиму

static void screen_rtc_test_draw(void)
{
	if (rtc_screen_mode == RTC_SCREEN_VIEW)
	{
		screen_rtc_test_clear_work_area(); 
		screen_rtc_test_draw_view_static();
		screen_rtc_test_draw_view_values();

		rtc_last_drawn_sec = rtc_current.sec;
	}
	else if (rtc_screen_mode == RTC_SCREEN_EDIT_DATE)
	{
		screen_rtc_test_draw_edit_date();
	}
	else if (rtc_screen_mode == RTC_SCREEN_EDIT_TIME)
	{
		screen_rtc_test_draw_edit_time();
	}
}

// выбираем следующее поле

static void rtc_next_field(void)
{
	if (rtc_screen_mode == RTC_SCREEN_EDIT_DATE)
	{
		if (rtc_selected_field < RTC_DATE_FIELD_COUNT - 1)
		{
			rtc_selected_field++;
		}
		else
		{
			rtc_selected_field = 0;
		}
	}

	else if (rtc_screen_mode == RTC_SCREEN_EDIT_TIME)
	{
		if (rtc_selected_field < RTC_TIME_FIELD_COUNT - 1)
		{
			rtc_selected_field++;
		}
		else
		{
			rtc_selected_field = 0;
		}
	}
}

// выбираем предыдущее поле

static void rtc_prev_field(void)
{
	if (rtc_screen_mode == RTC_SCREEN_EDIT_DATE)
	{
		if (rtc_selected_field > 0)
		{
			rtc_selected_field--;
		}
		else
		{
			rtc_selected_field = RTC_DATE_FIELD_COUNT - 1;
		}
	}

	else if (rtc_screen_mode == RTC_SCREEN_EDIT_TIME)
	{
		if (rtc_selected_field > 0)
		{
			rtc_selected_field--;
		}
		else
		{
			rtc_selected_field = RTC_TIME_FIELD_COUNT - 1;
		}
	}
}

// увеличиваем значение выбранного поля

static void rtc_increase_field(void)
{
	uint8_t max_date;

	if (rtc_screen_mode == RTC_SCREEN_EDIT_DATE)
	{
		if (rtc_selected_field == RTC_DATE_FIELD_DATE)
		{
			max_date = rtc_days_in_month(rtc_edit.month, rtc_edit.year);

			if (rtc_edit.date < max_date)
			{
				rtc_edit.date++;
			}
			else
			{
				rtc_edit.date = 1;
			}
		}

		else if (rtc_selected_field == RTC_DATE_FIELD_MONTH)
		{
			if (rtc_edit.month < 12)
			{
				rtc_edit.month++;
			}
			else
			{
				rtc_edit.month = 1;
			}

			rtc_limit_date();
		}

		else if (rtc_selected_field == RTC_DATE_FIELD_YEAR)
		{
			if (rtc_edit.year < 99)
			{
				rtc_edit.year++;
			}
			else
			{
				rtc_edit.year = 0;
			}

			rtc_limit_date();
		}

	}

	else if (rtc_screen_mode == RTC_SCREEN_EDIT_TIME)
	{
		if (rtc_selected_field == RTC_TIME_FIELD_HOUR)
		{
			if (rtc_edit.hour < 23)
			{
				rtc_edit.hour++;
			}
			else
			{
				rtc_edit.hour = 0;
			}
		}

		else if (rtc_selected_field == RTC_TIME_FIELD_MIN)
		{
			if (rtc_edit.min < 59)
			{
				rtc_edit.min++;
			}
			else
			{
				rtc_edit.min = 0;
			}
		}

		else if (rtc_selected_field == RTC_TIME_FIELD_SEC)
		{
			if (rtc_edit.sec < 59)
			{
				rtc_edit.sec++;
			}
			else
			{
				rtc_edit.sec = 0;
			}
		}
	}
}

// уменьшаем значение выбранного поля

static void rtc_decrease_field(void)
{
	uint8_t max_date;

	if (rtc_screen_mode == RTC_SCREEN_EDIT_DATE)
	{
		if (rtc_selected_field == RTC_DATE_FIELD_DATE)
		{
			max_date = rtc_days_in_month(rtc_edit.month, rtc_edit.year);

			if (rtc_edit.date > 1)
			{
				rtc_edit.date--;
			}
			else
			{
				rtc_edit.date = max_date;
			}
		}

		else if (rtc_selected_field == RTC_DATE_FIELD_MONTH)
		{
			if (rtc_edit.month > 1)
			{
				rtc_edit.month--;
			}
			else
			{
				rtc_edit.month = 12;
			}

			rtc_limit_date();
		}

		else if (rtc_selected_field == RTC_DATE_FIELD_YEAR)
		{
			if (rtc_edit.year > 0)
			{
				rtc_edit.year--;
			}
			else
			{
				rtc_edit.year = 99;
			}

			rtc_limit_date();
		}
	}

	else if (rtc_screen_mode == RTC_SCREEN_EDIT_TIME)
	{
		if (rtc_selected_field == RTC_TIME_FIELD_HOUR)
		{
			if (rtc_edit.hour > 0)
			{
				rtc_edit.hour--;
			}
			else
			{
				rtc_edit.hour = 23;
			}
		}

		else if (rtc_selected_field == RTC_TIME_FIELD_MIN)
		{
			if (rtc_edit.min > 0)
			{
				rtc_edit.min--;
			}
			else
			{
				rtc_edit.min = 59;
			}
		}

		else if (rtc_selected_field == RTC_TIME_FIELD_SEC)
		{
			if (rtc_edit.sec > 0)
			{
				rtc_edit.sec--;
			}
			else
			{
				rtc_edit.sec = 59;
			}
		}
	}
}

// сохраняем настройки

static void rtc_save_edit(void)
{
	rtc_time_t now;

	rtc_read(&now); // перед записью читаем актуальные данные DS1307

	if (rtc_screen_mode == RTC_SCREEN_EDIT_DATE)
	{
		rtc_edit.hour = now.hour;
		rtc_edit.min = now.min;
		rtc_edit.sec = now.sec;
	}
	else if (rtc_screen_mode == RTC_SCREEN_EDIT_TIME)
	{
		rtc_edit.date = now.date;
		rtc_edit.month = now.month;
		rtc_edit.year = now.year;
		rtc_edit.day = now.day;
	}

	rtc_write(&rtc_edit); // записываем итоговые данные в DS1307

	rtc_read(&rtc_current); // читаем обратно

	rtc_screen_mode = RTC_SCREEN_VIEW; // возвращаемся в режим просмотра

	rtc_saved_message = 1; // показываем сообщение "ЗАПИСАНО"

	screen_rtc_test_clear_work_area(); // очищаем рабочую область

	screen_rtc_test_draw_view_static(); // рисуем статичные надписи режима просмотра

	screen_rtc_test_draw_view_values(); // рисуем текущие дату и время

	rtc_last_drawn_sec = rtc_current.sec; // запоминаем последнюю нарисованную секунду
}

// открываем экран DS1307

void screen_rtc_test_start(void)
{
	lcd_fill_color(COLOR_BLACK); // очищаем экран

	last_rtc_key = KEY_NONE; // сбрасываем последнюю кнопку

	rtc_screen_mode = RTC_SCREEN_VIEW; // начинаем с режима просмотра текущего времени

	rtc_saved_message = 0; // сообщение "ЗАПИСАНО" пока не показываем

	rtc_view_update_tick = 0; // сбрасываем счётчик обновления

	rtc_init(); // инициализируем I2C

	rtc_read(&rtc_current); // читаем текущую дату и время

	lcd_print_string(4, 4, "ТЕСТ ДС1307", COLOR_WHITE, COLOR_BLACK, 1); // заголовок

	screen_rtc_test_draw_view_static(); // рисуем статичные надписи один раз

	screen_rtc_test_draw_view_values(); // рисуем текущие цифры даты и времени

	rtc_last_drawn_sec = rtc_current.sec; // запоминаем нарисованную секунду
}

// обрабатываем кнопки на экране DS1307

void screen_rtc_test_key(char key)
{
	if (key == KEY_NONE)
	{
		if (rtc_screen_mode == RTC_SCREEN_VIEW)
		{
			_delay_ms(100); // небольшая задержка, чтобы не читать DS1307 слишком часто

			rtc_view_update_tick++;

			if (rtc_view_update_tick >= 3)
			{
				// примерно раз в 300 мс проверяем DS1307

				rtc_view_update_tick = 0;

				rtc_read(&rtc_current); // читаем актуальное время

				if (rtc_current.sec != rtc_last_drawn_sec)
				{
					// если секунда изменилась, обновляем только цифры

					screen_rtc_test_draw_view_values();

					rtc_last_drawn_sec = rtc_current.sec;
				}
			}
		}

		return;
	}

	if (key == '5' && last_rtc_key == '5')
	{
		// двойное нажатие 5 — выход в меню платы №2

		screen_mode = SCREEN_BOARD2_MENU; // возвращаемся в меню платы №2

		last_rtc_key = KEY_NONE; // сбрасываем последнюю кнопку

		menu_draw(); // рисуем меню платы №2

		return;
	}

	if (rtc_screen_mode == RTC_SCREEN_VIEW)
	{
		if (key == '4')
		{
			// левая кнопка на экране просмотра:
			// перейти к настройке даты

			rtc_read(&rtc_edit); // берём текущую дату и время как основу

			rtc_screen_mode = RTC_SCREEN_EDIT_DATE; // переходим в режим настройки даты

			rtc_selected_field = RTC_DATE_FIELD_DATE; // начинаем с поля "число месяца"

			rtc_saved_message = 0; // убираем сообщение "ЗАПИСАНО"

			screen_rtc_test_draw_edit_date(); // рисуем экран настройки даты
		}

		else if (key == '6')
		{
			// правая кнопка на экране просмотра:
			// перейти к настройке времени

			rtc_read(&rtc_edit); // берём текущее время как основу

			rtc_screen_mode = RTC_SCREEN_EDIT_TIME; // переходим в режим настройки времени

			rtc_selected_field = RTC_TIME_FIELD_HOUR; // начинаем с поля "часы"

			rtc_saved_message = 0; // убираем сообщение "ЗАПИСАНО"

			screen_rtc_test_draw_edit_time(); // рисуем экран настройки времени
		}

		last_rtc_key = key;
		return;
	}

	if (rtc_screen_mode == RTC_SCREEN_EDIT_DATE || rtc_screen_mode == RTC_SCREEN_EDIT_TIME)
	{
		if (key == '4')
		{
			rtc_prev_field(); // левая кнопка — предыдущее поле

			screen_rtc_test_draw();
		}

		else if (key == '6')
		{
			rtc_next_field(); // правая кнопка — следующее поле

			screen_rtc_test_draw();
		}

		else if (key == '8')
		{
			rtc_decrease_field(); // нижняя кнопка — уменьшить значение

			screen_rtc_test_draw();
		}

		else if (key == '2')
		{
			rtc_increase_field(); // верхняя кнопка — увеличить значение

			screen_rtc_test_draw();
		}

		else if (key == '5')
		{
			rtc_save_edit(); // центральная кнопка — записать изменения и вернуться к просмотру
		}
	}

	last_rtc_key = key; // запоминаем последнюю кнопку
}
