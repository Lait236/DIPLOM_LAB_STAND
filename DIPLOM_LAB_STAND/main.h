	/*
 * main.h
 *
 * Created: 02.04.2026 22:29:26
 *  Author: Владислав
 */ 


#ifndef MAIN_H_
#define MAIN_H_

// установка частоты

#define F_CPU 8000000UL

// подключение системных файлов библиотек

#include <avr/io.h> // стандартная библиотека инициализации AVR
#include <util/delay.h> // задержки
#include <stdint.h> // фиксированные целочисленные типы
#include <avr/pgmspace.h> // работа с памятью программ Flash || PROGMEM хранит большие таблицы не в SRAM, а во Flash, так как SRAM мало || обращение к Flash памяти идет через оператор &
#include <avr/interrupt.h> // работа с прерываниями

// замена имен лапок на макроподстановки для удобной инициализации spi для дисплея

#define LCD_SCK  PB7 // пин тактового сигнала SPI
#define LCD_MOSI PB5 // пин MOSI SPI
#define LCD_CS   PB4 // пин Chip Select дисплея подключён к PB2 || 0 — дисплей выбран и слушает SPI || 1 — дисплей игнорирует SPI
#define LCD_DC   PD1 // пин DC дисплея подключён к PD1 || DC = Data / Command || 0 — отправляем команду || 1 — отправляем данные
#define LCD_RST  PD0 // пин аппаратного сброса дисплея подключён к PD0

// назначение для отрисовки на дисплее

#define LCD_XSTART 0 // смещение по X (LCD_XSTART поправка по горизонтали)
#define LCD_YSTART 0 // смещение по Y (LCD_YSTART поправка по вертикали)
#define LCD_MADCTL 0xC0 // значение для команды MADCTL || оно управляет ориентацией экрана и порядком обхода памяти(описание приведено в документации на ST7735)

// макроподстановки для цветов в формате RGB565 (начальная палитра || сайт для подбора значения цветов в формате RGB565:https://rgbcolorpicker.com/565)

#define COLOR_BLACK  0x0000 // чёрный цвет
#define COLOR_WHITE  0xFFFF // белый цвет
#define COLOR_RED    0xF800 // красный цвет
#define COLOR_GREEN  0x07E0 // зелёный цвет
#define COLOR_BLUE   0x001F // синий цвет
#define COLOR_YELLOW 0xFFE0 // жёлтый цвет

// крестовина для управления системой

#define BTN_LEFT   PD2 // левая кнопка
#define BTN_UP     PD3 // верхняя кнопка
#define BTN_CENTER PD4 // центральная кнопка
#define BTN_DOWN   PD5 // нижняя кнопка
#define BTN_RIGHT  PD6 // правая кнопка


// макроподстановка для клавиатуры

#define KEY_NONE 0 // значение, которое возвращает клавиатура, если кнопка не нажата

// макроподстановки для режимов экранов

#define SCREEN_HOME           0 // главный экран
#define SCREEN_BOARD_SELECT   1 // экран выбора платы
#define SCREEN_BOARD2_MENU    2 // меню платы №2
#define SCREEN_KEYBOARD_TEST  3 // тест клавиатуры
#define SCREEN_SERVO_TEST     4 // тест сервопривода
#define SCREEN_STEPPER_TEST   5 // тест шагового мотора
#define SCREEN_BOARD1_MENU    6 // меню платы №1
#define SCREEN_SEVEN_SEGMENT_TEST 7 // экран теста семисегментных индикаторов
#define SCREEN_LED_BAR_TEST 8 // экран теста 8 светодиодов
#define SCREEN_SPEAKER_TEST 9 // экран теста спикера
#define SCREEN_RTC_TEST 10 // экран теста часов реального времени
#define SCREEN_TEMP_LCD_TEST 11 // экран теста MAX31865 и LCD 16x2

// макроподстановки для меню платы №1

#define BOARD1_MENU_COUNT 3 // общее количество пунктов меню платы №1
#define BOARD1_MENU_VISIBLE 3 // количество пунктов меню, которые одновременно видны на экране

// макроподстановки для пунктов меню платы №1

#define BOARD1_MENU_ITEM_SEVEN_SEGMENT_TEST 0 // первый пункт меню платы №1: тест семисегментных индикаторов
#define BOARD1_MENU_ITEM_LED_BAR_TEST 1 // второй пункт меню платы №1: тест 8 светодиодов
#define BOARD1_MENU_ITEM_SPEAKER_TEST 2 // третий пункт меню платы №1: тест спикера

// макроподстановки для меню платы №2

#define MENU_COUNT 5 // общее количество пунктов меню платы №2
#define MENU_VISIBLE 4 // количество пунктов меню, которые одновременно видны на экране

// макроподстановки для пунктов меню платы №2

#define MENU_ITEM_KEYBOARD_TEST 0 // первый пункт меню платы №2: тест клавиатуры
#define MENU_ITEM_SERVO_TEST    1 // второй пункт меню платы №2: тест сервопривода
#define MENU_ITEM_STEPPER_TEST 2 // третий пункт меню платы №2: тест шагового мотора
#define MENU_ITEM_TEMP_LCD_TEST 3 // четвертый пункт меню платы №2: тест MAX31865 и LCD 16x2
#define MENU_ITEM_RTC_TEST 4 // пятый пункт меню платы №2: тест DS1307 (часов реального времени)

// объявления глобальных переменные для управления меню

extern uint8_t menu_selected;
extern uint8_t menu_top;
extern uint8_t screen_mode;

#endif /* MAIN_H_ */