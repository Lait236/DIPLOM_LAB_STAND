/*
 * stepmotor.h
 *
 * Created: 22.04.2026 0:30:15
 *  Author: Владислав
 */ 


#ifndef STEPMOTOR_H_
#define STEPMOTOR_H_

#include <stdint.h>

void SM_init(void); // инициализируем выводы шагового мотора
void SM_off(void); // отключаем все обмотки мотора
void SM_step_forward(uint16_t delay_us); // выполняем один полушаг вперёд
void SM_step_back(uint16_t delay_us); // выполняем один полушаг назад
void SM_forward_steps(uint16_t steps, uint16_t delay_us); // выполняем несколько полушагов вперёд
void SM_back_steps(uint16_t steps, uint16_t delay_us); // выполняем несколько полушагов назад
void SM_forvard(void); // выполняем 8 полушагов вперёд
void SM_back(void); // выполняем 8 полушагов назад

#endif /* STEPMOTOR_H_ */
