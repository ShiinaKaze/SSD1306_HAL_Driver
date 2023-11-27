/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SSD1306_HAL_H
#define __SSD1306_HAL_H

#include  "ssd1306_font.h"

extern I2C_HandleTypeDef hi2c1;
/*
 * b0111 1000
 * SA0 set 0,R/W# set 0
 * */

/*128x64*/
#define DISPLAY_RESOLUTION_WIDTH 128
#define DISPLAY_RESOLUTION_HEIGH 64
/*I2C definition*/
#define SSD1306_I2C_ADDRESS 0x78
#define SSD1306_I2C_CONTROL_COMMAND 0x00
#define SSD1306_I2C_CONTROL_DATA 0x40
#define SSD1306_I2C_Transmit_Command(commands, size) HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS, commands, size, SSD1306_I2C_TRANSMIT_TIMEOUT)
#define SSD1306_I2C_Transmit_Data(data, size) HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS, data, size, SSD1306_I2C_TRANSMIT_TIMEOUT)


#define SSD1306_I2C_TRANSMIT_TIMEOUT 500

//void OLED_I2C_Write_Byte(uint8_t control_byte, uint8_t data_byte);
void SSD1306_I2C_Init();
void SSD1306_I2C_Display_Clear();
void SSD1306_I2C_Display_Test(uint8_t x, uint8_t y, uint8_t *data,
		uint8_t data_length);
void SSD1306_I2C_Display_Char(uint8_t x, uint8_t y, uint8_t ch,
		Font_Type font_type);
void SSD1306_I2C_Display_String(uint8_t x, uint8_t y, uint8_t *str,
		uint8_t str_length, Font_Type font_type);
#endif /* __OLED_HAL_H */
