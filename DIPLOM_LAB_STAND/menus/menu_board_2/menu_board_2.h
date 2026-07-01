/*
 * menu.h
 *
 * Created: 02.04.2026 1:35:36
 *  Author: Владислав
 */ 


#ifndef MENU_H_
#define MENU_H_

// объявляем таблицу элементов меню

extern const char menu_items[MENU_COUNT][40]; // таблица элементов меню

// объявляем все функции необходиые для работы с меню

void menu_draw_item(uint8_t item_index); // отрисовываем один пункт меню
void menu_draw(void); // отрисовываем всё меню
void menu_up(void); // устанавливаем выделение пункта меню вверх
void menu_down(void); // устанавливаем выделение пункта меню вниз

#endif /* MENU_H_ */