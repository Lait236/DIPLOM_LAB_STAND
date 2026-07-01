/*
 * servo.h
 *
 * Created: 14.04.2026 22:57:59
 *  Author: Владислав
 */ 


#ifndef SERVO_H_
#define SERVO_H_

void servo_init(void); // инициализируем сервопривод || настраиваем D7 как вывод / OC1A используется для формирования ШИМ
void servo_set_angle(uint8_t angle); // устанавливаем угол сервопривода от 0 до 180 градусов
void servo_left(void); // поворачиваем сервопривод влево на один шаг
void servo_right(void); // поворачиваем сервопривод вправо на один шаг
void max_servo_right(void); // поворачиваем сервопривод максимально вправо
uint8_t servo_get_angle(void); // получаем текущий угол сервопривода
void servo_stop(void); // останавливаем сервосигнал и выключаем PD7

#endif /* SERVO_H_ */
