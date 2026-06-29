#ifndef _LCD_H_
#define _LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

void lcd_on(int index);
int get_lcd_fb_selector(void);
void lcd_off();
void lcd_backlight(int value);

#ifdef __cplusplus
}
#endif

#endif