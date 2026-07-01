/*
 * keybard_display_coderev.c
 *
 * Created: 02.04.2026 22:18:46
 * author : Владислав
 */ 

// подключаем заголовочные файлы

#include "main.h" // основная инициализация
#include "font/font_5x7.h" // таблицы для шрифта
#include "lcd/lcd.h" // подключение заголовочного файла для работы дисплея
#include "nav_buttons/nav_buttons.h" // подключение заголовочного файла для работы управляющих кнопок
#include "keyboard/keyboard.h" // подключение заголовочного файла для работы клавиатуры
#include "menus/menu_board_2/menu_board_2.h" // подключение заголовочного файла для работы меню второй платы
#include "screens/home/home_screen.h" // подключение заголовочного файла для работы главного экрана
#include "screens/board_select/board_select.h" // подключение заголовочного файла для работы экрана выбора плат
#include "screens/keyboard_test/keyboard_test.h" // подключение заголовочного файла для работы окна с тестом клавиатуры
#include "servo/servo.h" // подключение заголовочного файла для работы с сервоприводом
#include "screens/servo_test/servo_test.h" // подключение заголовочного файла для работы окна с тестом сервопривода
#include "stepmotor/stepmotor.h" // подключение заголовочного файла для работы с шаговым мотором
#include "screens/stepper_test/stepper_test.h" // подключение заголовочного файла для работы окна с тестом шагового мотора
#include "menus/menu_board_1/menu_board_1.h" // подключение заголовочного файла для работы меню первой платы
#include "seven_segment/seven_segment.h" // подключение заголовочного файла для работы с семисегментными индикаторами
#include "screens/seven_segment_test/seven_segment_test.h" // подключение заголовочного файла для работы окна с тестом семисегментных индикаторов
#include "led_bar/led_bar.h" // подключение заголовочного файла для работы с 8 светодиодами
#include "screens/led_bar_test/led_bar_test.h" // подключение заголовочного файла для работы окна с тестом 8 светодиодов
#include "speaker/speaker.h" // подключение заголовочного файла для работы со спикером
#include "screens/speaker_test/speaker_test.h" // подключение заголовочного файла для работы окна с тестом спикера
#include "twi/twi.h" // подключение заголовочного файла для работы I2C
#include "rtc/rtc.h" // подключение заголовочного файла для работы DS1307
#include "lcd16x2/lcd1602.h" // подключение заголовочного файла для работы c lcd 16x2
#include "screens/rtc_test/rtc_test.h" // подключение заголовочного файла экрана теста DS1307 (часов реального времени)
#include "screens/temp_lcd_test/temp_lcd_test.h" // подключение заголовочного файла экрана теста MAX31865 и LCD 16x2


// переменные для управления меню

uint8_t menu_selected = 0; // индекс выбранного пункта меню, 0 — первый пункт, 1 — второй пункт и так далее

uint8_t menu_top = 0; // индекс верхнего видимого пункта меню нужен для скролла например, если menu_top = 1, то первым на экране будет пункт с индексом 1

uint8_t screen_mode = SCREEN_HOME; // текущий режим экрана

uint8_t board_selected = 0; // выбранная плата на экране выбора платы, 0 — плата номер 1, 1 — плата номер 2

// настройки двойного нажатия центральной кнопки

#define CENTER_DOUBLE_TIMEOUT_COUNT 30 // сколько раз проверяем второе нажатие центральной кнопки

#define CENTER_DOUBLE_POLL_DELAY_MS 10 // пауза между проверками второго нажатия

// проверка второго нажатия центральной кнопки

uint8_t center_button_double_pressed(void)
{
	uint8_t i; // счётчик попыток ожидания

	char second_key; // здесь будет вторая прочитанная кнопка

	for (i = 0; i < CENTER_DOUBLE_TIMEOUT_COUNT; i++)
	{
		_delay_ms(CENTER_DOUBLE_POLL_DELAY_MS); // маленькая пауза между проверками

		second_key = nav_buttons_get_key(); // снова читаем навигационные кнопки

		if (second_key == '5')
		{
			// если второй раз нажали центральную кнопку, значит это двойное нажатие

			return 1;
		}

		if (second_key != KEY_NONE)
		{
			// если нажали не центральную кнопку, значит это уже не двойное нажатие 5

			return 0;
		}
	}

	return 0; // второго нажатия 5 не было
}

// функция настройки аппаратного SPI

void spi_init(void)
{
	
	// регистр управления SPI SPCR
	// значение SPE = 1 — включить SPI
	// значение MSTR = 1 — режим Master
	// биты SPR0 и SPR1 сброшены для минимального базового делителя
	SPCR = (1 << SPE) | (1 << MSTR);

	// регистр состояния SPI SPSR
	// значение SPI2X = 1 — включить двойную скорость
	// при F_CPU = 8 МГц скорость SPI будет F_CPU / 2 = 4 МГц
	SPSR = (1 << SPI2X);

}

// отключаем системный делитель частоты

