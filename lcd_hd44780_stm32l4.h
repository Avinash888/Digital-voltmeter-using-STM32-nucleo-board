/********************************************************************


*/
#ifndef LCD_HD44780_PIC18_H
#define	LCD_HD44780_PIC18_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include <stdint.h>


/*_________________________________________________________________________________________*/

/************************************************
	LCD CONNECTIONS
*************************************************/

#define LCD_DATA_GPIO	GPIOB	//data port
#define LCD_DATA_POS 	12		//data port starts at PB12

#define LCD_E_GPIO 	GPIOB 	//Enable pin
#define LCD_E_PIN		GPIO_PIN_2	//Enable is connected to PB2

#define LCD_RS_GPIO	GPIOB
#define LCD_RS_PIN 	GPIO_PIN_10	//RS is connected to PB10

#define LCD_RW_GPIO	GPIOB
#define LCD_RW_PIN 	GPIO_PIN_11 //RW is connected to PB11

#define LCD_TYPE_162					//LCD type is 16x2 can be 16x4 or 20x4 too


//************************************************

#define CURSOR_BLINK 0x01
#define CURSOR_ULINE 0x02
#define CURSOR_NONE	 0x00

#define HD44780_CMD_SET_CG_RAM_ADD 0x40



/***************************************************
			F U N C T I O N S
****************************************************/



void LCDInit(uint8_t style);
void LCDWriteString(const char *msg);
void LCDWriteInt(int val,int8_t field_length);
void LCDGotoXY(uint8_t x,uint8_t y);
void LCDWriteFloat(float val);
//Low level
void LCDByte(uint8_t,uint8_t);
#define LCDCmd(c) (LCDByte(c,0))
#define LCDData(d) (LCDByte(d,1))

void LCDGPIOInit(void);

void LCDBusyLoop(void);
void DelayUS(uint32_t us);



/***************************************************
			F U N C T I O N S     E N D
****************************************************/


/***************************************************
	M A C R O S
***************************************************/
#define LCDClear() LCDCmd(0b00000001)
#define LCDHome() LCDCmd(0b00000010)

#define LCDWriteStringXY(x,y,msg) {\
 LCDGotoXY(x,y);\
 LCDWriteString(msg);\
}

#define LCDWriteIntXY(x,y,val,fl) {\
 LCDGotoXY(x,y);\
 LCDWriteInt(val,fl);\
}
/***************************************************/

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_HD44780_PIC18_H */

