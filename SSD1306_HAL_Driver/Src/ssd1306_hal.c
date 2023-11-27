#include "stm32f1xx_hal.h"
#include "ssd1306_hal.h"
#include "stdlib.h"
#include "string.h"

uint8_t commands_length;
uint8_t formatted_data_length;
/* I2C-bus data format
 * Slave Address - Control Byte - Data Byte - Data Byte...
 * Slave Address:0x78,include R/W# is set 0
 * Control Byte:
 * D/C# 0 command,0x00
 * D/C# 1 data,0x40
 * */
uint8_t SSD1306_Init_Commands[] = {
SSD1306_I2C_CONTROL_COMMAND,/*Control Byte - OLED I2C Command */
0xAE,/*Set Display Off*/
/*Initial Settings Configuration*/
0xD5, 0x80, /*Set Display Clock Divide Ratio/Oscillator Frequency*/
0xA8, 0x3F,/*Set MUX Ratio*/
0xD3, 0x00,/*Set Display Offset*/
0x40,/*Set Display Start Line*/
0xA1,/*Set Segment Re-Map*/
0xC8,/*Set COM Output ScanDirection */
0xDA, 0x02,/*Set COM Pins hardware configuration*/
0x81, 0xCF,/*Set Contrast Control*/
0xD9, 0xF1,/*Set Pre-Charge Period*/
0xDB, 0x30,/*Set VCOMH Deselect Level*/
0xA4,/*Set Entire Display On/Off*/
0xA6,/*Set Normal/Inverse Display*/
/*Clear Screen*/
0x8D, 0x14,/*Set Charge Pump */
/*Addressing_Setting*/
0x20, 0x02,/*Set Memory Addressing Mode*/
0x00,/*Set Lower Column Start Address for Page Addressing Mode*/
0x10,/*Set Higher Column Start Address for Page Addressing Mode*/
0xB0, /*Set Page Start Address for Page Addressing Mode*/
/*Clear Screen*/
0xAF /*Set Display On*/
};

uint8_t Split_Data(uint8_t data, uint8_t start, uint8_t end) {
	uint8_t numBits = end - start + 1;

	uint8_t mask = (1 << numBits) - 1;

	uint8_t result = (data >> start) & mask;

	return result;
}

//uint8_t* SSD1306_I2C_Get_Position_Commands(uint8_t x, uint8_t y) {
//	uint8_t *commands;
//	if (y % 8 == 0) {
//		commands_length = 4;
//	} else {
//		commands_length = 8;
//	}
//	commands = (uint8_t*) calloc(commands_length, sizeof(uint8_t));
//	uint8_t start_page = (y / 8) % 8;
//	for (int i = 0; i < commands_length; i = i + 4) {
//		commands[i + 0] = SSD1306_I2C_CONTROL_COMMAND;
//		commands[i + 1] = 0xB0 + start_page;
//		commands[i + 2] = 0x00 + ((x % 127) & 0x0F);
//		commands[i + 3] = 0x10 + (((x % 127) & 0xF0) >> 4);
//		start_page = (start_page + 1) % 8;
//	}
//	return commands;
//}

// Optimized by ChatGPT
uint8_t* SSD1306_I2C_Get_Position_Commands(uint8_t x, uint8_t y) {
	uint8_t commands_length = 8;
	uint8_t start_page = (y / 8) % 8;
	uint8_t x_mod = x % 127;

	if (y % 8 == 0) {
		commands_length = 4;
	}

	uint8_t *commands = calloc(commands_length, sizeof(uint8_t));

	for (int i = 0; i < commands_length; i += 4) {
		commands[i] = SSD1306_I2C_CONTROL_COMMAND;
		commands[i + 1] = 0xB0 + start_page;
		commands[i + 2] = 0x00 + (x_mod & 0x0F);
		commands[i + 3] = 0x10 + ((x_mod & 0xF0) >> 4);
		start_page = (start_page + 1) % 8;
	}

	return commands;
}

uint8_t* SSD1306_I2C_Get_Data(uint8_t y, uint8_t *data, uint8_t data_length) {
	uint8_t *formatted_data;
	if (y % 8 == 0) {
		formatted_data_length = data_length + 1;
		formatted_data = (uint8_t*) calloc(formatted_data_length,
				sizeof(uint8_t));
		formatted_data[0] = SSD1306_I2C_CONTROL_DATA;
		for (int i = 1; i < formatted_data_length; i++) {
			formatted_data[i] = data[i - 1];
		}
	} else {
		formatted_data_length = (data_length + 1) * 2;
		formatted_data = (uint8_t*) calloc(formatted_data_length,
				sizeof(uint8_t));
		uint8_t split_start = 0;
		uint8_t split_end = 8 - (y % 8) - 1;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < data_length + 1; j++) {
				if (j == 0) {
					formatted_data[i * (data_length + 1) + j] =
					SSD1306_I2C_CONTROL_DATA;
				} else {
					if (i == 0) {
						formatted_data[i * (data_length + 1) + j] = Split_Data(
								data[j - 1], split_start, split_end) << y % 8;
					} else {
						formatted_data[i * (data_length + 1) + j] = Split_Data(
								data[j - 1], split_start, split_end);
					}
				}
			}
			split_start = split_end + 1;
			split_end = split_start + (y % 8) - 1;
		}
	}
	return formatted_data;
}

