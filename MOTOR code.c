#include "TM4C123.h"                    // Device header
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
 uint32_t d1 ,d2;
void delay_Microsecond(uint32_t time);
void Timer0A_init(void);
uint32_t Timer0A_Handler(void);
void Timer0B_init(void);
uint32_t Timer0B_Handler(void);
void Timer2A_Handler(void);
void IntGlobalEnable(void);
void Timer2A_init(void);
void delayMs(int n);
uint32_t d1 = 0;
uint32_t d2 = 0;

int pw = 0;
int pw1 =0;
const double _16MHz_1clock = 62.5e-9; /*Value of 1clock cycle in nanoseconds*/
const uint32_t MULTIPLIER  = 5882;  /*Derived from speed of sound*/
 
#define ECHO 0x40 //PB6
#define ECHO1 0x80//PB7
#define TRIGGER 0x10 //PA4(OUTPUT)
#define TRIGGER1 0x40 //PA 6
#define BLUE_LED 0x04//PF2 onboard Blue LED 
#define RED_LED 0x02 //PF1 onboard RED LED
uint32_t highEdge,lowEdge;
uint32_t highEdge1,lowEdge1;
uint32_t ddistance;
uint32_t ddistance1;/*Distance in centimeters*/
uint32_t counter =0; 

uint32_t Timer0A_Handler(void) 
	{ 
GPIOA->DATA &=~TRIGGER;
    delay_Microsecond(12);
    GPIOA->DATA |=ECHO|TRIGGER;
    delay_Microsecond(12);
    GPIOA->DATA &=~TRIGGER;
   
    TIMER0->ICR =4;
    while((TIMER0->RIS & 4)==0){}; //Wait till captured
        highEdge =  TIMER0->TAR;
 
        
        TIMER0->ICR =4; //clear timer capture flag
        while((TIMER0->RIS & 4)  ==0){};
            lowEdge = TIMER0->TAR;
              ddistance = lowEdge -highEdge;
              ddistance = _16MHz_1clock *(double) MULTIPLIER *(double)ddistance;
 
            return ddistance;
 
}

uint32_t Timer0B_Handler(void) { 
    GPIOA->DATA &=~TRIGGER1;
    delay_Microsecond(12);
    GPIOA->DATA |= ECHO1|TRIGGER1;
    delay_Microsecond(12);
    GPIOA->DATA &=~TRIGGER1;
    TIMER0->ICR |=0x400;//clears the timer before read


    while((TIMER0->RIS & 0x400)==0){}; //Wait till captured
    highEdge1 =  TIMER0->TBR;//reads the value from timer
    TIMER0->ICR |=0x400; //clear timer capture flag
    while((TIMER0->RIS & 0x400)  ==0){};
    lowEdge1 = TIMER0->TBR;
    ddistance1 = lowEdge1 -highEdge1;
    ddistance1 = _16MHz_1clock *(double) MULTIPLIER *(double)ddistance1;
			/*
			DISTANCE THRESHOLD
			Trigger interrupt;...int0.start();
			
			*/
    return ddistance1;
}

void Timer2A_Handler(void) 
	{ 
 TIMER2_ICR_R|= 0x001;
	
  d1 = Timer0A_Handler();
    if(d1 < 15) 
		{
			GPIOF->DATA |=BLUE_LED;
			GPIOD->DIR |= 0x0F;
			GPIOD->DATA |= 0x001;
			GPIOD->DATA &=~ 0x04;
		
			
		 for (pw = 100; pw < 3999; pw += 20)
        {
            PWM1->_3_CMPB = pw;
            delayMs(50);
        }
				
				GPIOD->DATA &=~ 0x001;
       
		}
	  else 
		{
			GPIOF->DATA &=~BLUE_LED;
		/*	GPIOD->DATA |=0x02;
		GPIOD->DATA &=~ 0x08;
			 for (pw = 3900; pw < 100; pw -= 20)
        {
            PWM1->_3_CMPB = pw;
            delayMs(50);
        }*/
       
		}
		
	 d2 = Timer0B_Handler();
    if(d2 < 15) 
		{
    GPIOF->DATA |=RED_LED;
			GPIOD->DIR |= 0x0F;
		GPIOD->DATA &=~ 0x01;
		GPIOD->DATA |= 0x04;
		 
        for (pw1 = 3900; pw1 >100; pw1 -= 20)
        {
            PWM1->_2_CMPB = pw1;
            delayMs(50);
        }
				GPIOD->DATA &=~ 0x04;
		}
	  else 
			GPIOF->DATA &=~RED_LED;
		 
}

void IntGlobalEnable(void)
{
    __asm("    cpsie   i\n");
}

