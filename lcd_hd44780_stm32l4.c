

#include "lcd_hd44780_stm32l4.h"
#include "custom_chars.h"

#define SET_E() (HAL_GPIO_WritePin(LCD_E_GPIO,LCD_E_PIN,GPIO_PIN_SET))
#define SET_RS() (HAL_GPIO_WritePin(LCD_RS_GPIO,LCD_RS_PIN,GPIO_PIN_SET))
#define SET_RW() (HAL_GPIO_WritePin(LCD_RW_GPIO,LCD_RW_PIN,GPIO_PIN_SET))

#define CLEAR_E() (HAL_GPIO_WritePin(LCD_E_GPIO,LCD_E_PIN,GPIO_PIN_RESET))
#define CLEAR_RS() (HAL_GPIO_WritePin(LCD_RS_GPIO,LCD_RS_PIN,GPIO_PIN_RESET))
#define CLEAR_RW() (HAL_GPIO_WritePin(LCD_RW_GPIO,LCD_RW_PIN,GPIO_PIN_RESET))

#ifdef LCD_TYPE_162
	#define LCD_TYPE_204
#endif

#ifdef LCD_TYPE_202
	#define LCD_TYPE_204
#endif


void LCDGPIOInit()
{
	/*
	If you change the connection of lcd with mcu, you must
	also edit this function to change all those GPIO pin
	to output push-pull type. Also also enable clock to
	those GPIOs
	*/
	
	GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12 
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB2 PB10 PB11 PB12 
                           PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12 
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void DelayUS(uint32_t us)
{
	us=us*50;
	
	for(uint32_t i=0;i<us;i++)
	{
		__ASM("NOP");
	}
}
void LCDByte(uint8_t c,uint8_t isdata)
{
//Sends a byte to the LCD in 4bit mode
//cmd=0 for data
//cmd=1 for command


//NOTE: THIS FUNCTION RETURS ONLY WHEN LCD HAS PROCESSED THE COMMAND

uint16_t hn,ln;			//Nibbles
uint16_t temp;

hn=c>>4;
ln=(c & 0x0F);

if(isdata==0)
	CLEAR_RS();
else
	SET_RS();

DelayUS(1);		//tAS


SET_E();

//Send high nibble

temp=(LCD_DATA_GPIO->ODR & (~(0x000F<<LCD_DATA_POS)))|(hn<<LCD_DATA_POS);
LCD_DATA_GPIO->ODR=temp;

DelayUS(1);			//tEH

//Now data lines are stable pull E low for transmission

CLEAR_E();

DelayUS(1);

//Send the lower nibble
SET_E();

temp=(LCD_DATA_GPIO->ODR & (~(0X000F<<LCD_DATA_POS)))|(ln<<LCD_DATA_POS);

LCD_DATA_GPIO->ODR=temp;

DelayUS(1);			//tEH

//SEND

CLEAR_E();

DelayUS(1);			//tEL

LCDBusyLoop();
}

void LCDBusyLoop()
{
	//This function waits till lcd is BUSY

	uint8_t busy,status=0x00,temp;

	//Change Port to input type because we are reading data
	LCD_DATA_GPIO->MODER = LCD_DATA_GPIO->MODER & ~(0x000000FF<<(LCD_DATA_POS*2));

	//change LCD mode
	SET_RW();		//Read mode
	CLEAR_RS();		//Read status

	//Let the RW/RS lines stabilize

	DelayUS(1);		//tAS


	do
	{

		SET_E();

		//Wait tDA for data to become available
		DelayUS(1);

		status=((LCD_DATA_GPIO->IDR)>>LCD_DATA_POS);
		status=status<<4;

		DelayUS(1);

		//Pull E low
		CLEAR_E();
		DelayUS(1);	//tEL

		SET_E();
		DelayUS(1);

		temp=((LCD_DATA_GPIO->IDR)>>LCD_DATA_POS);
		temp&=0x0F;

		status=status|temp;

		busy=status & 0x80;//0b10000000;

		DelayUS(1);

    CLEAR_E();
		DelayUS(1);	//tEL
	}while(busy);

	CLEAR_RW();		//write mode

  //Change Port to output
	LCD_DATA_GPIO->MODER = LCD_DATA_GPIO->MODER | (0x00000055<<(LCD_DATA_POS*2));

}

void LCDInit(uint8_t style)
{
	/*****************************************************************

	This function Initializes the lcd module
	must be called before calling lcd related functions

	Arguments:
	style = LS_BLINK,LS_ULINE(can be "OR"ed for combination)
	LS_BLINK : The cursor is blinking type
	LS_ULINE : Cursor is "underline" type else "block" type
        LS_NONE : No visible cursor

	*****************************************************************/

	//After power on Wait for LCD to Initialize
	HAL_Delay(30);


	LCDGPIOInit();
	
	//Set 4-bit mode
	SET_E();
	(LCD_DATA_GPIO->ODR)|=((0x02)<<LCD_DATA_POS);
	DelayUS(1);
	CLEAR_E();
	DelayUS(1);

	//Wait for LCD to execute the Functionset Command
	LCDBusyLoop();                                    //[B] Forgot this delay

	//Now the LCD is in 4-bit mode

	
	LCDCmd(0x28);             //function set 4-bit,2 line 5x7 dot format
  LCDCmd(0x0C|style);	//Display On

	/* Custom Char */
  LCDCmd(HD44780_CMD_SET_CG_RAM_ADD);


	uint8_t __i;
	for(__i=0;__i<sizeof(__cgram);__i++)
		LCDData(__cgram[__i]);


}
void LCDWriteString(const char *msg)
{
	/*****************************************************************

	This function Writes a given string to lcd at the current cursor
	location.

	Arguments:
	msg: a null terminated C style string to print

	Their are 8 custom char in the LCD they can be defined using
	"LCD Custom Character Builder" PC Software.

	You can print custom character using the % symbol. For example
	to print custom char number 0 (which is a degree symbol), you
	need to write

	LCDWriteString("Temp is 30%0C");
                                  ^^
                                   |----> %0 will be replaced by
                                          custom char 0.

	So it will be printed like.

		Temp is 30°C

	In the same way you can insert any symbols numbered 0-7


	*****************************************************************/
 while(*msg!='\0')
 {
 	//Custom Char Support
	if(*msg=='%')
	{
		msg++;
		int8_t cc=*msg-'0';

		if(cc>=0 && cc<=7)
		{
			LCDData(cc);
		}
		else
		{
			LCDData('%');
			LCDData(*msg);
		}
	}
	else
	{
		LCDData(*msg);
	}
	msg++;
 }
}

void LCDWriteInt(int val,int8_t field_length)
{
	/***************************************************************
	This function writes a integer type value to LCD module

	Arguments:
	1)int val	: Value to print

	2)unsigned int field_length :total length of field in which the value is printed
	must be between 1-5 if it is -1 the field length is no of digits in the val

	****************************************************************/

	char str[5]={0,0,0,0,0};
	int i=4,j=0;

    //Handle negative integers
    if(val<0)
    {
        LCDData('-');   //Write Negative sign
        val=val*-1;     //convert to positive
    }
    else
    {
        LCDData(' ');
    }

    if(val==0 && field_length<1)
    {
        LCDData('0');
        return;
    }

	while(val)
	{
            str[i]=val%10;
            val=val/10;
            i--;
	}
	if(field_length==-1)
		while(str[j]==0) j++;
	else
		j=5-field_length;

	
	for(i=j;i<5;i++)
	{
	LCDData(48+str[i]);
	}
}
void LCDWriteFloat(float myval)
{
	//float myval;
 unsigned char i, dig[10];	
 unsigned int a;
 
 a = (unsigned int)(myval * 10.0 + 0.5);
// WriteCommand(0xc8);
 for(i = 0; i < 11; i++)
     {
      dig[i] = (unsigned char)(a % 10);
      a /= 10;
     }	
 
 if(!dig[10])
 {
       LCDData(' ');
       
 }
 else
     LCDData(dig[10] + '0');
 // LCDData(dig[10] + '0');
 LCDData(dig[9] + '0');

 LCDData(dig[8] + '0');
 LCDData(dig[7] + '0');
 
 LCDData(dig[6] + '0');
  LCDData(dig[5] + '0');
  LCDData(dig[4] + '0');
  LCDData(dig[3] + '0');
  LCDData(dig[2] + '0');
  LCDData(dig[1] + '0');
  LCDData('.');
 
   LCDData(dig[0] + '0');

 
}
/********************************************************************

Position the cursor to specific part of the screen

********************************************************************/
void LCDGotoXY(uint8_t x,uint8_t y)
{
 	if(x>=20) return;

	#ifdef LCD_TYPE_204

	switch(y)
	{
		case 0:
			break;
		case 1:
			x|=0x40;//0b01000000;
			break;
		case 2:
			x+=0x14;
			break;
		case 3:
			x+=0x54;
			break;
	}

	#endif

	#ifdef LCD_TYPE_164
	switch(y)
	{
		case 0:
			break;
		case 1:
			x|=0x40;//0b01000000;
			break;
		case 2:
			x+=0x10;
			break;
		case 3:
			x+=0x50;
			break;
	}

	#endif

	x|=0x80;//0b10000000;
  	LCDCmd(x);
}