void SSD1306_I2C_Init() {
	HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS, SSD1306_Init_Commands,
			sizeof(SSD1306_Init_Commands),
			SSD1306_I2C_TRANSMIT_TIMEOUT);
}

void SSD1306_I2C_Display_Clear() {
	for (int i = 0; i < 8; i++) {
		/*Set page[0]->page[7]*/
		uint8_t buffer_commands[] = { SSD1306_I2C_CONTROL_COMMAND, 0xB0 + i };
		HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS, buffer_commands,
				sizeof(buffer_commands),
				SSD1306_I2C_TRANSMIT_TIMEOUT);
		/*Set each page to be filled with 0x00*/
		uint8_t buffer_data[DISPLAY_RESOLUTION_WIDTH + 1];
		for (int i = 0; i < DISPLAY_RESOLUTION_WIDTH + 1; i++) {
			if (i == 0) {
				buffer_data[i] = SSD1306_I2C_CONTROL_DATA;
			} else {
				buffer_data[i] = 0x00;
			}
		}
		HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS, buffer_data,
				sizeof(buffer_data),
				SSD1306_I2C_TRANSMIT_TIMEOUT);
	}
	/*Set Page Start Address to page[0]*/
	uint8_t buffer[] = { SSD1306_I2C_CONTROL_COMMAND, 0xB0 };
	HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDRESS, buffer, sizeof(buffer),
	SSD1306_I2C_TRANSMIT_TIMEOUT);
}

void SSD1306_I2C_Display_Test(uint8_t x, uint8_t y, uint8_t *data,
		uint8_t data_length) {
	uint8_t *commands;
	uint8_t *formatted_data;
	commands = SSD1306_I2C_Get_Position_Commands(x, y);
	formatted_data = SSD1306_I2C_Get_Data(y, data, data_length);
	if (y % 8 == 0) {
		SSD1306_I2C_Transmit_Command(commands, commands_length);
		SSD1306_I2C_Transmit_Data(formatted_data, formatted_data_length);
	} else {
		uint8_t temp_commands[4];
		uint8_t temp_data[formatted_data_length / 2];
//		for (int i = 0; i < 4; i++) {
//			temp_commands[i] = commands[i];
//		}
		memcpy(temp_commands, commands, 4);
		SSD1306_I2C_Transmit_Command(temp_commands, sizeof(temp_commands));

//		for (int i = 0; i < formatted_data_length / 2; i++) {
//			temp_data[i] = formatted_data[i];
//		}
		memcpy(temp_data, formatted_data, formatted_data_length / 2);
		SSD1306_I2C_Transmit_Data(temp_data, sizeof(temp_data));

//		for (int i = 4; i < 8; i++) {
//			temp_commands[i - 4] = commands[i];
//		}
		memcpy(temp_commands, commands + 4, 4);
		SSD1306_I2C_Transmit_Command(temp_commands, sizeof(temp_commands));

//		for (int i = (formatted_data_length / 2); i < formatted_data_length;
//				i++) {
//			temp_data[i - (formatted_data_length / 2)] = formatted_data[i];
//		}
		memcpy(temp_data, formatted_data + (formatted_data_length / 2),
				formatted_data_length / 2);
		SSD1306_I2C_Transmit_Data(temp_data, sizeof(temp_data));
	}
	free(commands);
	free(formatted_data);
}

void SSD1306_I2C_Display_Char(uint8_t x, uint8_t y, uint8_t ch,
		Font_Type font_type) {
	uint8_t *commands;
	uint8_t *formatted_data;
	uint8_t data_length;
	if (font_type == Font6x8) {
		data_length = 6;
	}
	uint8_t data[data_length];
	if (font_type == Font6x8) {
		for (int i = 0; i < data_length; i++) {
			data[i] = font6x8[ch - 0x20][i];
		}
	}
	commands = SSD1306_I2C_Get_Position_Commands(x, y);
	formatted_data = SSD1306_I2C_Get_Data(y, data, data_length);
	if (y % 8 == 0) {
		SSD1306_I2C_Transmit_Command(commands, commands_length);
		SSD1306_I2C_Transmit_Data(formatted_data, formatted_data_length);
	} else {
		uint8_t temp_commands[4];
		uint8_t temp_data[formatted_data_length / 2];

		memcpy(temp_commands, commands, 4);
		SSD1306_I2C_Transmit_Command(temp_commands, sizeof(temp_commands));

		memcpy(temp_data, formatted_data, formatted_data_length / 2);
		SSD1306_I2C_Transmit_Data(temp_data, sizeof(temp_data));

		memcpy(temp_commands, commands + 4, 4);
		SSD1306_I2C_Transmit_Command(temp_commands, sizeof(temp_commands));

		memcpy(temp_data, formatted_data + (formatted_data_length / 2),
				formatted_data_length / 2);
		SSD1306_I2C_Transmit_Data(temp_data, sizeof(temp_data));
	}
	free(commands);
	free(formatted_data);
}

void SSD1306_I2C_Display_String(uint8_t x, uint8_t y, uint8_t *str,
		uint8_t str_length, Font_Type font_type) {
	if (font_type == Font6x8) {
		for (int i = 0; i < str_length - 1; i++) {
			SSD1306_I2C_Display_Char(x + (i * 6), y, str[i], font_type);
		}
	}
}