int main(void){
	
		SYSCTL->RCGCPWM |= 0x02;        // enable clock to PWM1
    SYSCTL->RCGCGPIO |= 0x20;       // enable clock to GPIOF
    SYSCTL->RCGCGPIO |= 0x08;       // enable clock to GPIOB

    SYSCTL->RCC &= ~0x00100000;     // use system clock for PWM
    PWM1->_3_CTL = 0;               // disable PWM1_3 during configuration

    PWM1->_3_GENB = 0x0000080C;     // output high when load and low when match
    PWM1->_3_LOAD = 3999;           // 4 kHz
    PWM1->_3_CTL = 1;               // enable PWM1_3
	  PWM1->INVERT |= 0x80;           // positive pulse
	  PWM1->INVERT |= 0x10;
    PWM1->ENABLE |= 0x80;           // enable PWM1
    PWM1->_2_CTL = 0;               // disable PWM1_3 during configuration
    PWM1->_2_GENB = 0x0000080C;     // output high when load and low when match
    PWM1->_2_LOAD = 3999;           // 4 kHz
    PWM1->_2_CTL = 1;               // enable PWM1_3
    PWM1->ENABLE |= 0x10;           // enable PWM1
	
    GPIOF->DIR |= 0x09;            // set PORTF 3 pins as output (LED) pin
    GPIOF->DEN |= 0x09;             // set PORTF 3 pins as digital pins
    GPIOF->AFSEL |= 0x09;           // enable alternate function
    GPIOF->PCTL &= ~0x0000F00F;     // clear PORTF 3 alternate function
    GPIOF->PCTL |= 0x00005005;      // set PORTF 3 alternate funtion to PWM

    GPIOD->DEN |= 0x0F;             // PORTB 3 as digital pins
   // GPIOD->DIR |= 0x0F;             // set PORTB 3 as output
    GPIOD->DATA |= 0x00;            // enable PORTB 3
	SYSCTL->RCGCGPIO |=(1U<<0); //Enable clock for PORTA 
  // SYSCTL->RCGCGPIO |=(1U<<5); //Enable clock for PORTF 
   GPIOA->DIR |=TRIGGER;
   GPIOA->DIR |=TRIGGER1;
   GPIOF->DIR |=BLUE_LED;
	 GPIOF->DIR |=RED_LED;
   GPIOA->DEN |=(ECHO)|(TRIGGER);
	 GPIOA->DEN |= (ECHO1)|(TRIGGER1);
   GPIOF->DEN |= BLUE_LED;
	 GPIOF->DEN |=RED_LED;
   Timer0A_init();
	Timer0B_init();
	Timer2A_init();
	IntGlobalEnable();
	/*Interrupt0_init();
	timer start;
	*/
 
    while(1){
   
		 
   delay_Microsecond(100);
 
  }
  }

void delay_Microsecond(uint32_t time)
{
    int i;
    SYSCTL->RCGCTIMER |=(1U<<1); 
        TIMER1->CTL=0;
    TIMER1->CFG=0x04;
    TIMER1->TAMR=0x02;
    TIMER1->TAILR= 16-1;
    TIMER1->ICR =0x1;
    TIMER1->CTL |=0x01;
 
    for(i=0;i<time;i++){ 
       while((TIMER1->RIS & 0x1)==0)
        TIMER1->ICR = 0x1;
    }
 
}

void Timer0A_init(void)
{
    SYSCTL->RCGCTIMER |=(1U<<0); 
    SYSCTL->RCGCGPIO |=(1U<<1); //PortB
    GPIOB->DIR &=~ECHO;
    GPIOB->DEN |=ECHO;
    GPIOB->AFSEL |=ECHO;
    GPIOB->PCTL &=~0x0F000000;            
    GPIOB->PCTL |= 0x07000000;
    NVIC_PRI4_R &= ~0xE0000000;
    TIMER0->CTL &=~0x0001;           // TAen is disabled
    TIMER0->CFG =4;   //16 bit timer
    TIMER0->TAMR = 0x17;    // Timer count up TACDIR  Edge time mode TACMR CAPTURE MODE TAMR
    TIMER0->CTL |=0x00C;    // bOTH EDGES ARE ENABLE
    TIMER0->CTL |=0x0001;      // TAen enable to configure CFG
}


void Timer0B_init(void)
{
   // SYSCTL->RCGCTIMER |=(1U<<0); 
   // SYSCTL->RCGCGPIO |=(1U<<1); //PortB
    GPIOB->DIR &=~ECHO1;
    GPIOB->DEN |=ECHO1;
    GPIOB->AFSEL |=ECHO1;
    GPIOB->PCTL &=~0xF0000000;            
    GPIOB->PCTL |= 0x70000000;
 	  NVIC_PRI5_R |=0x00000020;
    TIMER0->CTL &=~0x100;           // Tben is disabled
    TIMER0->CFG =4;   //16 bit timer
    TIMER0->TBMR = 0x17;    // Timer count up TACDIR  Edge time mode TACMR CAPTURE MODE TBMR
    TIMER0->CTL |=0xC00;    // bOTH EDGES ARE ENABLE
    TIMER0->CTL |=0x0100;      // TBen enable to configure CFG
}

void Timer2A_init(void)
{
    SYSCTL->RCGCTIMER |=(1U<<2); 
   // NVIC_PRI4_R &= ~0xE0000000;
   TIMER2->CTL &=~0x001;           // TAen is disabled
    	TIMER2->CFG =4;   //16 bit timer
    	TIMER2->TAMR = 0x02;    // Timer PERIODIC
	TIMER2_TAILR_R = 7999999;
	NVIC_PRI5_R &=~0xE0000000;
	NVIC_EN0_R |= 0x00800000; // Enabling Timer2A
	TIMER2_IMR_R|= 0x001;  //arming Timer2A
    //	TIMER0->CTL |=0xC00;    // bOTH EDGES ARE ENABLE
    	TIMER2->CTL |=0x0001;      // TBen enable to configure CFG
}

void delayMs(int n)
{
    int i, j;
    for(i = 0 ; i < n; i++)

    for(j = 0; j < 3180; j++)
            {}  // do nothing for 1 ms
}