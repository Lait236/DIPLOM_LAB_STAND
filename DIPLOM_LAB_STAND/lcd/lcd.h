/*
 * lcd.h
 *
 * Created: 02.04.2026 1:01:47
 *  Author: Владислав
 */ 

#ifndef LCD_H_
#define LCD_H_

// объявляем все функции принадлежащие работе с дисплеем, которые будут вызываться извне

void lcd_port_init(void); // иницализация портов для дисплея
void lcd_init(void); // инициализация дисплея
void lcd_fill_color(uint16_t color); // заливка всего экрана одним цветом
void lcd_draw_pixel(uint8_t x, uint8_t y, uint16_t color); // отрисовка одиного пикселя
void lcd_draw_char(uint8_t x, uint8_t y, char c, uint16_t color, uint16_t bg, uint8_t scale); // отрисовка одиного ASCII-символа
void lcd_print_string(uint8_t x, uint8_t y, const char *text, uint16_t color, uint16_t bg, uint8_t scale); // отрисовка строки на дисплее
void lcd_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color); // заливка прямоугольной области одним цветом

#endif /* LCD_H_ */