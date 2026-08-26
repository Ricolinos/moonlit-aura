/* D-042: sustituto de host para firmware/rockbox/firmware/export/lcd.h
 * -- ese lcd.h real depende de cpu.h/config.h/events.h (solo existen
 * dentro del build de Rockbox). moonlit_art.c solo necesita el tipo
 * fb_data; ipod6g es LCD_DEPTH 16 (config/ipod6g.h), asi que fb_data
 * real es un pixel de 16 bits (lcd.h:104-106). Ver test/file.h para el
 * mismo mecanismo aplicado a open/read/write/close. */
#ifndef MOONLIT_TEST_LCD_H
#define MOONLIT_TEST_LCD_H
#include <stdint.h>
typedef uint16_t fb_data;
#endif
