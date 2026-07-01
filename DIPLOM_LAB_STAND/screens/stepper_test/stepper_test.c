/*
 * stepper_test.c
 *
 * Created: 22.04.2026 0:28:37
 *  Author: Владислав
 */ 


#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_2/menu_board_2.h"
#include "../../stepmotor/stepmotor.h"
#include "stepper_test.h"

#define STEPPER_MOVE_STEPS 256 // сколько полушагов делать за одно нажатие 4 или 6

#define STEPPER_DELAY_MIN 600 // минимальная задержка между полушагами

#define STEPPER_DELAY_MAX 3000 // максимальная задержка между полушагами

#define STEPPER_DELAY_STEP 100 // на сколько менять задержку кнопками 2 и 8

static char last_stepper_key = KEY_NONE; // последняя нажатая клавиша нужна для выхода по двойному нажатию 5

static uint16_t stepper_delay_us = 800; // текущая задержка между полушагами в микросекундах, меньше значение — быстрее мотор, больше значение — медленнее мотор

static int16_t stepper_position = 0; // условная позиция мотора в полушагах

// отрисовываем беззнаковое число

static void screen_stepper_test_draw_u16(uint8_t x, uint8_t y, uint16_t value, uint16_t color, uint16_t bg, uint8_t scale)
{
	uint16_t div;
	uint8_t digit;
	uint8_t started;

	div = 10000;
	started = 0;

	while (div > 0)
	{
		digit = value / div;
		value = value % div;

		if (digit > 0 || started || div == 1)
		{
			lcd_draw_char(x, y, '0' + digit, color, bg, scale);

			x += 6 * scale;
			started = 1;
		}

		div = div / 10;
	}
}

// отрисовываем состояние шагового мотора

static void screen_stepper_test_draw_status(void)
{
	lcd_fill_rect(0, 94, 128, 62, COLOR_BLACK); // очищаем нижнюю область статуса

	lcd_print_string(4, 98, "ЗАДЕРЖКА", COLOR_WHITE, COLOR_BLACK, 1); // показываем текущую задержку между шагами

	screen_stepper_test_draw_u16(4, 112, stepper_delay_us, COLOR_GREEN, COLOR_BLACK, 1); // печатаем значение задержки

	lcd_print_string(44, 112, "МКС", COLOR_WHITE, COLOR_BLACK, 1); // подпись "микросекунды"

}

// открываем экран теста шагового мотора

void screen_stepper_test_start(void)
{
	SM_init(); // инициализируем выводы шагового мотора
	
	lcd_fill_color(COLOR_BLACK); // очищаем экран

	last_stepper_key = KEY_NONE; // сбрасываем последнюю клавишу

	lcd_print_string(4, 4, "ТЕСТ МОТОРА", COLOR_WHITE, COLOR_BLACK, 1); // заголовок экрана

	lcd_print_string(4, 24, "ЛЕВО", COLOR_WHITE, COLOR_BLACK, 1); // кнопка 4 — вращение влево

	lcd_print_string(4, 40, "ПРАВО", COLOR_WHITE, COLOR_BLACK, 1); // кнопка 6 — вращение вправо

	lcd_print_string(4, 56, "ВЕРХ БЫСТРЕЕ", COLOR_WHITE, COLOR_BLACK, 1); // кнопка 2 — уменьшить задержку, то есть увеличить скорость

	lcd_print_string(4, 72, "НИЗ МЕДЛЕННЕЕ", COLOR_WHITE, COLOR_BLACK, 1); // кнопка 8 — увеличить задержку, то есть уменьшить скорость

	screen_stepper_test_draw_status(); // рисуем скорость и позицию

	lcd_print_string(4, 146, "2Р ЦЕНТР ВЫХОД", COLOR_YELLOW, COLOR_BLACK, 1); // двойное нажатие 5 — выход обратно в меню
}


// обрабатываем кнопку на экране теста шагового мотора

void screen_stepper_test_key(char key)
{
	if (key == KEY_NONE)
	{
		return;
	}
	// если кнопка не нажата — ничего не делаем

	if (key == '5' && last_stepper_key == '5')
	{
		screen_mode = SCREEN_BOARD2_MENU; // возвращаемся в меню

		last_stepper_key = KEY_NONE; // сбрасываем последнюю клавишу

		SM_off(); // отключаем обмотки мотора
		// так мотор перестанет держать положение и меньше будет греться

		menu_draw(); // рисуем меню

		return;
	}

	if (key == '4')
	{
		SM_back_steps(STEPPER_MOVE_STEPS, stepper_delay_us); // кнопка 4 — вращение влево

		stepper_position -= STEPPER_MOVE_STEPS; // меняем условную позицию

		screen_stepper_test_draw_status(); // обновляем статус
	}

	if (key == '6')
	{
		SM_forward_steps(STEPPER_MOVE_STEPS, stepper_delay_us); // кнопка 6 — вращение вправо

		stepper_position += STEPPER_MOVE_STEPS; // меняем условную позицию

		screen_stepper_test_draw_status(); // обновляем статус
	}

	if (key == '2')
	{
		if (stepper_delay_us > STEPPER_DELAY_MIN + STEPPER_DELAY_STEP)
		{
			stepper_delay_us -= STEPPER_DELAY_STEP;
		}
		else
		{
			stepper_delay_us = STEPPER_DELAY_MIN;
		}

		screen_stepper_test_draw_status();
	}

	if (key == '8')
	{
		if (stepper_delay_us < STEPPER_DELAY_MAX - STEPPER_DELAY_STEP)
		{
			stepper_delay_us += STEPPER_DELAY_STEP;
		}
		else
		{
			stepper_delay_us = STEPPER_DELAY_MAX;
		}

		screen_stepper_test_draw_status(); 
	}

	last_stepper_key = key; // запоминаем последнюю клавишу для выхода
}
