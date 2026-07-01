/*
 * max31865.h
 *
 * Created: 21.05.2026 1:53:52
 *  Author: Владислав
 */ 


#ifndef MAX31865_H_
#define MAX31865_H_

#include <stdint.h>

#define MAX31865_ERROR_TEMP -32768 // ошибка чтения температуры
void max31865_init(void); // инициализируем MAX31865
int16_t max31865_read_temp_c10(void); // читаем температуру в десятых долях градуса

#endif /* MAX31865_H_ */