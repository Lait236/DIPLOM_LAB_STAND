/*
 * font_5x7.h
 *
 * Created: 02.04.2026 0:33:38
 *  Author: Владислав
 */ 


#ifndef FONT_5X7_H_
#define FONT_5X7_H_

// объявляем таблицы для букв и цифр

extern const uint8_t font_letters[34][5] PROGMEM; // объявление таблицы для букв || extern объявляет font_letters как глобальный двумерный массив, который создан в font_5x7.c, благодаря этому к font_letters можно обращаться из других файлов проекта
extern const uint8_t font_digit[10][5] PROGMEM; // объявление таблицы цифр
extern const uint8_t font_service_symb[3][5] PROGMEM; // служебные символы (*, #, .)

#endif /* FONT_5X7_H_ */