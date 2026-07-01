/*
 * speaker_test.h
 *
 * Created: 14.05.2026 0:12:54
 *  Author: Владислав
 */ 


#include "../../main.h"
#include "../../lcd/lcd.h"
#include "../../menus/menu_board_1/menu_board_1.h"
#include "../../speaker/speaker.h"
#include "speaker_test.h"

typedef struct
{
	uint16_t freq; // частота ноты, 0 означает паузу
	uint16_t duration_ms; // длительность ноты в миллисекундах
	
} speaker_note_t;

// последовательность нот тестовой мелодии

static const speaker_note_t speaker_melody[] =
{
	{2093, 120},
	{2637, 120},
	{3136, 120},
	{4186, 220},
	{3136, 120},
	{4186, 300},
	{0,    80},
	{2637, 120},
	{3136, 250}
};

#define SPEAKER_MELODY_LENGTH (sizeof(speaker_melody) / sizeof(speaker_melody[0])) // количество нот в мелодии

#define SPEAKER_PROCESS_TICK_MS 5 // каждые 5 мс проверяем, не пора ли перейти к следующей ноте

static char last_speaker_key = KEY_NONE; // последняя нажатая управляющая кнопка нужна для выхода по двойному нажатию 5

static uint8_t speaker_melody_active = 0; // 0 — мелодия выключена, 1 — мелодия играет

static uint8_t speaker_melody_index = 0; // индекс текущей ноты мелодии

static uint16_t speaker_note_time = 0; // сколько миллисекунд уже играет текущая нота

// отрисовываем состояние спикера

static void screen_speaker_test_draw_status(const char *text)
{
	lcd_fill_rect(0, 94, 128, 18, COLOR_BLACK); // очищаем строку статуса

	lcd_print_string(4, 98, text, COLOR_GREEN, COLOR_BLACK, 1); // печатаем статус
}

// запускаем мелодию с начала

static void screen_speaker_test_start_melody(void)
{
	speaker_melody_active = 1; // включаем флаг мелодии

	speaker_melody_index = 0; // начинаем с первой ноты

	speaker_note_time = 0; // сбрасываем счётчик времени ноты

	if (speaker_melody[0].freq == 0)
	{
		speaker_tone_stop();
	}
	else
	{
		speaker_tone_start(speaker_melody[0].freq);
	}
	// запускаем первую ноту

	screen_speaker_test_draw_status("ИГРАЕТ"); // показываем статус
}

// останавливаем мелодию

static void screen_speaker_test_stop_melody(void)
{

	speaker_melody_active = 0; // сбрасываем флаг мелодии

	speaker_melody_index = 0; // возвращаем индекс к началу

	speaker_note_time = 0; // сбрасываем время ноты

	speaker_tone_stop(); // выключаем звук

	screen_speaker_test_draw_status("СТОП"); // показываем статус
}

// обрабатываем воспроизведение мелодии

static void screen_speaker_test_process_melody(void)
{
	

	if (speaker_melody_active == 0)
	{
		// если мелодия не играет

		return; // завершаем обработку при выключенной мелодии
	}

	_delay_ms(SPEAKER_PROCESS_TICK_MS); // задержка между проверками длительности ноты

	speaker_note_time += SPEAKER_PROCESS_TICK_MS; // увеличиваем время текущей ноты

	if (speaker_note_time < speaker_melody[speaker_melody_index].duration_ms)
	{
		// если текущая нота ещё не закончилась

		return; // пока остаёмся на ней
	}

	speaker_melody_index++; // переходим к следующей ноте

	speaker_note_time = 0; // сбрасываем время новой ноты

	if (speaker_melody_index >= SPEAKER_MELODY_LENGTH)
	{
		// если мелодия закончилась

		screen_speaker_test_stop_melody(); // останавливаем звук

		return;
	}

	if (speaker_melody[speaker_melody_index].freq == 0)
	{
		// если частота 0, это пауза

		speaker_tone_stop();
	}
	else
	{
		// иначе запускаем новую ноту

		speaker_tone_start(speaker_melody[speaker_melody_index].freq);
	}
}

