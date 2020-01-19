//Led blinker

#include "stm8s.h"

#define ARR_P					32
#define LED_G					TIM1->CCR1L
#define LED_B					TIM1->CCR2L
#define LED_O					TIM1->CCR3L
#define LED_R					TIM1->CCR4L

#define SW_INT				GPIOD->DDR &=(~0x10);GPIOD->CR1 |=0x10;GPIOD->CR2 |=0x10;				//PD4 - Interrupt ON
#define SW_NORM				GPIOD->DDR &=(~0x10);GPIOD->CR1 |=0x10;GPIOD->CR2 &=(~0x10);		//PD4 - Interrupt ON
#define	TIM_PIN_ON		GPIOC->DDR |=0xD8;GPIOC->CR1 |=0xD8;GPIOC->CR2 |=0xD8;//&=(~0xD8); 					//PC7,6,4,3 - Tim Out
#define	TIM_PIN_OFF		GPIOC->DDR &=(~0xD8);GPIOC->CR1 |=0xD8;GPIOC->CR2 &=(~0xD8); 		//PC7,6,4,3 - Tim Out

#define SW1						(GPIOD->IDR & 0x10)
#define PRESS_FILTR		50

uint8_t const TIM4_PSCR=0x04;
uint8_t const TIM4_PERIOD=124;

volatile uint16_t press_time;
unsigned int TimingDelay;												//mSec counter

//=========Functions and Procedures==========
void TimingDelayDec(void) 							{					// 1ms interrupt

	if (TimingDelay != 0x00) {TimingDelay--;}
	if (!SW1 && press_time<=2000) {press_time++;}

}
void DelayMs(int Delay_time) 						{// ms Timer

	TimingDelay=Delay_time;
	while(TimingDelay!= 0x00);
	}
void sleep_10ms(uint16_t sleep_time) 		{	// go to sleep mode
		while(sleep_time) { 
			sleep_time--; 
			AWU->APR &=(~0x3F) | 0x30; 						//AWU->APR &=(~0x3F) | 0x30; 
			AWU->TBR |=0x4; 											//AWU->TBR |=0x4;  10 mcек
			AWU->CSR |=AWU_CSR_AWUEN ;   					//Enable interrupt.
			_asm("halt");//Sleep go on
			AWU->CSR &=(~AWU_CSR_AWUEN);
		}
}
void sleep_mode(void) 									{//Sleep Mode Routine
	TIM_PIN_OFF;
	AWU->CSR &=(~AWU_CSR_AWUEN);
	TIM1->CR1 &=~TIM1_CR1_CEN;
	CLK->PCKENR2 &=(~CLK_PCKENR2_AWU);
	CLK->PCKENR1 &=(~CLK_PCKENR1_TIM1);
	_asm("sim");		
		EXTI->CR1 |=	0b10<<6; 		// 01-Rising Edge, 10 - Falling Edge, 11 - Both
	_asm("rim");
	SW_INT;
	_asm("halt");//Sleep go on
	SW_NORM; 
	TIM_PIN_ON;	
	CLK->PCKENR1 	|= CLK_PCKENR1_TIM1;
	CLK->PCKENR2	|= CLK_PCKENR2_AWU;
	TIM1->CR1 		|=TIM1_CR1_CEN;
}

void power(void) {
if (!SW1 && press_time>=1999) {LED_O = ARR_P;LED_R = ARR_P;LED_G = ARR_P;LED_B = ARR_P;}
if (SW1 && press_time>=1999) {press_time=0;LED_O=0;LED_R=0;LED_G=0;LED_B=0;
DelayMs(10);sleep_mode();}
if (SW1 && press_time<1999) {press_time=0;}

}

