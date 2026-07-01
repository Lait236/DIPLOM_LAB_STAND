/*
 * twi.h
 *
 * Created: 18.05.2026 23:30:44
 *  Author: Владислав
 */ 


#ifndef TWI_H_
#define TWI_H_

#include <stdint.h>

void I2C_Init(void); // инициализируем аппаратный TWI
void I2C_StartCondition(void); // формируем условие START
void I2C_StopCondition(void); // формируем условие STOP
void I2C_SendByte(uint8_t data); // отправляем один байт по I2C
void I2C_SendByteByADDR(uint8_t data, uint8_t addr); // отправляем байт data устройству с адресом addr
// используется как простая вспомогательная функция

uint8_t I2C_ReadByte(void); // читаем байт с подтверждением ACK

uint8_t I2C_ReadLastByte(void); // читаем последний байт без подтверждения ACK

#endif /* TWI_H_ */
