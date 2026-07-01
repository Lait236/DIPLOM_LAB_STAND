/*
 * lcd1602.h
 *
 * Created: 20.05.2026 1:52:05
 *  Author: Владислав
 */ 


#ifndef LCD1602_H_
#define LCD1602_H_

#include <stdint.h>

void lcd1602_init(void); // инициализация LCD 16x2
void lcd1602_clear(void); // очищаем LCD 16x2
void lcd1602_set_pos(uint8_t x, uint8_t y); // устанавливаем позицию курсора
void lcd1602_print_string(char str[]); // выводим строку на LCD 16x2
void lcd1602_off(void); // выключаем отображение LCD 16x2 и освобождаем выводы

#endif /* LCD1602_H_ */