void initial(void)											{//Init GPIO & Timer
		CLK->ICKR |= CLK_ICKR_HSIEN;
		CLK->PCKENR1 	=0;CLK->PCKENR2	=0;
		CLK->PCKENR1 	|= CLK_PCKENR1_TIM1 | CLK_PCKENR1_TIM4;
		CLK->PCKENR2	|= CLK_PCKENR2_AWU;
//=============TIM4============	
		TIM4->PSCR = TIM4_PSCR;
		TIM4->ARR = TIM4_PERIOD;
		TIM4->SR1 &=~TIM4_SR1_UIF; 	
		TIM4->IER	|= TIM4_IER_UIE; 
		TIM4->CR1 |= TIM4_CR1_CEN; // Start tim4
		_asm("rim");
//------------------GPIO CONFIG------------		
		GPIOA->DDR &=(~0xFF);GPIOA->CR1 |=0xFF;GPIOA->CR2 &=(~0xFF);		//input pull up
    GPIOB->DDR &=(~0xFF);GPIOB->CR1 |=0xFF;GPIOB->CR2 &=(~0xFF);		//input pull up
		GPIOC->DDR &=(~0xFF);GPIOC->CR1 |=0xFF;GPIOC->CR2 &=(~0xFF);		//input pull up
		GPIOD->DDR &=(~0xFF);GPIOD->CR1 |=0xFF ;GPIOD->CR2 &=(~0xFF);		//input pull up
//--------------TIM1_CONFIG---------------- 
		TIM_PIN_ON;
//===========TIM1=================		
		TIM1->PSCRH =0;TIM1->PSCRL = 100;
		TIM1->ARRH = 0;TIM1->ARRL = ARR_P;
		//TIM1->SMCR 	=0;  																
		TIM1->CCMR1	|= 0b110<<4 | TIM1_CCMR_OCxPE; 			//PWM1 mode
		TIM1->CCMR2 |= 0b110<<4 | TIM1_CCMR_OCxPE;				//------
		TIM1->CCMR3 |= 0b110<<4 | TIM1_CCMR_OCxPE;				//------
		TIM1->CCMR4 |= 0b110<<4 | TIM1_CCMR_OCxPE;				//------
		TIM1->CCER1 |= TIM1_CCER1_CC1P | TIM1_CCER1_CC1E;	//Active pulse Low
		TIM1->CCER1 |= TIM1_CCER1_CC2P | TIM1_CCER1_CC2E;
		TIM1->CCER2 |= TIM1_CCER2_CC3P | TIM1_CCER2_CC3E;	
		TIM1->CCER2 |= TIM1_CCER2_CC4P | TIM1_CCER2_CC4E; //Enable
		
		TIM1->CNTRH=0;	TIM1->CNTRL=0;									//—брос счетчика
		TIM1->CCR1H=0;	TIM1->CCR1L=0;									 		
		TIM1->CCR2H=0;	TIM1->CCR2L=0;
		TIM1->CCR3H=0;	TIM1->CCR3L=0;
		TIM1->CCR4H=0;	TIM1->CCR4L=0;
		
		TIM1->CR1 |= TIM1_CR1_CMS;												//CMS = 11 UP and Down 
		TIM1->BKR |=TIM1_BKR_MOE;
		TIM1->CR1 |= TIM1_CR1_CEN; 											// запустить таймер
		TIM1->EGR |= TIM1_EGR_UG;
}
void flash_on(uint8_t time, uint8_t led) 		{	//led 0bABCD  
uint8_t i=0;
for (i=0;i<ARR_P+1;i++)
{
	power();
 if (led&0b0001) {LED_O = i;}
 if (led&0b0010) {LED_R = i;}
 if (led&0b0100) {LED_G = i;}
 if (led&0b1000) {LED_B = i;}
 DelayMs(time);
 
 }


}
void flash_off(uint8_t time, uint8_t led) 	{
uint8_t i=0;
for (i=0;i<ARR_P+1;i++){
	power();
 if (led&0b0001) {LED_O = ARR_P-i;}
 if (led&0b0010) {LED_R = ARR_P-i;}
 if (led&0b0100) {LED_G = ARR_P-i;}
 if (led&0b1000) {LED_B = ARR_P-i;}
 DelayMs(time);
 
}
}
void snake_left (uint8_t time){
 
LED_B = ARR_P; DelayMs(time);
LED_G = ARR_P; DelayMs(time);
LED_R = ARR_P; DelayMs(time);
LED_O = ARR_P; DelayMs(time);
LED_B = 0; DelayMs(time);
LED_G = 0; DelayMs(time);
LED_R = 0; DelayMs(time);
LED_O = 0; DelayMs(time);

}
void snake_right(uint8_t time){
LED_O = ARR_P; DelayMs(time);
LED_R = ARR_P; DelayMs(time);
LED_G = ARR_P; DelayMs(time);
LED_B = ARR_P; DelayMs(time);
LED_O = 0; DelayMs(time);
LED_R = 0; DelayMs(time);
LED_G = 0; DelayMs(time);
LED_B = 0; DelayMs(time);
}
//=================================================

int main(void) 													{
initial();

while(1){ //=========== main cycle=================
//snake_left(100);
//snake_right(100);

flash_on(50, 1);
flash_off(50, 1);

flash_on(50, 2);
flash_off(50, 2);

flash_on(50, 4);
flash_off(100, 4);

flash_on(50, 8);
flash_off(100, 8);

sleep_10ms(100);

flash_on(100,0xf); flash_off(100, 0xf);
sleep_10ms(50);

flash_on(20, 1);flash_on(20, 2);flash_on(20, 4);flash_on(20, 8);
sleep_10ms(50);
flash_off(20, 1);flash_off(20, 2);flash_off(20, 4);flash_off(20, 8);
sleep_10ms(50);
flash_off(50, 0x3);flash_off(50, 0x5);
flash_off(50, 0xc);flash_off(50, 0x2);
sleep_10ms(50);

} 												// end of mine cycle
} 

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line)
{ 
 //printf("Wrong parameters value: file %s on line %d\r\n", file, (int) line);	
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
