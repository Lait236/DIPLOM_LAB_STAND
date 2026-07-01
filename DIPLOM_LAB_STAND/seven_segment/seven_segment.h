/*
 * seven_segment.h
 *
 * Created: 10.05.2026 21:07:24
 *  Author: Владислав
 */ 


#ifndef SEVEN_SEGMENT_H_
#define SEVEN_SEGMENT_H_

#include <stdint.h>

void seven_segment_init(void); // инициализируем семисегментные индикаторы
// сегменты: PORTA
// разряды: PB0, PB1, PB2, PB3

void seven_segment_enable(void); // разрешаем отображение числа
void seven_segment_disable(void); // выключаем все разряды и гасим сегменты
void seven_segment_set_number(uint16_t num); // выводим число 0..9999

void seven_segment_set_digits(uint8_t thousands, uint8_t hundreds, uint8_t tens, uint8_t units); // задаём 4 цифры вручную:
// значение THOUSANDS — тысячи,
// значение HUNDREDS — сотни,
// значение TENS — десятки,
// значение UNITS — единицы


#endif /* SEVEN_SEGMENT_H_ */
