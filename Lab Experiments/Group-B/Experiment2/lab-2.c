/*

* Author: Texas Instruments 

* Editted by: Krishna Deepak, Bharadwaj 
	      CSE Department, IIT Bombay

* Description: This code will familiarize you with using GPIO, Interrupts and timers

* Filename: lab-2.c 

* Functions: setup(), ledPinConfig(), switchPinConfig(), main(), Timer0IntHandler()

* Global Variables: sw2Status, ui8LED

*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"

// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))

// states in state diagram
#define s1 1
#define s2 2
#define s3 3

int sw2Status = 1; // switch 2 press count
int ui8LED = 0; // color

int state1 = s1; // For the state diagram
int state2 = s1; // For the state diagram

/*

* Function Name: setup()

* Input: none

* Output: none

* Description: Set crystal frequency and enable GPIO Peripherals  

* Example Call: setup();

*/
void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}

/*

* Function Name: switchPinConfig()

* Input: none

* Output: none

* Description: Set PORTF Pin 0 and Pin 4 as input. Note that Pin 0 is locked.

* Example Call: switchPinConfig();

*/
void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;

	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_DIR_MODE_IN); // Set Pin-4 of PORT F as Input. Modifiy this to use another switch
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

/*

* Function Name: ledPinConfig()

* Input: none

* Output: none

* Description: Set PORTF Pin 1, Pin 2, Pin 3 as output.

* Example Call: ledPinConfig();

*/
void ledPinConfig(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  // Pin-1 of PORT F set as output. Modifiy this to use other 2 LEDs.
}

/*

* Function Name: Timer0IntHandler()

* Input: none

* Output: none

* Description: Interrupt handler called when this interrupt is raised. States changed acc to state diagram 
	       Upon pressing SW1, we change LED lights between RGB. 
	       For SW2, we just increase sw2status and monitor it.

* Example Call: 

*/
void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
	{
		if (state1 == s1) state1 = s2;
		else if (state1 == s2) {
			state1 = s3;
			if(ui8LED == 2)
				ui8LED = 8;
			else {
				if(ui8LED == 8)
					ui8LED = 4;
				else
					ui8LED = 2;
			}
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED);
		}
	}
	else
	{
		state1 = s1;
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
	}

	if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))
	{
		if (state2  == s1) state2 = s2;
		else if (state2 == s2) {
			state2 = s3;
			sw2Status++;
		}
	}
	else
	{
		state2 = s1;
	}
}

/*

* Function Name: main()

* Input: none

* Output: none

* Description: Setting up for interrupts and timers 

* Example Call: 

*/
int main(void){
	// setting up
	setup();
	ledPinConfig();
	switchPinConfig();

	uint32_t ui32Period;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	ui32Period = (SysCtlClockGet() / 10) / 2;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);

	while(1)
	{
	}
}

