/*
 * rtc.c
 *
 * Created: 18.05.2026 23:33:08
 *  Author: Владислав
 */ 


#include "../main.h"
#include "../twi/twi.h"
#include "rtc.h"

#define DS1307_ADDRESS_WRITE 0b11010000 // адрес DS1307 для записи 0x68 << 1 + бит записи 0 = 0xD0

#define DS1307_ADDRESS_READ  0b11010001 // адрес DS1307 для чтения 0x68 << 1 + бит чтения 1 = 0xD1

#define DS1307_REG_SECONDS 0x00 // первый регистр DS1307 — секунды

uint8_t RTC_ConvertFromDec(uint8_t value)
{
	uint8_t result;

	result = ((value >> 4) * 10) + (value & 0b00001111); // старшая тетрада BCD — десятки, младшая тетрада BCD — единицы

	return result;
}

uint8_t RTC_ConvertFromBinDec(uint8_t value)
{
	uint8_t result;

	result = ((value / 10) << 4) | (value % 10); // десятки переносим в старшую тетраду, единицы оставляем в младшей тетраде

	return result;
}

void rtc_init(void)
{
	I2C_Init(); // настраиваем аппаратный I2C / TWI
}

void rtc_read(rtc_time_t *time)
{
	uint8_t sec_bcd;
	uint8_t min_bcd;
	uint8_t hour_bcd;
	uint8_t day_bcd;
	uint8_t date_bcd;
	uint8_t month_bcd;
	uint8_t year_bcd;

	I2C_StartCondition(); // сигнал START

	I2C_SendByte(DS1307_ADDRESS_WRITE); // обращаемся к DS1307 на запись

	I2C_SendByte(DS1307_REG_SECONDS); // ставим указатель регистра на 0x00

	I2C_StartCondition(); // повторный START

	I2C_SendByte(DS1307_ADDRESS_READ); // обращаемся к DS1307 на чтение

	sec_bcd = I2C_ReadByte();
	min_bcd = I2C_ReadByte();
	hour_bcd = I2C_ReadByte();
	day_bcd = I2C_ReadByte();
	date_bcd = I2C_ReadByte();
	month_bcd = I2C_ReadByte();
	year_bcd = I2C_ReadLastByte();

	I2C_StopCondition(); // сигнал STOP

	time->sec = RTC_ConvertFromDec(sec_bcd & 0b01111111); // в регистре секунд старший бит CH значение CH = Clock Halt, поэтому при чтении секунд его убираем

	time->min = RTC_ConvertFromDec(min_bcd);
	time->hour = RTC_ConvertFromDec(hour_bcd & 0b00111111); // используем 24-часовой режим

	time->day = RTC_ConvertFromDec(day_bcd);
	time->date = RTC_ConvertFromDec(date_bcd);
	time->month = RTC_ConvertFromDec(month_bcd & 0b00011111);
	time->year = RTC_ConvertFromDec(year_bcd);

	if (time->day < 1 || time->day > 7)
	{
		time->day = 1;
	}

	if (time->date < 1 || time->date > 31)
	{
		time->date = 1;
	}

	if (time->month < 1 || time->month > 12)
	{
		time->month = 1;
	}

	if (time->hour > 23)
	{
		time->hour = 0;
	}

	if (time->min > 59)
	{
		time->min = 0;
	}

	if (time->sec > 59)
	{
		time->sec = 0;
	}
}

void rtc_write(const rtc_time_t *time)
{
	I2C_StartCondition(); // сигнал START

	I2C_SendByte(DS1307_ADDRESS_WRITE); // адрес DS1307 на запись

	I2C_SendByte(DS1307_REG_SECONDS); // начинаем запись с регистра секунд 0x00

	I2C_SendByte(RTC_ConvertFromBinDec(time->sec) & 0b01111111); // секунды || старший бит CH сбрасываем в 0, чтобы часы шли

	I2C_SendByte(RTC_ConvertFromBinDec(time->min)); // минуты

	I2C_SendByte(RTC_ConvertFromBinDec(time->hour)); // часы в 24-часовом формате

	I2C_SendByte(RTC_ConvertFromBinDec(time->day)); // день недели

	I2C_SendByte(RTC_ConvertFromBinDec(time->date)); // число месяца

	I2C_SendByte(RTC_ConvertFromBinDec(time->month)); // месяц

	I2C_SendByte(RTC_ConvertFromBinDec(time->year)); // год 0..99

	I2C_StopCondition(); // сигнал STOP
}