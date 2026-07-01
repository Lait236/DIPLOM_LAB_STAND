/*
 * led_bar.h
 *
 * Created: 13.05.2026 23:36:58
 *  Author: Владислав
 */ 


#ifndef LED_H_
#define LED_H_

void led_bar_init(void); // инициализируем 8 светодиодов, вывод PC7..PC0 делаем выходами
void led_bar_off(void); // выключаем все светодиоды
void led_bar_set_value(uint8_t value); // устанавливаем значение

#endif /* LED_H_ */