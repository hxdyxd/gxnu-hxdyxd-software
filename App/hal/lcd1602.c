#include "lcd1602.h"


/*******************************/
static void delay_nus(uint32_t n)
{
    while(n--) {
        uint8_t i = 7;
        while(i--);
    }
}


#define  LCD_GPIO_READ(port, pin)              HAL_GPIO_ReadPin(port, pin)
#define  LCD_GPIO_WRITE(port, pin, value)      HAL_GPIO_WritePin(port, pin, ((value)==0)?(GPIO_PIN_RESET):(GPIO_PIN_SET))
#define  LCD_Delay(t)                          delay_nus(t)

struct tGpio {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
};

struct tLCD1602 {
    struct tGpio LCD_RS;
    struct tGpio LCD_RW;
    struct tGpio LCD_EN;
    struct tGpio LCD_DB7;
    struct tGpio LCD_DB6;
    struct tGpio LCD_DB5;
    struct tGpio LCD_DB4;
};

const struct tLCD1602 LCD1602_PORT = {
    .LCD_RS = {
        .GPIOx = LCD1602_RS_GPIO_Port,
        .GPIO_Pin = LCD1602_RS_Pin,
    },
    .LCD_RW = {
        .GPIOx = LCD1602_RW_GPIO_Port,
        .GPIO_Pin = LCD1602_RW_Pin,
    },
    .LCD_EN = {
        .GPIOx = LCD1602_EN_GPIO_Port,
        .GPIO_Pin = LCD1602_EN_Pin,
    },
    
    .LCD_DB7 = {
        .GPIOx = LCD1602_DB7_GPIO_Port,
        .GPIO_Pin = LCD1602_DB7_Pin,
    },
    .LCD_DB6 = {
        .GPIOx = LCD1602_DB6_GPIO_Port,
        .GPIO_Pin = LCD1602_DB6_Pin,
    },
    .LCD_DB5 = {
        .GPIOx = LCD1602_DB5_GPIO_Port,
        .GPIO_Pin = LCD1602_DB5_Pin,
    },
    .LCD_DB4 = {
        .GPIOx = LCD1602_DB4_GPIO_Port,
        .GPIO_Pin = LCD1602_DB4_Pin,
    },
};
/********************************/
char lcd_console_buffer[64];


static inline void lcd1602_write_byte(uint8_t data)
{
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_EN.GPIOx, LCD1602_PORT.LCD_EN.GPIO_Pin, 1);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB4.GPIOx, LCD1602_PORT.LCD_DB4.GPIO_Pin, data&0X10);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB5.GPIOx, LCD1602_PORT.LCD_DB5.GPIO_Pin, data&0X20);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB6.GPIOx, LCD1602_PORT.LCD_DB6.GPIO_Pin, data&0X40);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB7.GPIOx, LCD1602_PORT.LCD_DB7.GPIO_Pin, data&0X80);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_EN.GPIOx, LCD1602_PORT.LCD_EN.GPIO_Pin, 0);
    
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_EN.GPIOx, LCD1602_PORT.LCD_EN.GPIO_Pin, 1);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB4.GPIOx, LCD1602_PORT.LCD_DB4.GPIO_Pin, data&0X01);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB5.GPIOx, LCD1602_PORT.LCD_DB5.GPIO_Pin, data&0X02);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB6.GPIOx, LCD1602_PORT.LCD_DB6.GPIO_Pin, data&0X04);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_DB7.GPIOx, LCD1602_PORT.LCD_DB7.GPIO_Pin, data&0X08);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_EN.GPIOx, LCD1602_PORT.LCD_EN.GPIO_Pin, 0);
}



static inline void lcd1602_write_command(uint8_t data)
{
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_EN.GPIOx, LCD1602_PORT.LCD_EN.GPIO_Pin, 0);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_RW.GPIOx, LCD1602_PORT.LCD_RW.GPIO_Pin, 0);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_RS.GPIOx, LCD1602_PORT.LCD_RS.GPIO_Pin, 0);
    lcd1602_write_byte(data);
    LCD_Delay(100);
}

static inline void lcd1602_write_data(uint8_t data)
{
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_EN.GPIOx, LCD1602_PORT.LCD_EN.GPIO_Pin, 0);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_RW.GPIOx, LCD1602_PORT.LCD_RW.GPIO_Pin, 0);
    LCD_GPIO_WRITE(LCD1602_PORT.LCD_RS.GPIOx, LCD1602_PORT.LCD_RS.GPIO_Pin, 1);
    lcd1602_write_byte(data);
    LCD_Delay(100);
}


void lcd1602_init(void)
{
    LCD_Delay(10);
    
    lcd1602_write_command(0x28);
    lcd1602_write_command(0x28);
    lcd1602_write_command(0x28); //4bit mode
    
    lcd1602_write_command(0x0c);
    lcd1602_write_command(0x06);
    lcd1602_clear(); //clear
}

void lcd1602_clear(void)
{
    lcd1602_write_command(0x01);
}

void lcd1602_write_char(uint8_t x, uint8_t y, char c)
{
    switch(y) {
        case 0:lcd1602_write_command(0x80 + x); break;
        case 1:lcd1602_write_command(0xC0 + x); break;
        case 2:lcd1602_write_command(0x94 + x); break;
        case 3:lcd1602_write_command(0xD4 + x); break;
        default:lcd1602_write_command(0x80 + x); break;
    }
    lcd1602_write_data(c);
}


void lcd1602_write_string(uint8_t x, uint8_t y, char *s)
{
    uint8_t i, end = 0;
    switch(y) {
        case 0:lcd1602_write_command(0x80 + x); break;
        case 1:lcd1602_write_command(0xC0 + x); break;
        case 2:lcd1602_write_command(0x94 + x); break;
        case 3:lcd1602_write_command(0xD4 + x); break;
        default:lcd1602_write_command(0x80 + x); break;
    }
    
    for(i=x;i<16;i++) {
        if(*s=='\0'){
            end = 1;
        }
        if(end){
            lcd1602_write_data(' ');
        }else{
            lcd1602_write_data(*s++);
        }
    }
}

