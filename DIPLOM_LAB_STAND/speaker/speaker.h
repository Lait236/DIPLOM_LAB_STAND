/*
 * speaker.h
 *
 * Created: 14.05.2026 0:09:27
 *  Author: Владислав
 */ 


#ifndef SPEAKER_H_
#define SPEAKER_H_

void speaker_init(void); // инициализируем спикер
void speaker_tone_start(uint16_t freq_hz); // запускаем звук заданной частоты
void speaker_tone_stop(void); // останавливаем звук
void speaker_beep(uint16_t freq_hz, uint16_t duration_ms); // воспроизводим короткий звуковой сигнал

#endif /* SPEAKER_H_ */
