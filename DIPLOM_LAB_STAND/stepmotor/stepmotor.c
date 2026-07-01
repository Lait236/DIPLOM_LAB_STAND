/*
 * stepmotor.c
 *
 * Created: 22.04.2026 0:29:54
 *  Author: Владислав
 */ 


#include "../main.h"
#include "stepmotor.h"

// назначение выводов шагового мотора

#define SM_IN1_PORT PORTB // порт первого входа драйвера
#define SM_IN1_DDR  DDRB  // регистр направления первого входа драйвера
#define SM_IN1_PIN  PB0   // пин первого входа драйвера

#define SM_IN2_PORT PORTB // порт второго входа драйвера
#define SM_IN2_DDR  DDRB  // регистр направления второго входа драйвера
#define SM_IN2_PIN  PB1   // пин второго входа драйвера

#define SM_IN3_PORT PORTB // порт третьего входа драйвера
#define SM_IN3_DDR  DDRB  // регистр направления третьего входа драйвера
#define SM_IN3_PIN  PB2   // пин третьего входа драйвера

#define SM_IN4_PORT PORTB // порт четвёртого входа драйвера
#define SM_IN4_DDR  DDRB  // регистр направления четвёртого входа драйвера
#define SM_IN4_PIN  PB3   // пин четвёртого входа драйвера

#define SM_DEFAULT_DELAY_US 800 // задержка по умолчанию между полушагами

static uint8_t sm_phase = 0; // текущая фаза шагового мотора

// выполняем задержку между полушагами

static void SM_delay_us(uint16_t delay_us)
{
	while (delay_us > 0)
	{
		_delay_us(1);
		delay_us--;
	}
}


// устанавливаем состояние управляющего вывода

static void SM_write_pin(volatile uint8_t *port, uint8_t pin, uint8_t value)
{
	if (value)
	{
		*port |= (1 << pin);
	}
	else
	{
		*port &= ~(1 << pin);
	}
}


// устанавливаем состояния входов драйвера

static void SM_set_outputs(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4)
{
	SM_write_pin(&SM_IN1_PORT, SM_IN1_PIN, in1);
	SM_write_pin(&SM_IN2_PORT, SM_IN2_PIN, in2);
	SM_write_pin(&SM_IN3_PORT, SM_IN3_PIN, in3);
	SM_write_pin(&SM_IN4_PORT, SM_IN4_PIN, in4);
}


// устанавливаем выбранную фазу шагового мотора

static void SM_apply_phase(uint8_t phase)
{

	if (phase == 0)
	{
		SM_set_outputs(1, 0, 0, 0);
	}

	if (phase == 1)
	{
		SM_set_outputs(1, 0, 0, 1);
	}

	if (phase == 2)
	{
		SM_set_outputs(0, 0, 0, 1);
	}

	if (phase == 3)
	{
		SM_set_outputs(0, 0, 1, 1);
	}

	if (phase == 4)
	{
		SM_set_outputs(0, 0, 1, 0);
	}

	if (phase == 5)
	{
		SM_set_outputs(0, 1, 1, 0);
	}

	if (phase == 6)
	{
		SM_set_outputs(0, 1, 0, 0);
	}

	if (phase == 7)
	{
		SM_set_outputs(1, 1, 0, 0);
	}
}


// инициализируем выводы шагового мотора

void SM_init(void)
{
	SM_IN1_DDR |= (1 << SM_IN1_PIN);
	SM_IN2_DDR |= (1 << SM_IN2_PIN);
	SM_IN3_DDR |= (1 << SM_IN3_PIN);
	SM_IN4_DDR |= (1 << SM_IN4_PIN); 

	SM_off(); // сначала выключаем все обмотки

	sm_phase = 0; // начинаем с нулевой фазы
}

void SM_off(void)
{
	SM_set_outputs(0, 0, 0, 0); // на всех входах драйвера 0, обмотки мотора отключены
}


// выполняем один полушаг вперёд

void SM_step_forward(uint16_t delay_us)
{
	SM_apply_phase(sm_phase); // выставляем текущую фазу

	SM_delay_us(delay_us); // ждём, чтобы мотор успел сдвинуться

	sm_phase++; // переходим к следующей фазе

	if (sm_phase > 7)
	{
		sm_phase = 0;
	}
	// после 7 снова возвращаемся к 0
}


// выполняем один полушаг назад

void SM_step_back(uint16_t delay_us)
{
	if (sm_phase == 0)
	{
		sm_phase = 7;
	}
	else
	{
		sm_phase--;
	}
	// переходим к предыдущей фазе

	SM_apply_phase(sm_phase); // выставляем фазу

	SM_delay_us(delay_us); // ждём
}


// выполняем несколько полушагов вперёд

void SM_forward_steps(uint16_t steps, uint16_t delay_us)
{
	uint16_t i;

	for (i = 0; i < steps; i++)
	{
		SM_step_forward(delay_us);
	}
}


// выполняем несколько полушагов назад

void SM_back_steps(uint16_t steps, uint16_t delay_us)
{
	uint16_t i;
	for (i = 0; i < steps; i++)
	{
		SM_step_back(delay_us);
	}
}


// выполняем восемь полушагов вперёд

void SM_forvard(void)
{
	SM_forward_steps(8, SM_DEFAULT_DELAY_US);
}

// выполняем восемь полушагов назад

void SM_back(void)
{
	SM_back_steps(8, SM_DEFAULT_DELAY_US);
}
