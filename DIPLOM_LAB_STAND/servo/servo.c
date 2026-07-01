/*
 * servo.c
 *
 * Created: 14.04.2026 22:58:10
 *  Author: Владислав
 */

#include "../main.h"
#include "servo.h"
#include <util/atomic.h>

#define SERVO_PIN PD7 // вывод PD7 — физический пин 21 ATmega1284P на этот пин будет идти сигнал управления сервой

#define SERVO_MIN_PULSE 800 // минимальный импульс сервопривода в микросекундах

#define SERVO_MAX_PULSE 2400 // максимальный импульс сервопривода в микросекундах

#define SERVO_PERIOD 20000 // период сигнала сервопривода в микросекундах 20000 мкс = 20 мс = 50 Гц

#define SERVO_STEP 5 // шаг поворота сервопривода в градусах за одно нажатие кнопки

#define SERVO_TIMER_TICKS_PER_US 1 // количество тиков Timer1 в одной микросекунде

static uint8_t servo_angle = 90; // текущий угол сервопривода начинаем с 90 градусов, то есть примерно с середины

static volatile uint16_t servo_pulse_us = 1500; // текущая длительность высокого импульса в микросекундах модификатор volatile нужен, потому что переменная используется и в основном коде, и в прерывании

static volatile uint8_t servo_phase = 0; // фаза сигнала: 0 — начать высокий импульсь, 1 — закончить высокий импульс

// инициализируем управление сервоприводом

void servo_init(void)
{
	DDRD |= (1 << SERVO_PIN); // вывод PD7 становится выходом

	PORTD &= ~(1 << SERVO_PIN); // сначала ставим PD7 в 0

	TCCR1A = 0; // очищаем регистр настройки Timer1 A

	TCCR1B = 0; // очищаем регистр настройки Timer1 B

	TIMSK1 = 0; // запрещаем прерывания Timer1, пока настраиваем таймер

	TCNT1 = 0; // сбрасываем счётчик Timer1

	OCR1A = 100; // первое короткое значение сравнения, после него начнётся первый импульс

	TCCR1B |= (1 << WGM12); // включаем режим CTC Timer1 таймер считает до OCR1A, вызывает прерывание и сбрасывается

	TCCR1B |= (1 << CS11); // запускаем Timer1 с делителем 8

	TIFR1 = (1 << OCF1A); // сбрасываем флаг совпадения Timer1 Compare A

	TIMSK1 |= (1 << OCIE1A); // разрешаем прерывание Timer1 Compare A

	servo_phase = 0; // начинаем с фазы старта импульса

	// устанавливаем угол сервопривода
	servo_set_angle(90); // ставим сервопривод в среднее положение
}

// задаём длительность управляющего импульса в микросекундах

static void servo_set_pulse_us(uint16_t pulse_us)
{
	if (pulse_us < SERVO_MIN_PULSE)
	{
		pulse_us = SERVO_MIN_PULSE;
	}
	// если значение слишком маленькое, ограничиваем его безопасным минимумом

	if (pulse_us > SERVO_MAX_PULSE)
	{
		pulse_us = SERVO_MAX_PULSE;
	}
	// если значение слишком большое, ограничиваем его безопасным максимумом

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		servo_pulse_us = pulse_us;
	}
}

// задаем угол

void servo_set_angle(uint8_t angle)
{
	uint16_t pulse; // здесь будет рассчитанная длительность импульса

	if (angle > 180)
	{
		angle = 180;
	}
	// ограничиваем угол диапазоном 0..180 градусов

	servo_angle = angle; // запоминаем текущий угол

	pulse = SERVO_MIN_PULSE; // начинаем с минимального импульса

	pulse += ((uint32_t)(SERVO_MAX_PULSE - SERVO_MIN_PULSE) * (180 - angle)) / 180; // переводим угол в длительность импульса:
	// значение ANGLE = 0   -> 600 мкс
	// значение ANGLE = 90  -> 1500 мкс
	// значение ANGLE = 180 -> 2400 мкс

	servo_set_pulse_us(pulse); // передаём рассчитанный импульс в генератор сигнала
}

// поворачиваем сервопривод влево на один шаг

void servo_left(void)
{
	if (servo_angle >= SERVO_STEP)
	{
		servo_angle -= SERVO_STEP;
	}
	else
	{
		servo_angle = 0;
	}

	servo_set_angle(servo_angle);
}

// поворачиваем сервопривод вправо на один шаг

void servo_right(void)
{
	if (servo_angle <= 180 - SERVO_STEP)
	{
		servo_angle += SERVO_STEP;
	}
	else
	{
		servo_angle = 180;
	}

	servo_set_angle(servo_angle);
}

// поворачиваем сервопривод максимально вправо

void max_servo_right(void)
{
	servo_angle = 180;

	servo_set_angle(servo_angle);
}

// возвращаем текущий угол сервопривода

uint8_t servo_get_angle(void)
{
	return servo_angle;
}

// останавливаем сервосигнал

void servo_stop(void)
{
	TIMSK1 &= ~(1 << OCIE1A); // запрещаем прерывание Timer1 Compare A

	TCCR1A = 0; // сбрасываем настройки Timer1 A
	TCCR1B = 0; // останавливаем Timer1

	PORTD &= ~(1 << SERVO_PIN); // вывод PD7 = 0 сигнал сервопривода выключен

	servo_phase = 0; // сбрасываем фазу формирования сигнала
}

// инициализируем прерывание для timer1

ISR(TIMER1_COMPA_vect)
{
	uint16_t pulse_copy; // копия длительности импульса

	uint16_t timer_ticks; // количество тиков Timer1

	if (servo_phase == 0)
	{
		// начинаем высокий импульс сервосигнала

		PORTD |= (1 << SERVO_PIN); // вывод PD7 = 1

		pulse_copy = servo_pulse_us; // берём текущую длительность импульса в микросекундах

		timer_ticks = (uint16_t)((uint32_t)pulse_copy * SERVO_TIMER_TICKS_PER_US); // переводим микросекунды в тики Timer1

		OCR1A = timer_ticks - 1; // следующее прерывание будет через pulse_copy микросекунд

		servo_phase = 1; // следующая фаза — закончить высокий импульс
	}
	else
	{
		// заканчиваем высокий импульс

		PORTD &= ~(1 << SERVO_PIN); // вывод PD7 = 0

		pulse_copy = servo_pulse_us; // берём текущую длительность импульса

		timer_ticks = (uint16_t)((uint32_t)(SERVO_PERIOD - pulse_copy) * SERVO_TIMER_TICKS_PER_US); // считаем остаток периода

		OCR1A = timer_ticks - 1; // ждём остаток периода до 20 мс

		servo_phase = 0; // следующая фаза — начать новый импульс
	}
}
