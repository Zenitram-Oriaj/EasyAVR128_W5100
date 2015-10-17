/*
 * EasyAVR128.h
 *
 * Created: 10/14/2015 7:45:00 PM
 *  Author: Jairo Martinez
 */ 

#ifndef EASYAVR128_H_
#define EASYAVR128_H_

//////////////////////////////////////////////////////

#define SPI_DDR		DDRB				//  target-specific DDR for the SPI port lines 
#define SPI_PORT 	PORTB				//  target-specific port containing the SPI lines 
#define SPI_CS		PORTB0
#define SPI_SCLK	PORTB1
#define SPI_MOSI	PORTB2
#define SPI_MISO	PORTB3
#define RESET_BIT	PORTB4			//  target-specific port line used as reset 

//////////////////////////////////////////////////////

#define LED_DDR DDRC
#define	LEDPORT	PORTC

#define MAX_LEDS 8

#define LED8	PORTC0
#define LED7	PORTC1
#define LED6	PORTC2
#define LED5	PORTC3
#define LED4	PORTC4
#define LED3	PORTC5
#define LED2	PORTC6
#define LED1	PORTC7

// LED Macros

#define LED1_SET LED_DDR |= 1 << LED1;
#define LED2_SET LED_DDR |= 1 << LED2;
#define LED3_SET LED_DDR |= 1 << LED3;
#define LED4_SET LED_DDR |= 1 << LED4;
#define LED5_SET LED_DDR |= 1 << LED5;
#define LED6_SET LED_DDR |= 1 << LED6;
#define LED7_SET LED_DDR |= 1 << LED7;
#define LED8_SET LED_DDR |= 1 << LED8;

#define LED1_ON LEDPORT&=~(1<<LED1)
#define LED1_OF LEDPORT|=(1<<LED1)
#define LED1_TG LEDPORT^=(1<<LED1)

#define LED2_ON LEDPORT&=~(1<<LED2)
#define LED2_OF LEDPORT|=(1<<LED2)
#define LED2_TG LEDPORT^=(1<<LED2)

#define LED3_ON LEDPORT&=~(1<<LED3)
#define LED3_OF LEDPORT|=(1<<LED3)
#define LED3_TG LEDPORT^=(1<<LED3)

#define LED4_ON LEDPORT&=~(1<<LED4)
#define LED4_OF LEDPORT|=(1<<LED4)
#define LED4_TG LEDPORT^=(1<<LED4)

#define LED5_ON LEDPORT&=~(1<<LED5)
#define LED5_OF LEDPORT|=(1<<LED5)
#define LED5_TG LEDPORT^=(1<<LED5)

#define LED6_ON LEDPORT&=~(1<<LED6)
#define LED6_OF LEDPORT|=(1<<LED6)
#define LED6_TG LEDPORT^=(1<<LED6)

#define LED7_ON LEDPORT&=~(1<<LED7)
#define LED7_OF LEDPORT|=(1<<LED7)
#define LED7_TG LEDPORT^=(1<<LED7)

#define LED8_ON LEDPORT&=~(1<<LED8)
#define LED8_OF LEDPORT|=(1<<LED8)
#define LED8_TG LEDPORT^=(1<<LED8)

//////////////////////////////////////////////////////

#define BTN_DDR DDRB
#define	BTNPORT	PORTB

#define MAX_BTNS 8

#define BTN1	PINB0
#define BTN2	PINB1
#define BTN3	PINB2
#define BTN4	PINB3
#define BTN5	PINB4
#define BTN6	PINB5
#define BTN7	PINB6
#define BTN8	PINB7

#endif /* EASYAVR128_H_ */