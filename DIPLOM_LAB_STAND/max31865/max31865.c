/*
 * max31865.c
 *
 * Created: 21.05.2026 1:53:41
 *  Author: Владислав
 */ 


/*
 * важно:
 * дисплей TFT ST7735 обычно работает в SPI mode 0
 * модуль MAX31865 работает в SPI mode 1 или mode 3
 *
 * поэтому перед обменом с MAX31865 переключаем SPI в mode 1,
 * а после обмена возвращаем SPI в mode 0 для TFT
 *
 */

#include "../main.h"
#include "max31865.h"

#define MAX31865_CS_PORT PORTA // порт линии CS модуля MAX31865
#define MAX31865_CS_DDR  DDRA // регистр направления линии CS
#define MAX31865_CS_PIN  PA7 // пин линии CS модуля MAX31865

#define MAX31865_REG_CONFIG       0x00 // адрес регистра конфигурации
#define MAX31865_REG_RTD_MSB      0x01 // адрес старшего байта значения RTD
#define MAX31865_REG_RTD_LSB      0x02 // адрес младшего байта значения RTD
#define MAX31865_REG_FAULT_STATUS 0x07 // адрес регистра состояния ошибок

#define MAX31865_WRITE_MASK 0x80 // маска бита записи в регистр MAX31865

#define MAX31865_CONFIG_VBIAS       0b10000000 // значение VBIAS = 1 включает смещение RTD-цепи

#define MAX31865_CONFIG_AUTO        0b01000000 // значение AUTO conversion = 1 модуль MAX31865 сам периодически измеряет сопротивление

#define MAX31865_CONFIG_3WIRE       0b00010000 // 3-wire mode нужен для трёхпроводного PT100

#define MAX31865_CONFIG_FAULT_CLEAR 0b00000010 // значение FAULT status clear сброс флагов ошибки

#define MAX31865_CONFIG_50HZ        0b00000001 // фильтр 50 Гц для стабильности показаний

#define MAX31865_USE_3WIRE 1 // 1 — PT100 подключён тремя проводами, 0 - 2 или 4 провода


#define MAX31865_RREF_CENTI_OHMS 43000L // номинал эталонного резистора Rref в сотых долях Ома 43000 = 430.00 Ом

#define MAX31865_AVERAGE_SAMPLES 4 // количество измерений для усреднения

#define MAX31865_SAMPLE_DELAY_MS 65 // задержка между измерениями

static uint8_t max31865_make_config(void)
{
	uint8_t config;

	// включаем:
	// значение VBIAS = 1
	// значение AUTO = 1
	// значение FAULT CLEAR = 1
	// 50 Hz filter = 1
	
	config = MAX31865_CONFIG_VBIAS | MAX31865_CONFIG_AUTO | MAX31865_CONFIG_FAULT_CLEAR | MAX31865_CONFIG_50HZ;

	if (MAX31865_USE_3WIRE)
	{
		config |= MAX31865_CONFIG_3WIRE; // включаем режим 3-wire для трёхпроводного PT100
	}

	return config;
}

static void spi_mode_tft(void)
{
	SPCR &= ~((1 << CPOL) | (1 << CPHA));
}

static void spi_mode_max31865(void)
{
	SPCR &= ~(1 << CPOL);
	SPCR |= (1 << CPHA);
}

static uint8_t max31865_spi_transfer(uint8_t data)
{
	SPDR = data; // записываем байт в SPI

	while (!(SPSR & (1 << SPIF))); // ждём окончания передачи

	return SPDR; // возвращаем принятый байт
}

static void max31865_select(void)
{
	PORTB |= (1 << LCD_CS);
	// отключаем TFT, чтобы на SPI-шине слушал только MAX31865, дисплей TFT CS активен по нулю, поэтому 1 = дисплей не выбран

	spi_mode_max31865(); // переключаем SPI в режим MAX31865

	MAX31865_CS_PORT &= ~(1 << MAX31865_CS_PIN); // значение CS MAX31865 = 0, модуль MAX31865 выбран
}

static void max31865_unselect(void)
{
	MAX31865_CS_PORT |= (1 << MAX31865_CS_PIN); // значение CS MAX31865 = 1, модуль MAX31865 отключён от SPI

	spi_mode_tft(); // возвращаем SPI mode 0 для TFT ST7735
}

