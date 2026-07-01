/*
 * twi.c
 *
 * Created: 18.05.2026 23:30:56
 *  Author: Владислав
 */ 


#include "../main.h"
#include "twi.h"

#define I2C_SCL_CLOCK 100000UL // частота I2C 100 кГц

// инициализируем аппаратный TWI

void I2C_Init(void)
{
	TWSR = 0x00; // предделитель TWI = 1

	TWBR = (uint8_t)(((F_CPU / I2C_SCL_CLOCK) - 16UL) / 2UL); // расчёт TWBR для частоты SCL

	TWCR = (1 << TWEN); // включаем TWI
}

// формируем условие START

void I2C_StartCondition(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // регистр TWINT = 1 — сбросить флаг завершения операции

	while (!(TWCR & (1 << TWINT))); // ждём завершения операции
}

// формируем условие STOP

void I2C_StopCondition(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // регистр TWSTO = 1 — сформировать STOP
}

// отправляем один байт по I2C

void I2C_SendByte(uint8_t data)
{
	TWDR = data; // загружаем байт в регистр данных TWI

	TWCR = (1 << TWINT) | (1 << TWEN); // запускаем передачу байта

	while (!(TWCR & (1 << TWINT))); // ждём завершения передачи
}

// отправляем байт устройству с заданным адресом

void I2C_SendByteByADDR(uint8_t data, uint8_t addr)
{
	I2C_StartCondition(); // сигнал START

	I2C_SendByte(addr); // адрес устройства + бит записи/чтения

	I2C_SendByte(data); // данные

	I2C_StopCondition(); // сигнал STOP
}

// читаем байт с подтверждением ACK

uint8_t I2C_ReadByte(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // читаем байт и после чтения отправляем ACK

	while (!(TWCR & (1 << TWINT))); // ждём завершения чтения

	return TWDR; // возвращаем прочитанный байт
}
// читаем последний байт без подтверждения ACK

uint8_t I2C_ReadLastByte(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN); // читаем последний байт без ACK

	while (!(TWCR & (1 << TWINT))); // ждём завершения чтения

	return TWDR; // возвращаем последний байт
}