void clock_init(void)
{
	CLKPR = (1 << CLKPCE);// разрешаем изменение делителя частоты
	CLKPR = 0;
}

int main(void)
{
	clock_init(); // отключаем делитель частоты

	lcd_port_init(); // настраиваем порты дисплея

	spi_init(); // настраиваем SPI

	nav_buttons_init(); // инициализация пяти управляющих кнопок

	keypad_init(); // настраиваем клавиатуру

	lcd_init(); // инициализируем дисплей
	
	lcd1602_init(); // один раз инициализируем LCD 16x2

	lcd1602_off(); // выключаем отображение LCD 16x2
	
	sei(); // разрешаем глобальные прерывания

	screen_mode = SCREEN_HOME; // начинаем с главного экрана

	screen_home_start(); // рисуем главный экран

	while (1)
	{
		// читаем пять навигационных кнопок
		// они возвращают виртуальные клавиши:
		// '2' — вверх
		// '8' — вниз
		// '4' — влево
		// '6' — вправо
		// '5' — центр
		char key = nav_buttons_get_key();

		if (screen_mode == SCREEN_HOME)
		{
			// если открыт главный экран

			screen_home_key(key); // передаём кнопку главному экрану
		}

		else if (screen_mode == SCREEN_BOARD_SELECT)
		{
			// если открыт экран выбора платы

			if (key == '5')
			{
				// если нажата центральная кнопка,
				// сначала проверяем:
				// это одиночное нажатие или двойное

				if (center_button_double_pressed())
				{
					// если было двойное нажатие 55, возвращаемся на главный экран

					screen_mode = SCREEN_HOME; // меняем режим экрана

					screen_home_start(); // рисуем главный экран
				}
				else
				{
					// если второго нажатия не было, значит это обычный выбор платы

					screen_board_select_key(key); // передаём одиночную 5 экрану выбора плат
				}
			}
			else
			{
				// если нажата не центральная кнопка, просто передаём её экрану выбора плат

				screen_board_select_key(key);
			}
		}
		
		else if (screen_mode == SCREEN_BOARD1_MENU)
		{
			// если открыто меню платы номер 1

			if (key == '2')
			{
				// верхняя кнопка, двигаем выделение меню вверх

				uint8_t old_selected = menu_board_1_get_selected(); // запоминаем старый выбранный пункт

				menu_board_1_up(); // перемещаем выделение вверх

				menu_board_1_draw_item(old_selected); // перерисовываем старый пункт

				menu_board_1_draw_item(menu_board_1_get_selected()); // перерисовываем новый выбранный пункт
			}

			if (key == '8')
			{
				// нижняя кнопка, двигаем выделение меню вниз

				uint8_t old_selected = menu_board_1_get_selected(); // запоминаем старый выбранный пункт

				menu_board_1_down(); // перемещаем выделение вниз

				menu_board_1_draw_item(old_selected); // перерисовываем старый пункт

				menu_board_1_draw_item(menu_board_1_get_selected()); // перерисовываем новый выбранный пункт
			}
			
			if (key == '5')
			{
				// центральная кнопка
				// в меню платы №1:
				// одиночное 5 — открыть пункт меню
				// двойное 55 — вернуться на экран выбора плат

				if (center_button_double_pressed())
				{
					// если было двойное нажатие 55, возвращаемся на экран выбора плат

					screen_mode = SCREEN_BOARD_SELECT;	// меняем режим экрана

					screen_board_select_start(); // рисуем экран выбора плат
				}
				else
				{
					// если второго нажатия не было, значит это обычный выбор пункта меню

					if (menu_board_1_get_selected() == BOARD1_MENU_ITEM_SEVEN_SEGMENT_TEST)
					{
						// если выбран первый пункт:
						// "ТЕСТ 7 СЕГМ"

						screen_mode = SCREEN_SEVEN_SEGMENT_TEST; // переходим в экран теста семисегментных индикаторов

						screen_seven_segment_test_start(); // открываем экран теста
					}

					else if (menu_board_1_get_selected() == BOARD1_MENU_ITEM_LED_BAR_TEST)
					{
						// если выбран второй пункт:
						// "ТЕСТ ДИОДОВ"

						screen_mode = SCREEN_LED_BAR_TEST; // переходим в экран теста 8 светодиодов

						screen_led_bar_test_start(); // открываем экран теста
					}

					else if (menu_board_1_get_selected() == BOARD1_MENU_ITEM_SPEAKER_TEST)
					{
						// если выбран третий пункт:
						// "ТЕСТ СПИКЕРА"

						screen_mode = SCREEN_SPEAKER_TEST; // переходим в экран теста спикера

						screen_speaker_test_start(); // открываем экран теста
					}
				}
			}
		}

		else if (screen_mode == SCREEN_BOARD2_MENU)
		{
			// если открыто меню платы номер 2

			if (key == '2')
			{
				// верхняя кнопка, двигаем выделение меню вверх

				uint8_t old_selected = menu_selected; // запоминаем старый выбранный пункт

				uint8_t old_top = menu_top; // запоминаем старую верхнюю позицию меню

				menu_up(); // меняем выбранный пункт вверх

				if (old_top != menu_top)
				{
					// если меню прокрутилось, перерисовываем всё меню

					menu_draw();
				}
				else
				{
					// если прокрутки не было, перерисовываем только два пункта

					menu_draw_item(old_selected); // старый выбранный пункт

					menu_draw_item(menu_selected); // новый выбранный пункт
				}
			}

			if (key == '8')
			{
				// нижняя кнопка, двигаем выделение меню вниз

				uint8_t old_selected = menu_selected; // запоминаем старый выбранный пункт

				uint8_t old_top = menu_top; // запоминаем старую верхнюю позицию меню

				menu_down(); // меняем выбранный пункт вниз

				if (old_top != menu_top)
				{
					// если меню прокрутилось, перерисовываем всё меню

					menu_draw();
				}
				else
				{
					// если прокрутки не было, перерисовываем только два пункта

					menu_draw_item(old_selected); // старый выбранный пункт

					menu_draw_item(menu_selected); // новый выбранный пункт
				}
			}

			if (key == '5')
			{
				// центральная кнопка
				// в меню платы №2:
				// одиночное 5 — открыть пункт меню
				// двойное 55 — вернуться на экран выбора плат

				if (center_button_double_pressed())
				{
					// если было двойное нажатие 55, возвращаемся на экран выбора плат

					screen_mode = SCREEN_BOARD_SELECT; // меняем режим экрана

					screen_board_select_start(); // рисуем экран выбора плат
				}
				else
				{
					// если второго нажатия не было, значит это обычный выбор пункта меню

					if (menu_selected == MENU_ITEM_KEYBOARD_TEST)
					{
						// выбран тест клавиатуры

						screen_mode = SCREEN_KEYBOARD_TEST; // переходим в экран теста клавиатуры

						screen_keypad_test_start(); // рисуем экран теста клавиатуры
					}

					else if (menu_selected == MENU_ITEM_SERVO_TEST)
					{
						// выбран тест сервопривода

						screen_mode = SCREEN_SERVO_TEST; // переходим в экран теста сервопривода

						screen_servo_test_start(); // рисуем экран теста сервопривода
					}

					else if (menu_selected == MENU_ITEM_STEPPER_TEST)
					{
						// выбран тест шагового мотора

						screen_mode = SCREEN_STEPPER_TEST; // переходим в экран теста шагового мотора

						screen_stepper_test_start(); // рисуем экран теста шагового мотора
					}
					
					
					else if (menu_selected == MENU_ITEM_TEMP_LCD_TEST)
					{
						// выбран четвёртый пункт:
						// модуль MAX31865 + LCD 16x2

						screen_mode = SCREEN_TEMP_LCD_TEST; // переходим в экран теста lcd и max31865

						screen_temp_lcd_test_start(); // открываем экран теста lcd и max31865
					}

					else if (menu_selected == MENU_ITEM_RTC_TEST)
					{
						// выбран пятый пункт:
						// тест часов реального времени DS1307

						screen_mode = SCREEN_RTC_TEST; // переходим в экран теста DS1307

						screen_rtc_test_start(); // открываем экран теста DS1307
					}

				}
			}
		}
		
		else if (screen_mode == SCREEN_SEVEN_SEGMENT_TEST)
		{
			// если открыт тест семисегментных индикаторов

			screen_seven_segment_test_key(key); // передаём управляющую кнопку экрану теста
		}
		
		else if (screen_mode == SCREEN_LED_BAR_TEST)
		{
			// если открыт тест 8 светодиодов

			screen_led_bar_test_key(key); // передаём управляющую кнопку экрану теста
		}
		
		else if (screen_mode == SCREEN_SPEAKER_TEST)
		{
			// если открыт тест спикера

			screen_speaker_test_key(key); // передаём управляющую кнопку экрану теста
		}

		else if (screen_mode == SCREEN_KEYBOARD_TEST)
		{
			// если открыт тест клавиатуры

			screen_keypad_test_key(key); // передаём управляющую кнопку экрану теста клавиатуры, сама клавиатура 4x3 читается внутри keyboard_test.c
		}

		else if (screen_mode == SCREEN_SERVO_TEST)
		{
			// если открыт тест сервопривода

			screen_servo_test_key(key); // передаём управляющую кнопку экрану теста сервопривода
		}

		else if (screen_mode == SCREEN_STEPPER_TEST)
		{
			// если открыт тест шагового мотора

			screen_stepper_test_key(key); // передаём управляющую кнопку экрану теста шагового мотора
		}
		
		else if (screen_mode == SCREEN_RTC_TEST)
		{
			// если открыт тест DS1307

			screen_rtc_test_key(key); // передаём управляющую кнопку экрану теста часов
		}
		
		else if (screen_mode == SCREEN_TEMP_LCD_TEST)
		{
			screen_temp_lcd_test_key(key); // передаём управляющую кнопку экрану теста MAX31865 и lcd16x2
		}
	}
}