static void max31865_write_reg(uint8_t reg, uint8_t value)
{
	max31865_select(); // выбор модуля MAX31865 для обмена по SPI

	max31865_spi_transfer(reg | MAX31865_WRITE_MASK); // адрес регистра с битом записи

	max31865_spi_transfer(value); // значение

	max31865_unselect(); // завершение обмена с модулем MAX31865
}

static uint8_t max31865_read_reg(uint8_t reg)
{
	uint8_t value;

	max31865_select();

	max31865_spi_transfer(reg & 0x7F); // адрес регистра для чтения

	value = max31865_spi_transfer(0xFF); // читаем значение

	max31865_unselect();

	return value;
}

static void max31865_clear_faults(void)
{
	max31865_write_reg(MAX31865_REG_CONFIG, max31865_make_config());
}

uint8_t max31865_read_fault(void)
{
	return max31865_read_reg(MAX31865_REG_FAULT_STATUS);
}

static uint16_t max31865_read_rtd_raw_once(void)
{
	uint8_t msb;
	uint8_t lsb;
	uint16_t rtd_raw;

	msb = max31865_read_reg(MAX31865_REG_RTD_MSB);
	lsb = max31865_read_reg(MAX31865_REG_RTD_LSB);

	if (lsb & 0x01)
	{
		// младший бит LSB — признак ошибки, значение RTD в этом случае использовать нельзя

		(void)max31865_read_fault(); // читаем fault-регистр, чтобы можно было диагностировать ошибку

		max31865_clear_faults(); // сбрасываем ошибку

		return 0;
	}

	rtd_raw = ((uint16_t)msb << 8) | lsb;
	rtd_raw >>= 1; // получаем 15-битное значение RTD

	return rtd_raw;
}

static uint16_t max31865_read_rtd_raw_average(uint8_t samples)
{
	uint8_t i;
	uint8_t good_samples;
	uint16_t raw;
	uint32_t sum;

	good_samples = 0;
	sum = 0;

	for (i = 0; i < samples; i++)
	{
		raw = max31865_read_rtd_raw_once();

		if (raw != 0)
		{
			sum += raw;
			good_samples++;
		}

		_delay_ms(MAX31865_SAMPLE_DELAY_MS); // небольшая пауза между чтениями
	}

	if (good_samples == 0)
	{
		return 0;
	}

	return (uint16_t)(sum / good_samples);
}

void max31865_init(void)
{
	MAX31865_CS_DDR |= (1 << MAX31865_CS_PIN); // вывод PA7 делаем выходом для CS MAX31865

	MAX31865_CS_PORT |= (1 << MAX31865_CS_PIN); // модуль MAX31865 пока не выбран

	// вывод PB5 MOSI — выход
	// вывод PB7 SCK  — выход
	// вывод PB4 CS TFT — выход

	DDRB |= (1 << PB5) | (1 << PB7) | (1 << LCD_CS);

	DDRB &= ~(1 << PB6); // вывод PB6 MISO — вход

	PORTB |= (1 << LCD_CS); // дисплей TFT не выбран

	// настраиваем MAX31865:
	// значение VBIAS = 1,
	// значение AUTO = 1,
	// 3WIRE = 1,
	// 50Hz = 1,
	// значение FAULT clear = 1
	
	max31865_clear_faults();

	_delay_ms(150); // даём MAX31865 время на первое измерение
}

int16_t max31865_read_temp_c10(void)
{
	uint16_t rtd_raw;
	int32_t resistance_centi_ohm;
	int32_t temp_c10;

	rtd_raw = max31865_read_rtd_raw_average(MAX31865_AVERAGE_SAMPLES); // читаем RTD несколько раз и усредняем

	if (rtd_raw == 0)
	{
		return MAX31865_ERROR_TEMP;
	}

	// сопротивление RTD в сотых долях Ома
	// например:
	// 10000 = 100.00 Ом
	// формула:
	// значение RRTD = raw * Rref / 32768

	resistance_centi_ohm = ((int32_t)rtd_raw * MAX31865_RREF_CENTI_OHMS) / 32768L;
	
	// приближённый пересчёт для PT100:
	// 0 °C  примерно 100.00 Ом
	// 1 °C  примерно +0.385 Ом
	// значение TEMP_C10 — температура в десятых долях градуса

	temp_c10 = ((resistance_centi_ohm - 10000L) * 100L) / 385L;

	return (int16_t)temp_c10;
}
