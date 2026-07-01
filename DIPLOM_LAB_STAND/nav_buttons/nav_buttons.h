/*
 * nav_buttons.h
 *
 * Created: 09.05.2026 22:06:30
 *  Author: Владислав
 */ 


#ifndef NAV_BUTTONS_H_
#define NAV_BUTTONS_H_

void nav_buttons_init(void); // инициализируем пять кнопок управления стендом
char nav_buttons_get_key(void); // получаем виртуальную клавишу:
// левая      -> '4'
// верхняя    -> '2'
// центр      -> '5'
// нижняя     -> '8'
// правая     -> '6'
// если ничего не нажато -> KEY_NONE

#endif /* NAV_BUTTONS_H_ */