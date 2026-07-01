/*
 * rtc.h
 *
 * Created: 18.05.2026 23:32:55
 *  Author: Владислав
 */ 


#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>

// структура для установки времени DS1307
typedef struct
{
	uint8_t sec; // значение SEC   — секунды 0..59
	uint8_t min; // значение MIN   — минуты 0..59
	uint8_t hour; // значение HOUR  — часы 0..23
	uint8_t day; // значение DAY   — день недели 1..7
	uint8_t date; // значение DATE  — число месяца 1..31
	uint8_t month; // значение MONTH — месяц 1..12
	uint8_t year; // значение YEAR  — год 0..99
} rtc_time_t; 

void rtc_init(void); // инициализируем I2C для DS1307
void rtc_read(rtc_time_t *time); // читаем дату и время из DS1307
void rtc_write(const rtc_time_t *time); // записываем дату и время в DS1307
uint8_t RTC_ConvertFromDec(uint8_t value); // переводим BCD -> обычное десятичное число
uint8_t RTC_ConvertFromBinDec(uint8_t value); // переводим обычное десятичное число -> BCD

#endif /* RTC_H_ */