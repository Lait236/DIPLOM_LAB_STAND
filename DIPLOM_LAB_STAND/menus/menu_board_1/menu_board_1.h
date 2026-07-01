/*
 * menu_board_1.h
 *
 * Created: 10.05.2026 21:28:14
 *  Author: Владислав
 */ 


#ifndef MENU_BOARD_1_H_
#define MENU_BOARD_1_H_

// объявляем таблицу элементов меню платы №1

extern const char menu_items_board_1[BOARD1_MENU_COUNT][40]; // таблица пунктов меню платы №1 сам массив находится в menu_items_board_1.c

// объявляем функции для работы с меню платы №1

void menu_board_1_reset(void); // сбрасываем меню платы №1 в начальное состояние, выбран будет первый пункт
void menu_board_1_draw_item(uint8_t item_index); // отрисовываем один пункт меню платы №1
void menu_board_1_draw(void); // отрисовываем всё меню платы №1
void menu_board_1_up(void); // перемещаем выделение меню платы №1 вверх
void menu_board_1_down(void); // перемещаем выделение меню платы №1 вниз
uint8_t menu_board_1_get_selected(void); // получаем индекс выбранного пункта меню платы №1

#endif /* MENU_BOARD_1_H_ */