/*
 * speaker.c
 *
 * Created: 14.05.2026 0:09:37
 *  Author: Владислав
 */ 

#include "../main.h"
#include "speaker.h"

#define SPEAKER_PIN PD7 // пин спикера

#define SPEAKER_PORT PORTD // порт спикера

#define SPEAKER_DDR DDRD // регистр направления порта спикера

// инициализируем спикер

void speaker_init(void)
{
	SPEAKER_DDR |= (1 << SPEAKER_PIN); // вывод PD7 делаем выходом

	SPEAKER_PORT &= ~(1 << SPEAKER_PIN); // на PD7 ставим 0

// останавливаем генерацию звука
	speaker_tone_stop(); // выключаем звук после инициализации
}

// таймер Timer2 срабатывает с частотой в 2 раза выше нужной частоты звука, обработчик прерывания переключает состояние PD7, в AVR запись 1 в PIND переключает соответствующий выход PORTD

ISR(TIMER2_COMPA_vect)
{
	PIND = (1 << SPEAKER_PIN); // переключаем PD7
}

void speaker_tone_stop(void)
{
	TIMSK2 &= ~(1 << OCIE2A); // запрещаем прерывание Timer2 Compare A

	TCCR2B = 0; // останавливаем Timer2

	TCCR2A = 0; // сбрасываем режим Timer2

	SPEAKER_PORT &= ~(1 << SPEAKER_PIN); // на выходе спикера ставим 0
}

// запускаем звук заданной частоты

void speaker_tone_start(uint16_t freq_hz)
{
	uint32_t ocr;

	if (freq_hz == 0)
	{
		// частота 0 означает паузу

		speaker_tone_stop();
		return;
	}

	ocr = (F_CPU / (2UL * 64UL * freq_hz));

	if (ocr == 0)
	{
		ocr = 1;
	}

	ocr--; // регистр OCR2A считает от 0 до OCR2A включительно

	if (ocr > 255)
	{
		ocr = 255;
	}
	// таймер Timer2 восьмибитный, поэтому OCR2A не может быть больше 255

	speaker_tone_stop(); // сначала выключаем старый звук

	OCR2A = (uint8_t)ocr; // задаём частоту

	TCNT2 = 0; // сбрасываем счётчик Timer2

	TCCR2A = (1 << WGM21); // режим CTC, таймер Timer2 считает до OCR2A

	TCCR2B = (1 << CS22); // делитель Timer2 = 64

	TIMSK2 |= (1 << OCIE2A); // разрешаем прерывание по совпадению с OCR2A
}

// воспроизводим короткий звуковой сигнал

void speaker_beep(uint16_t freq_hz, uint16_t duration_ms)
{
	speaker_tone_start(freq_hz); // запускаем звук

	while (duration_ms > 0)
	{
		_delay_ms(1);
		duration_ms--;
	}
	// ждём нужную длительность
	speaker_tone_stop(); // останавливаем звук
}