// открываем экран теста спикера

void screen_speaker_test_start(void)
{

	lcd_fill_color(COLOR_BLACK); // очищаем TFT-экран

	last_speaker_key = KEY_NONE; // сбрасываем последнюю кнопку

	speaker_init(); // инициализируем PD7 и Timer2

	speaker_tone_stop(); // выключаем звук при открытии экрана

	speaker_melody_active = 0; // мелодия при входе выключена

	speaker_melody_index = 0; // индекс мелодии в начало

	speaker_note_time = 0; // время ноты в 0

	lcd_print_string(4, 4, "ТЕСТ СПИКЕРА", COLOR_WHITE, COLOR_BLACK, 1); // заголовок экрана

	lcd_print_string(4, 28, "ЦЕНТР ПУСК", COLOR_WHITE, COLOR_BLACK, 1); // центральная кнопка запускает мелодию

	lcd_print_string(4, 44, "ЛЕВО СТОП", COLOR_WHITE, COLOR_BLACK, 1); // левая кнопка останавливает звук

	lcd_print_string(4, 60, "ВЕРХ ВЫСОКИЙ", COLOR_WHITE, COLOR_BLACK, 1); // верхняя кнопка — короткий высокий сигнал

	lcd_print_string(4, 76, "НИЗ НИЗКИЙ", COLOR_WHITE, COLOR_BLACK, 1); // нижняя кнопка — короткий низкий сигнал

	lcd_print_string(4, 144, "2Р ЦЕНТР ВЫХОД", COLOR_YELLOW, COLOR_BLACK, 1); // двойное нажатие центральной кнопки — выход

	screen_speaker_test_draw_status("СТОП"); // начальный статус
}

// обрабатываем кнопку на экране теста спикера

void screen_speaker_test_key(char key)
{
	screen_speaker_test_process_melody(); // мелодия должна продолжаться даже тогда, когда кнопки не нажимаются

	if (key == KEY_NONE)
	{
		return;
	}
	// если кнопка не нажата, дальше ничего не обрабатываем

	if (key == '5' && last_speaker_key == '5')
	{
		// если два раза подряд нажали центральную кнопку, выходим обратно в меню платы номер 1

		screen_speaker_test_stop_melody(); // останавливаем звук

		screen_mode = SCREEN_BOARD1_MENU; // возвращаемся в меню платы номер 1

		last_speaker_key = KEY_NONE; // сбрасываем последнюю кнопку

		menu_board_1_draw(); // рисуем меню платы номер 1

		return;
	}

	if (key == '5')
	{
		// одиночное нажатие центральной кнопки запускаем мелодию с начала

		screen_speaker_test_start_melody();

		last_speaker_key = key; // запоминаем кнопку, чтобы второе нажатие 5 считалось выходом

		return;
	}

	if (key == '4')
	{
		// левая кнопка — остановить звук

		screen_speaker_test_stop_melody();
	}

	if (key == '2')
	{
		// верхняя кнопка — короткий высокий звук

		screen_speaker_test_stop_melody(); // сначала останавливаем мелодию

		screen_speaker_test_draw_status("ВЫСОКИЙ"); // показываем статус

		speaker_beep(3500, 120); // короткий высокий сигнал

		screen_speaker_test_draw_status("СТОП"); // возвращаем статус
	}

	if (key == '8')
	{
		// нижняя кнопка — короткий низкий звук

		screen_speaker_test_stop_melody(); // сначала останавливаем мелодию

		screen_speaker_test_draw_status("НИЗКИЙ"); // показываем статус

		speaker_beep(800, 120); // короткий низкий сигнал

		screen_speaker_test_draw_status("СТОП"); // возвращаем статус
	}

	last_speaker_key = key; // запоминаем последнюю кнопку для выхода
}
